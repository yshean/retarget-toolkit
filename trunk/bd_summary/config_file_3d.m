%% Config File
% Store all parameters
img_ext = '.bmp';
patch_size = [7,7,5];

scaling_factor = [1/16,1/16,1/8];
resize_gap = [1,1,1];
resize_increase_factor = 0.95;
resize_target = [1,1,0.6];

converge_thresh = 20;
upsample_factor = 2^(0.25);
complete_weight = 1;
cohere_weight = 1;

% NN search radius
R = 1;

data_folder = '../data/';
result_folder = '../results/';