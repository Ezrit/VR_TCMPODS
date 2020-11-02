#! /usr/bin/env python3

'''generateSequence.py: sequentially executes a pretrained ODSEncoderNet.'''

import sys
import os
import argparse
import tqdm

import torch
import torch.optim as optim

import utils
import config
from Datasets import ODSDataset
from Networks import ODSDecoder

import cv2
import numpy as np

__author__ = 'Moritz Mühlhausen'
__email__ = 'muehlhausen@cg.cs.tu-bs.de'


def main():
    # parse conmmand line arguments and load global config file
    parser = argparse.ArgumentParser(description='train.py: trains a new network instance.')
    parser.add_argument('-c', '--config', action='store', dest='config_path', default=None, metavar='PATH/TO/CONFIG_FILE/', required=True, help='The config file to configure training.')
    args = parser.parse_args()
    config.loadConfig(args.config_path)

    # setup torch env
    tensor_type = utils.setupTorch(config.data.training.use_cuda, config.data.training.gpu_index)

    # create dataset and loader
    dataset = ODSDataset()

    # create networks and optimizers
    net = ODSDecoder(len(dataset).bit_length())
    optimizer = optim.Adam(net.parameters(), config.data.training.learningrate, betas=(config.data.training.adam_beta1, config.data.training.adam_beta2))
    init_epoch = 0

    # load network from checkpoint
    if config.data.training.initial_checkpoint is None:
        utils.ColorLogger.print('No checkpoint provided!', 'ERROR')
        sys.exit(0)
    net, optimizer_g, init_epoch = utils.load_checkpoint(config.data.training.initial_checkpoint, net, optimizer)

    # create output directories
    output_path = config.data.io.network_output_path
    os.makedirs(os.path.join(output_path, "left"), exist_ok=True)
    os.makedirs(os.path.join(output_path, "right"), exist_ok=True)
    os.makedirs(os.path.join(output_path, "combined"), exist_ok=True)

    # generate combined image frames
    net.eval()
    with torch.no_grad():
        for index in tqdm.trange(len(dataset), desc="samples"):
            indices, _, disp, _, _, _ = utils.castDataTuple(dataset[index], tensor_type, unsqueeze=True)
            out = net(indices)
            combined = torch.cat((torch.cat((disp[0, 0], disp[0, 1]), dim=1), torch.cat((out[0, 0], out[0, 1]), dim=1)), dim=0)
            cv2.imwrite(os.path.join(output_path, "combined/%04d.png" % (index+1)), (combined.data.cpu().numpy() * 255.0).astype(np.uint8))

    # generate video from combined image frames
    command_string = "cd {0} && ffmpeg -i %04d.png -c:v libx264 -vf fps=25 -pix_fmt yuv420p video.mp4".format(os.path.join(output_path, "combined"))
    if os.system(command_string) != 0:
        utils.ColorLogger.print('failed to create video using: "{0}"'.format(command_string), 'ERROR')

    # visualize outputs as left/right single images
    net.eval()
    with torch.no_grad():
        for sample in tqdm.trange(len(dataset), desc="samples"):
            out = net(torch.tensor([float(i) for i in dataset.graycodes[sample]]).type(tensor_type).unsqueeze(0))
            cv2.imwrite(os.path.join(output_path, "left/%04d.png" % (sample+1)), (out[0, 0].data.cpu().numpy() * 255.0).astype(np.uint8))
            cv2.imwrite(os.path.join(output_path, "right/%04d.png" % (sample+1)), (out[0, 1].data.cpu().numpy() * 255.0).astype(np.uint8))


if __name__ == '__main__':
    main()
