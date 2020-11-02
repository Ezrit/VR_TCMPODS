#! /usr/bin/env python3

'''createMegaTextures.py: Takes the frames, depths & opacity maps to combine them into a MEGATEXTURE and video.'''

import os
import argparse
import cv2
import numpy as np
import utils
from tqdm import tqdm
import math

__author__ = 'Moritz Mühlhausen'
__email__ = 'muehlhausen@cg.cs.tu-bs.de'


def main():
    parser = argparse.ArgumentParser(description='createOpacityMaps.py: creates opacity maps based on canny edge detection.')
    parser.add_argument('-s', '--scene', action='store', dest='scene_directory', default=None, metavar='PATH/TO/SCENE/', required=True, help='The input scene directory.')
    parser.add_argument('-m', '--maxDepth', action='store', dest='maxDepth', type=float, default=100.0)
    parser.add_argument('-r', '--radius', action='store', dest='radius', type=float, default=0.0715)
    args = parser.parse_args()

    # get all images and make sure all other folders have the same #
    image_filenames = utils.list_sorted_files(os.path.join(args.scene_directory, 'flow', 'left'))
    os.makedirs(os.path.join(args.scene_directory, 'flow_depth', 'left'), exist_ok=True)
    os.makedirs(os.path.join(args.scene_directory, 'flow_depth', 'right'), exist_ok=True)

    # iterate over all filenames and convert them into
    for idx in tqdm(range(len(image_filenames)), desc="image"):
        flowLeft = cv2.readOpticalFlow(os.path.join(args.scene_directory, 'flow', 'left', image_filenames[idx]))[:, :, 0]
        flowLeft[flowLeft > 0] = 0
        floimage = np.abs(flowLeft)
        floimage = floimage / np.shape(floimage)[1] * 2.0 * math.pi

        # compute distance to cylindrical representation
        d_c = np.ones(np.shape(floimage)) * args.maxDepth
        sin_angle = np.sin(floimage/2.0)
        d_c[sin_angle > 0] = args.radius / sin_angle[sin_angle > 0]

        # d_c_h is the height difference from the aquator to the actual point
        d_c_h = np.ones(np.shape(floimage)) * args.maxDepth
        sin_theta = np.sin(np.indices(np.shape(floimage))[0] / np.shape(floimage)[0] * math.pi)
        d_c_h[sin_theta > 0] = np.sqrt((d_c[sin_theta > 0] / sin_theta[sin_theta > 0]) * (d_c[sin_theta > 0] / sin_theta[sin_theta > 0]) - d_c[sin_theta > 0] * d_c[sin_theta > 0])

        # d_s is the distance to the ODS center
        d_s = np.sqrt(d_c * d_c - args.radius * args.radius)

        # d is the distance to the actual camera position
        d = np.sqrt(d_s * d_s + d_c_h * d_c_h)

        d[d > args.maxDepth] = args.maxDepth
        cv2.imwrite(os.path.join(args.scene_directory, 'flow_depth', 'left', image_filenames[idx][:-3] + 'png'), d * 255.0 / args.maxDepth)

        flowRight = cv2.readOpticalFlow(os.path.join(args.scene_directory, 'flow', 'right', image_filenames[idx]))[:, :, 0]
        flowRight[flowRight < 0] = 0
        floimage = np.abs(flowRight)
        floimage = floimage / np.shape(floimage)[1] * 2.0 * math.pi

        # compute distance to cylindrical representation
        d_c = np.ones(np.shape(floimage)) * args.maxDepth
        sin_angle = np.sin(floimage/2.0)
        d_c[sin_angle > 0] = args.radius / sin_angle[sin_angle > 0]

        # d_c_h is the height difference from the aquator to the actual point
        d_c_h = np.ones(np.shape(floimage)) * args.maxDepth
        sin_theta = np.sin(np.indices(np.shape(floimage))[0] / np.shape(floimage)[0] * math.pi)
        d_c_h[sin_theta > 0] = np.sqrt((d_c[sin_theta > 0] / sin_theta[sin_theta > 0]) * (d_c[sin_theta > 0] / sin_theta[sin_theta > 0]) - d_c[sin_theta > 0] * d_c[sin_theta > 0])

        # d_s is the distance to the ODS center
        d_s = np.sqrt(d_c * d_c - args.radius * args.radius)

        # d is the distance to the actual camera position
        d = np.sqrt(d_s * d_s + d_c_h * d_c_h)

        d[d > args.maxDepth] = args.maxDepth
        cv2.imwrite(os.path.join(args.scene_directory, 'flow_depth', 'right', image_filenames[idx][:-3] + 'png'), d * 255.0 / args.maxDepth)


if __name__ == '__main__':
    main()
