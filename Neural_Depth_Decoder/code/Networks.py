#!/usr/bin/env python3

'''Networks.py: pytorch models. '''

import sys

import torch
import torch.nn as nn

import config
import utils

__author__ = 'Moritz Mühlhausen'
__email__ = 'muehlhausen@cg.cs.tu-bs.de'


# upsampling block
class UpsamplingBlock(nn.Module):
    def __init__(self, num_input, num_output, target_output_size):
        super(UpsamplingBlock, self).__init__()
        self.target_output_size = target_output_size
        # create block
        self.block = nn.Sequential(nn.ConvTranspose2d(num_input, num_output, kernel_size=5, stride=2, padding=1, output_padding=1, bias=True),
                                   nn.InstanceNorm2d(num_output),
                                   nn.ReLU(inplace=True),
                                   nn.Conv2d(num_output, num_output, kernel_size=7, stride=1, padding=3, bias=True),
                                   nn.InstanceNorm2d(num_output),
                                   nn.ReLU(inplace=True),
                                   nn.Conv2d(num_output, num_output, kernel_size=7, stride=1, padding=3, bias=True),
                                   nn.InstanceNorm2d(num_output),
                                   nn.ReLU(inplace=True))

    def forward(self, inputs):
        # return source if source.size()[2:] == target.size()[2:] else source[:, :, :target.size(2), :target.size(3)]
        # print(inputs.shape)''
        outputs = self.block(inputs)

        return outputs if outputs.size()[2:] == self.target_output_size else outputs[:, :, :self.target_output_size[0], :self.target_output_size[1]]


# ODS decoder network
class ODSDecoder(nn.Module):
    def __init__(self, num_input_bits):

        super(ODSDecoder, self).__init__()
        # initial fc layer
        num_output = int(config.data.dataset.image_width / 2**config.data.network.num_layers) * int(config.data.dataset.image_height / 2**config.data.network.num_layers)
        self.fc_layer = nn.Sequential(nn.Linear(num_input_bits, num_output), nn.LeakyReLU(0.2, inplace=True))
        # create upsampling layers
        upsamling_blocks_left_list = []
        upsamling_blocks_right_list = []

        # assert whether numchannels is multiple of 2
        if (config.data.network.num_intermediate_channels % 2 == 1):
            utils.ColorLogger.print('Num intermediate channels must be multiple of 2! Current value: {0}'.format(config.data.network.num_intermediate_channels), 'ERROR')
            sys.exit(0)

        for i in range(config.data.network.num_layers):
            upsamling_blocks_left_list.append(UpsamplingBlock(config.data.network.num_intermediate_channels // 2 if i != 0 else 1, config.data.network.num_intermediate_channels // 2, (int(config.data.dataset.image_height / 2**(config.data.network.num_layers-i-1)), int(config.data.dataset.image_width / 2**(config.data.network.num_layers-i-1)))))
            upsamling_blocks_right_list.append(UpsamplingBlock(config.data.network.num_intermediate_channels // 2 if i != 0 else 1, config.data.network.num_intermediate_channels // 2, (int(config.data.dataset.image_height / 2**(config.data.network.num_layers-i-1)), int(config.data.dataset.image_width / 2**(config.data.network.num_layers-i-1)))))
        self.upsamling_blocks_left = nn.Sequential(*upsamling_blocks_left_list)
        self.upsamling_blocks_right = nn.Sequential(*upsamling_blocks_right_list)
        # final conv
        self.final_layer_left = nn.Sequential(nn.Conv2d(config.data.network.num_intermediate_channels // 2, 1, kernel_size=7, stride=1, padding=3, bias=True), nn.Sigmoid())
        self.final_layer_right = nn.Sequential(nn.Conv2d(config.data.network.num_intermediate_channels // 2, 1, kernel_size=7, stride=1, padding=3, bias=True), nn.Sigmoid())

    # for wgan
    def clampWeights(self, clamp_val):
        for p in self.parameters():
            p.data.clamp_(-clamp_val, clamp_val)

    def forward(self, x):
        # create first 2d activations from linear layer
        out_fc = self.fc_layer(x).view(-1, 1, int(config.data.dataset.image_height / 2**config.data.network.num_layers), int(config.data.dataset.image_width / 2**config.data.network.num_layers))
        # upsample to final spatial resolution
        out_upsampling_left = self.upsamling_blocks_left(out_fc)
        out_upsampling_right = self.upsamling_blocks_right(out_fc)
        # get and return final output
        out_final = torch.cat((self.final_layer_left(out_upsampling_left), self.final_layer_right(out_upsampling_right)), dim=1)
        return out_final
