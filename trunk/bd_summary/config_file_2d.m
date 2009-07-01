%% Config File
% Store all parameters
img_ext = '.jpg';
patch_size = [7,7];

scaling_factor = 1/4;
resize_gap = [1,1];
resize_increase_factor = 0.95;
resize_target = [0.5,0.5];

converge_thresh = 10;
upsample_factor = 2^(0.25);
complete_weight = 1;
cohere_weight = 1;

% NN search radius
R = 1;

data_folder = '../data/';
result_folder = '../results/';