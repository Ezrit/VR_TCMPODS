#! /usr/bin/env python3

'''createMegaTextures.py: Takes the frames, depths & opacity maps to combine them into a MEGATEXTURE and video.'''

import os
import argparse
import cv2
import numpy as np
import utils
from tqdm import tqdm

__author__ = 'Moritz Mühlhausen'
__email__ = 'muehlhausen@cg.cs.tu-bs.de'


def main():
    parser = argparse.ArgumentParser(description='createOpacityMaps.py: creates opacity maps based on canny edge detection.')
    parser.add_argument('-s', '--scene', action='store', dest='scene_directory', default=None, metavar='PATH/TO/SCENE/', required=True, help='The input scene directory.')
    parser.add_argument('-l', '--layout', action='store', dest='texture_layout', default='oneFourOne', required=False, help='Layout for the resulting megatexture.')
    parser.add_argument('-v', '--video', action='store', dest='video_needed', type=bool, default=False, required=False, help='convert into a video?')
    parser.add_argument('-d', '--depth', action='store', dest='depth_type', default='flow', required=False, help='Depth source')
    parser.add_argument('-o', '--opacity', action='store', dest='opacity', default=0, required=False, type=int)
    args = parser.parse_args()

    # check for the chosen layout and change the image folder accordingly
    images_folder = 'high_res'
    if args.texture_layout == 'oneFourOne':
        images_folder = 'high_res'
    elif args.texture_layout == 'stacked':
        images_folder = 'low_res'

    # vars to the different folders
    images_path = os.path.join(args.scene_directory, images_folder)
    depth_path = os.path.join(args.scene_directory, args.depth_type + '_depth')
    opacity_path = os.path.join(args.scene_directory, args.depth_type + '_canny_opacity')
    texture_path = os.path.join(args.scene_directory, args.depth_type + '_megatexture')
    os.makedirs(texture_path, exist_ok=True)

    # get all images and make sure all other folders have the same #
    image_filenames = utils.list_sorted_files(os.path.join(images_path, 'right'))
    depth_filenames = utils.list_sorted_files(os.path.join(depth_path, 'left'))

    # iterate over all filenames and convert them into
    for idx in tqdm(range(len(depth_filenames)), desc="image"):
        # read all images to be combined
        image_left = cv2.imread(os.path.join(images_path, 'left', image_filenames[idx]))
        image_right = cv2.imread(os.path.join(images_path, 'right', image_filenames[idx]))
        depth_left = cv2.imread(os.path.join(depth_path, 'left', depth_filenames[idx]))
        depth_right = cv2.imread(os.path.join(depth_path, 'right', depth_filenames[idx]))
        if args.opacity == 0:
            opacity_left = cv2.imread(os.path.join(opacity_path, 'left', image_filenames[idx]))
            opacity_right = cv2.imread(os.path.join(opacity_path, 'right', image_filenames[idx]))
        else:
            opacity_left = np.ones(depth_left.shape)*255
            opacity_right = np.ones(depth_right.shape)*255

        # combine images according to layout
        if args.texture_layout == 'oneFourOne':
            middle = np.concatenate((depth_left, opacity_left, depth_right, opacity_right), axis=0)
            megatexture = np.concatenate((image_left, middle, image_right), axis=1)
        elif args.texture_layout == 'stacked':
            left_stack = np.concatenate((image_left, depth_left, opacity_left), axis=0)
            right_stack = np.concatenate((image_right, depth_right, opacity_right), axis=0)
            megatexture = np.concatenate((left_stack, right_stack), axis=1)

        # save the resulting megatexture
        cv2.imwrite(os.path.join(texture_path, image_filenames[idx]), megatexture)

    if args.video_needed:
        command_string = "ffmpeg -i %04d.png -c:v libx264 -vf fps=25 -pix_fmt yuv420p {0}/video.mp4".format(texture_path)
        if os.system(command_string) != 0:
            utils.ColorLogger.print('failed to create video using: "{0}"'.format(command_string), 'ERROR')


if __name__ == '__main__':
    main()
