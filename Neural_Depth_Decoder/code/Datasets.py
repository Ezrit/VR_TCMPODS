#!/usr/bin/env python3

'''Datasets.py: PyTorch dataset implementation'''

import sys
import os
import cv2
import numpy as np
from tqdm import tqdm
import torch
from torch.utils.data import Dataset

from sympy.combinatorics.graycode import GrayCode

import utils
import config

__author__ = 'Moritz Mühlhausen'
__email__ = 'muehlhausen@cg.cs.tu-bs.de'


# dataset class for ODSEncoding
class ODSDataset(Dataset):

    # constructor
    def __init__(self):
        super(ODSDataset, self).__init__()
        # get file lists for dataloading
        self.left_eye_image_path = os.path.join(config.data.dataset.path, 'texture', 'left')
        self.right_eye_image_path = os.path.join(config.data.dataset.path, 'texture', 'right')
        self.left_eye_depth_path = os.path.join(config.data.dataset.path, 'depth', 'left')
        self.right_eye_depth_path = os.path.join(config.data.dataset.path, 'depth', 'right')
        self.left_eye_image_filenames = utils.list_sorted_files(self.left_eye_image_path)
        self.right_eye_image_filenames = utils.list_sorted_files(self.right_eye_image_path)
        self.left_eye_depth_filenames = utils.list_sorted_files(self.left_eye_depth_path)
        self.right_eye_depth_filenames = utils.list_sorted_files(self.right_eye_depth_path)
        graycodes_strings = list(GrayCode(len(self.left_eye_image_filenames).bit_length()).generate_gray())
        self.graycodes = [np.array([float(char_index) for char_index in graycodes_strings[sample_index]], np.float32) for sample_index in range(len(self.left_eye_depth_filenames))]
        # check if left/right eye list have the same length
        if (not (len(self.left_eye_depth_filenames) == len(self.right_eye_depth_filenames) == len(self.left_eye_image_filenames) == len(self.right_eye_image_filenames))):
            utils.ColorLogger.print('Amount of left/right eye images and depths does not match!', 'ERROR')
            sys.exit(0)
        # if load_dynamic flag is set, load the entire dataset into RAM
        if not config.data.dataset.load_dynamic:
            self.samples = []
            utils.ColorLogger.print('Loading dataset {0} into RAM...'.format(config.data.dataset.path), 'BOLD')
            for i in tqdm(range(len(self.left_eye_depth_filenames)), desc="loading sequence", leave=False):
                self.samples.append(self.loadSample(i))

    # loads a sample (index, images(left,right), depth(left,right))
    def loadSample(self, index):
        # create index tensor
        index_tensor = self.graycodes[index]
        # load images and flow
        image_left = cv2.imread(os.path.join(self.left_eye_image_path, self.left_eye_image_filenames[index]), 0)
        image_right = cv2.imread(os.path.join(self.right_eye_image_path, self.right_eye_image_filenames[index]), 0)
        flow_left = cv2.imread(os.path.join(self.left_eye_depth_path, self.left_eye_depth_filenames[index]), 0)
        flow_right = cv2.imread(os.path.join(self.right_eye_depth_path, self.right_eye_depth_filenames[index]), 0)
        # check if loading succeeded
        if ((image_left is None) or (image_right is None) or (flow_left is None) or (flow_right is None)):
            utils.ColorLogger.print('failed loading image or optical flow for sample {0}'.format(index), 'ERROR')
        # normalize
        image = np.stack((image_left, image_right), axis=0).astype(np.float32) / 255.0
        flow = np.stack((flow_left, flow_right), axis=0).astype(np.float32) / 255.0
        # return sample
        return (index_tensor, image, flow)

    def __getitem__(self, idx):
        # return two consecutive samples, use preloaded if dynamic is False
        sample = self.loadSample(idx) + self.loadSample(idx + 1) if config.data.dataset.load_dynamic else self.samples[idx] + self.samples[idx + 1]
        return [torch.from_numpy(x) for x in sample]

    def __len__(self):
        return len(self.left_eye_depth_filenames) - 1  # skip one element due to multi-sample loading
