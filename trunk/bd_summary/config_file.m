%% Config File
% Store all parameters
img_ext = '.jpg';
patch_size = [7,7];

scaling_factor = 1/4;
resize_gap = 0.95;
resize_increase_factor = 0.95;
resize_target = 0.9;

converge_thresh = 0;
upsample_factor = 2^(0.25);
complete_weight = 1;
cohere_weight = 1;

% NN search radius
R = 3;

data_folder = '../data/';
result_folder = '../results/';