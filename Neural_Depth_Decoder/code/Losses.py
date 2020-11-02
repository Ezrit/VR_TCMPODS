#! /usr/bin/env python3

'''Losses.py: Loss functions.'''

import sys

import torch
import torch.nn as nn

import config
import utils

__author__ = 'Moritz Mühlhausen'
__email__ = 'muehlhausen@cg.cs.tu-bs.de'


class DataLoss(nn.Module):
    def __init__(self):
        super(DataLoss, self).__init__()
        loss_type_dict = {'l1': lambda: nn.L1Loss(reduction='none'), 'mse': lambda: nn.MSELoss(reduction='none'), 'smooth_l1': lambda: nn.SmoothL1Loss(reduction='none')}
        self.criterion = loss_type_dict[config.data.training.data_loss_type]()

    def forward(self, net_output, gt_flow, weight_mask):
        loss = self.criterion(net_output, gt_flow) * weight_mask
        return config.data.training.data_loss_lambda * loss.mean()


class SpatialGradient(nn.Module):
    def __init__(self, num_channels, order=1):
        super(SpatialGradient, self).__init__()
        if order not in [1, 2]:
            utils.ColorLogger.print('SpatialSmoothnessLoss currently only supports 1st and 2nd order smoothness!', 'ERROR')
            sys.exit(0)
        # create convs with fixed custom kernel
        kernel_data_x = torch.tensor([[0.0, 0.0, 0.0], [1.0, -2.0, 1.0], [0.0, 0.0, 0.0]] if order == 2 else [[-1.0, 0.0, 1.0], [-2.0, 0.0, 2.0], [-1.0, 0.0, 1.0]])
        kernel_data_y = torch.tensor([[0.0, 1.0, 0.0], [0.0, -2.0, 0.0], [0.0, 1.0, 0.0]] if order == 2 else [[-1.0, -2.0, -1.0], [0.0, 0.0, 0.0], [1.0, 2.0, 1.0]])
        conv_kernel_x = torch.zeros((num_channels, num_channels, 3, 3))
        conv_kernel_y = torch.zeros((num_channels, num_channels, 3, 3))
        for c in range(num_channels):
            conv_kernel_x[c, c] = kernel_data_x
            conv_kernel_y[c, c] = kernel_data_y
        self.conv_x = nn.Conv2d(num_channels, num_channels, kernel_size=3, stride=1, padding=1, bias=False)
        self.conv_x.weight = nn.Parameter(conv_kernel_x)
        self.conv_x.weight.requires_grad = False
        self.conv_y = nn.Conv2d(num_channels, num_channels, kernel_size=3, stride=1, padding=1, bias=False)
        self.conv_y.weight = nn.Parameter(conv_kernel_y)
        self.conv_y.weight.requires_grad = False

    def forward(self, input_tensor, direction=0):
        # direction: <0 = x, >0 = y, 0 = norm_2(x,y)
        if direction < 0:
            out_grad = self.conv_x(input_tensor)
        elif direction > 0:
            out_grad = self.conv_y(input_tensor)
        else:
            grad_x = self.conv_x(input_tensor)
            grad_y = self.conv_y(input_tensor)
            out_grad = torch.norm(torch.stack([grad_x, grad_y], 0), 2, 0)

        return torch.abs(out_grad)


class SpatialSmoothnessLoss(nn.Module):
    def __init__(self):
        super(SpatialSmoothnessLoss, self).__init__()
        self.spatialGradient1stOrder = SpatialGradient(2, order=1)
        self.spatialGradient2ndOrder = SpatialGradient(2, order=2)

    # carbonnier penalty
    def charbonnierPenalty(self, inputTensor, epsilon=0.001):
        return torch.sqrt(torch.pow(inputTensor, 2) + epsilon)

    def forward(self, net_output, gt_image, weight_mask):
        grad_depth_x = self.spatialGradient2ndOrder(net_output, -1)
        grad_depth_y = self.spatialGradient2ndOrder(net_output, 1)
        grad_img_x = torch.exp(-config.data.training.smoothness_image_exponent * self.spatialGradient1stOrder(gt_image, -1))
        grad_img_y = torch.exp(-config.data.training.smoothness_image_exponent * self.spatialGradient1stOrder(gt_image, 1))
        loss = (self.charbonnierPenalty(grad_depth_x * grad_img_x) + self.charbonnierPenalty(grad_depth_y * grad_img_y)) * weight_mask
        return config.data.training.smoothness_loss_lambda * loss.mean()


