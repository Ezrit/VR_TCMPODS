#! /usr/bin/env python3

'''inpaintingLayer.py: creates an inpainting layer, given the background layer with its depth.'''

import cv2
import numpy as np
import argparse

__author__ = 'Moritz Mühlhausen'
__email__ = 'muehlhausen@cg.cs.tu-bs.de'


def main():
    parser = argparse.ArgumentParser(description='inpaintingLayer.py: creates an inpainting layer, given the background layer with its depth.')
    parser.add_argument('-b', '--background', action='store', dest='background_layer', default=None, metavar='PATH/TO/SCENE/', required=True, help='The input background layer.')
    parser.add_argument('-d', '--depth', action='store', dest='depth_layer', default=None, metavar='PATH/TO/SCENE/', required=True, help='The depth of the input background layer.')
    parser.add_argument('-k', '--kernelsize', action='store', dest='kernel_size', type=int, default=25, required=False, help='The kernelsize for the blur.')
    args = parser.parse_args()

    # read img and depth and resize depth to fit the image
    img = cv2.imread(args.background_layer)
    oldshape = img.shape[1::-1]
    depth = cv2.imread(args.depth_layer, flags=cv2.IMREAD_GRAYSCALE)
    img = cv2.resize(img.copy(), depth.shape[1::-1])
    newimg = img.copy()
    newdepth = depth.copy()

    it = np.nditer(depth, flags=['multi_index'])
    while not it.finished:
        minx = int(max(0, it.multi_index[0] - (args.kernel_size - 1) / 2))
        miny = int(max(0, it.multi_index[1] - (args.kernel_size - 1) / 2))
        maxx = int(min(depth.shape[0] - 1, it.multi_index[0] + (args.kernel_size - 1) / 2))
        maxy = int(min(depth.shape[1] - 1, it.multi_index[1] + (args.kernel_size - 1) / 2))
        kernel = depth[minx:maxx, miny:maxy]
        kernel = kernel.copy() / kernel.sum()

        newdepth[it.multi_index] = np.max(depth[minx:maxx, miny:maxy])
        newimg[it.multi_index] = (img[minx:maxx, miny:maxy] * np.expand_dims(kernel, axis=2)).sum(axis=0).sum(axis=0)
        it.iternext()

    cv2.imwrite('inpainting.png', newimg)
    cv2.imwrite('inpainting_depth.png', newdepth)
    newimg = cv2.resize(newimg.copy(), oldshape)
    cv2.imwrite('inpainting_big.png', newimg)


if __name__ == '__main__':
    main()
