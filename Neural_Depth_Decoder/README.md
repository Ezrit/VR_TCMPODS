# Neural Depth Decoder

For training, generate a config file (see configs/default_initial.yaml and configs/default_finetuning.yaml for reference) with paths to your dataset. Then you can use the train.py script with the according config file, e.g.:

python code/train.py -c configs/default_initial.yaml

For generation of the learned depths use the generateSequence.py script with a generate.yaml config, e.g.:

python code/generateSequence.py configs/generate.yaml

## Training

Training will write checkpoints into the checkpoints folder according to the given name in the config file and the time the training started (every X epochs, you can set this in the config file!).
Furthermore it will create tensorboard logs to check the training progress.

## Dataset Folder layout

The training scripts expect a specific folder layout. Only the according frames must be in the folders (it counts the number of files). Number of frames must match in all folders!

Layout:

{name}/texture/left -> left eye image frames
{name}/texture/right -> right eye image frames

{name}/depth/left -> left eye depth frames to memorize
{name}/depth/right -> right eye depth frames to memorize

## Conda environment

Use the configs/conda.yml to create the environment we used for training. You need to set the prefix to the path to your conda3/envs folder. Then run:

conda env create -f configs/conda.yml

## Compute Depth from Flow

The script "computeDepth.py" can convert flow-files into depth images. You need to set the scene path (-s), the maxdepth which will be white in the depth images (-m) and the radius of your ODS setup (half of interpupillary distance for ODS creation) (-r).
The flow files need to be in a subfolder named flow/left and flow/right for the left/right eye flow accordingly. It will create flow_depth/left and flow_depth/right folders and save the depths images in these.
