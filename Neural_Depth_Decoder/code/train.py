#! /usr/bin/env python3

'''train.py: trains a new network instance.'''

import os
import argparse
import datetime
from tqdm import tqdm

import torch
from torch.utils.data import DataLoader
from torch.utils.tensorboard import SummaryWriter
import torch.optim as optim

import utils
import config
from Datasets import ODSDataset
from Networks import ODSDecoder
from Losses import ODSEncoderLoss

__author__ = 'Moritz Mühlhausen'
__email__ = 'muehlhausen@cg.cs.tu-bs.de'


def main():
    # parse conmmand line arguments and load global config file
    parser = argparse.ArgumentParser(description='train.py: trains a new network instance.')
    parser.add_argument('-c', '--config', action='store', dest='config_path', default=None, metavar='PATH/TO/CONFIG_FILE/', required=True,
                        help='The config file to configure training.')
    args = parser.parse_args()
    config.loadConfig(args.config_path)

    # setup torch env
    tensor_type = utils.setupTorch(config.data.training.use_cuda, config.data.training.gpu_index)

    # create dataset and loader
    dataset = ODSDataset()
    dataloader = DataLoader(dataset, batch_size=config.data.training.batch_size, shuffle=config.data.training.shuffle_loading, num_workers=config.data.training.num_workers)

    # create training criterion
    criterion = ODSEncoderLoss().type(tensor_type)

    # create networks and optimizers
    net = ODSDecoder(len(dataset).bit_length())
    optimizer = optim.Adam(net.parameters(), config.data.training.learningrate, betas=(config.data.training.adam_beta1, config.data.training.adam_beta2))
    init_epoch = 0
    lr_scheduler = optim.lr_scheduler.StepLR(optimizer, step_size=config.data.training.learningrate_decay_stepsize, gamma=config.data.training.learningrate_decay)

    # load checkpoint (if available)
    if config.data.training.initial_checkpoint is not None:
        net, optimizer_g, init_epoch = utils.load_checkpoint(config.data.training.initial_checkpoint, net, optimizer)

    # IO: create visualization objects and checkpoint output directory
    training_name_prefix = '{0}{date:%Y-%m-%d-%H:%M:%S}'.format(config.data.io.name_prefix + '_' if config.data.io.name_prefix is not None else '', date=datetime.datetime.now())
    checkpoint_output_path = os.path.join(config.data.io.output_checkpoints_dir, training_name_prefix)
    os.makedirs(checkpoint_output_path, exist_ok=True)
    writer = SummaryWriter(os.path.join(config.data.io.tensorboard_logdir, training_name_prefix))
    utils.ColorLogger.print('training name prefix: ' + training_name_prefix, 'OKBLUE')
    # write config file to tensorboard
    with open(args.config_path, 'r') as file:
        writer.add_text("config", file.read(), global_step=None, walltime=None)
    # save visualization samples and output to tensorboard
    visualization_samples = [utils.castDataTuple(dataset[i], tensor_type, unsqueeze=True) for i in config.data.io.visualization_indices]
    utils.visualizeSamples(writer, 'Ground Truth', 0, [visualization_samples[i][2] for i in range(len(visualization_samples))])

    # mainloop
    try:

        # loop over all epochs
        for epoch in tqdm(range(init_epoch, init_epoch + config.data.training.num_epochs), desc="epoch"):

            # train network
            net.train()
            for sample in tqdm(dataloader, desc="samples", leave=False):
                optimizer.zero_grad()
                indices1, images1, flows1, indices2, images2, flows2 = utils.castDataTuple(sample, tensor_type, unsqueeze=False)
                out1 = net(indices1)
                out2 = net(indices2)
                loss = criterion(out1, out2, flows1, images1, flows2, images2)
                loss.backward()
                optimizer.step()

            # update scheduler
            lr_scheduler.step()

            # print loss
            criterion.printLosses(writer, epoch, clear=True)

            # save checkpoint
            if epoch % config.data.io.checkpoint_backup_interval == 0:
                utils.save_checkpoint(epoch, net, optimizer, os.path.join(checkpoint_output_path, 'epoch_{0:03d}'.format(epoch)))

            # visualize outputs
            if epoch % config.data.io.visualization_interval == 0:
                net.eval()
                outputs = []
                with torch.no_grad():
                    for sample in visualization_samples:
                        out = net(sample[0])
                        outputs.append(out)
                utils.visualizeSamples(writer, 'Network Output', epoch, outputs)

    except KeyboardInterrupt:
        # save checkpoint before closing
        utils.ColorLogger.print('keyboard interrupt. saving last checkpoint.', 'WARNING')
        utils.save_checkpoint(epoch, net, optimizer, os.path.join(checkpoint_output_path, 'manual_interrupt'))

    writer.close()


if __name__ == '__main__':
    main()