class TemporalLoss(nn.Module):

    def __init__(self):
        super(TemporalLoss, self).__init__()

    # carbonnier penalty
    def charbonnierPenalty(self, inputTensor, epsilon=0.001):
        return torch.sqrt(torch.pow(inputTensor, 2) + epsilon)

    def forward(self, out1, out2, images1, images2, weight_mask):
        grad_img = torch.exp(-config.data.training.temporal_image_exponent * torch.abs(images2 - images1))
        loss = self.charbonnierPenalty(torch.abs(out2 - out1) * grad_img) * weight_mask
        return config.data.training.temporal_loss_lambda * loss.mean()


class ODSEncoderLoss(nn.Module):
    def __init__(self):
        super(ODSEncoderLoss, self).__init__()
        # create sublosses
        self.data_loss = DataLoss()
        self.smoothness_loss = SpatialSmoothnessLoss()
        self.temporal_loss = TemporalLoss()

        # create loss accumulators
        self.total_loss_accumulator = utils.AverageMeter()
        self.data_loss_accumulator = utils.AverageMeter()
        self.smoothness_loss_accumulator = utils.AverageMeter()
        self.temporal_loss_accumulator = utils.AverageMeter()

    def getForegroundWeighting(self, gt_flow):
        if config.data.training.data_loss_weighting_threshold > 0.0:
            fg_mask = torch.where(gt_flow > config.data.training.data_loss_weighting_threshold, torch.tensor([1.0]), torch.tensor([0.0]))
            n_t = gt_flow[0, 0].numel()
            n_f = fg_mask.sum(dim=(2, 3), keepdim=False)
            w_f = torch.zeros_like(n_f)
            w_f[n_f > 0] = n_t / (2.0 * n_f[n_f > 0])
            w_b = torch.zeros_like(n_f)
            w_b[(n_t - n_f) > 0] = n_t / (2.0 * (n_t - n_f)[(n_t - n_f) > 0])
            weight_mask = (fg_mask * w_f.unsqueeze(2).unsqueeze(3)) + ((1.0 - fg_mask) * w_b.unsqueeze(2).unsqueeze(3))
        else:
            weight_mask = torch.ones_like(gt_flow)
        return weight_mask

    def forward(self, out1, out2, flows1, images1, flows2, images2):
        # get foreground weighting masks
        weight_mask1 = self.getForegroundWeighting(flows1)
        weight_mask2 = self.getForegroundWeighting(flows2)
        # calculate losses
        l_data = self.data_loss(out1, flows1, weight_mask1) + self.data_loss(out2, flows2, weight_mask2)
        l_smooth = self.smoothness_loss(out1, images1, weight_mask1) + self.smoothness_loss(out2, images2, weight_mask2)
        l_temporal = self.temporal_loss(out1, out2, images1, images2, weight_mask1 + weight_mask2)
        total_loss = l_data + l_smooth + l_temporal

        # update accumulators
        self.data_loss_accumulator.update(l_data.item(), out1.size(0))
        self.smoothness_loss_accumulator.update(l_smooth.item(), out1.size(0))
        self.temporal_loss_accumulator.update(l_temporal.item(), out1.size(0))
        self.total_loss_accumulator.update(total_loss.item(), out1.size(0))

        # return loss
        return total_loss

    def printLosses(self, writer, epoch, clear=True):
        writer.add_scalar('total_loss', self.total_loss_accumulator.avg, epoch)
        writer.add_scalar('training_loss', self.data_loss_accumulator.avg, epoch)
        writer.add_scalar('smoothness_loss', self.smoothness_loss_accumulator.avg, epoch)
        writer.add_scalar('temporal_loss', self.temporal_loss_accumulator.avg, epoch)
        if clear:
            self.clearLosses()

    def clearLosses(self):
        self.total_loss_accumulator.reset()
        self.data_loss_accumulator.reset()
        self.smoothness_loss_accumulator.reset()
        self.temporal_loss_accumulator.reset()
