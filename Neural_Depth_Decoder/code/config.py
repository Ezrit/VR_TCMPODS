'''Config.py: loads a global yaml config.'''

import yaml
from munch import DefaultMunch

import utils

__author__ = 'Moritz Kappel'
__email__ = 'kappel@cg.cs.tu-bs.de'

global data


def loadConfig(config_path):
    try:
        yaml_dict = yaml.unsafe_load(open(config_path))
        global data
        data = DefaultMunch.fromDict(yaml_dict)
    except():
        utils.ColorLogger.print('failed to parse config file at location: "{0}"'.format(config_path), 'ERROR')
