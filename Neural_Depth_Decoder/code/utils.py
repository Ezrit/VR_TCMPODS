#! /usr/bin/env python3

'''misc.py: miscellaneous helper functions and classes.'''

import os
import numpy as np
import torch
import torch.backends.cudnn as cudnn
import cv2
import natsort

import config

__author__ = 'Moritz Mühlhausen'
__email__ = 'muehlhausen@cg.cs.tu-bs.de'

SUPPORTED_TORCH_VERSION = '1.4.0'
SUPPORTED_CV_VERSION = '4.2.0'
SUPPORTED_CUDA_VERSION = '10.1'


# colored terminal printing
class ColorLogger:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    ERROR = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

    @staticmethod
    def print(msg, msg_type):
        print('{0}{1}{2}'.format(ColorLogger.__dict__[msg_type], msg, ColorLogger.ENDC) if msg_type not in ('ERROR', 'WARNING') else '{0}{1}:{2} {3}'.format(ColorLogger.__dict__[msg_type], msg_type, ColorLogger.ENDC, msg))


# AverageMeter.py: AverageMeter class extracted from the Pytoch Imagenet examples (https://github.com/pytorch/examples/blob/master/imagenet/main.py)
class AverageMeter(object):
    # Computes and stores the average and current value
    def __init__(self):
        self.reset()

    def reset(self):
        self.val = 0
        self.avg = 0
        self.sum = 0
        self.count = 0

    def update(self, val, n=1):
        self.val = val
        self.sum += val * n
        self.count += n
        self.avg = self.sum / self.count


def checkLibraryVersions():
    if torch.__version__ != SUPPORTED_TORCH_VERSION: ColorLogger.print('current Pytorch version: {0} (tested with {1})'.format(torch.__version__, SUPPORTED_TORCH_VERSION), 'WARNING')
    if torch.version.cuda != SUPPORTED_CUDA_VERSION: ColorLogger.print('current (Pytorch) CUDA version: {0} (tested with {1})'.format(torch.version.cuda, SUPPORTED_CUDA_VERSION), 'WARNING')
    if cv2.__version__ != SUPPORTED_CV_VERSION: ColorLogger.print('current OpenCV version: {0} (tested with {1})'.format(cv2.__version__, SUPPORTED_CV_VERSION), 'WARNING')


def save_checkpoint(epoch, net, optimizer, checkpoint_path):
    torch.save({'epoch': epoch,
                'net_state_dict': net.state_dict() if net is not None else None,
                'optimizer_state_dict': optimizer.state_dict() if optimizer is not None else None}, checkpoint_path)


def load_checkpoint(checkpoint_path, net, optimizer, map_location={'cuda:1': 'cuda:0'}):
    checkpoint = torch.load(checkpoint_path, map_location=map_location)
    if checkpoint['net_state_dict'] is not None and net is not None: net.load_state_dict(checkpoint['net_state_dict'])
    if checkpoint['optimizer_state_dict'] is not None and optimizer is not None and not config.data.training.reset_optimizer: optimizer.load_state_dict(checkpoint['optimizer_state_dict'])
    epoch = checkpoint['epoch'] if checkpoint['epoch'] is not None else 0
    del checkpoint
    torch.cuda.empty_cache()
    return net, optimizer, epoch


def setupTorch(USE_CUDA, GPU_INDEX):
    # print library version mismatches
    checkLibraryVersions()
    # set default params + enable gpu
    tensor_type = 'torch.FloatTensor'
    if USE_CUDA and torch.cuda.is_available():
        torch.cuda.set_device(GPU_INDEX)
        tensor_type = 'torch.cuda.FloatTensor'
        torch.set_default_tensor_type(tensor_type)
        cudnn.benchmark = True
        cudnn.fastest = True
    # additional debug stuff
    torch.utils.backcompat.broadcast_warning.enabled = True
    # uncomment to fix potential opencv-pytorch multiprocessing issues
    # multiprocessing.set_start_method('spawn', force=True)
    return tensor_type


def castDataTuple(t, tensor_type, unsqueeze=False):
    func = lambda x: x.type(tensor_type) if not unsqueeze else x.type(tensor_type).unsqueeze(0)
    return tuple(func(item) for item in t)


def list_sorted_files(path):
    return natsort.natsorted([file_name for file_name in os.listdir(path) if os.path.isfile(os.path.join(path, file_name))])


def list_sorted_directories(path):
    return natsort.natsorted([file_name for file_name in os.listdir(path) if not os.path.isfile(os.path.join(path, file_name))])


# generates rbg image from pytorch 2xHxW tensors
def pseudocolorOpticalFlow(flow, tensor_type, norm_factor=None):
    flow_numpy = flow.data.cpu().numpy().astype(np.float32)
    (x, y) = (np.array(flow_numpy[0], copy=True), np.array(flow_numpy[1], copy=True))
    (magnitude, angle) = cv2.cartToPolar(x, y, angleInDegrees=True)
    if norm_factor is None:
        cv2.normalize(magnitude, magnitude, alpha=0, beta=1, norm_type=cv2.NORM_MINMAX)
    else:
        magnitude = np.divide(magnitude, norm_factor)
        ret, magnitude = cv2.threshold(magnitude, 1.0, 0, cv2.THRESH_TRUNC)
    hsv = cv2.merge((angle, magnitude, np.ones(angle.shape, dtype=angle.dtype)))
    rgb = cv2.cvtColor(hsv, cv2.COLOR_HSV2RGB)
    return torch.from_numpy(rgb.transpose((2, 0, 1))).type(tensor_type)


# sets the given optimizers default learning rate (for all parameters)
def setOptimizerLR(optimizer, lr):
    for param_group in optimizer.param_groups:
        param_group['lr'] = lr


# visualize a list of samples (disp_left, disp_right) in tensorboard
def visualizeSamples(writer, tag, epoch, samples):
    # cat images and push to tensorboard
    images = []
    for sample in samples:
        # unpack data
        images.append(torch.cat((sample[:, 0], sample[:, 1]), 1).unsqueeze(1))
    writer.add_images(tag, torch.cat(images, dim=0), epoch, dataformats='NCHW')
