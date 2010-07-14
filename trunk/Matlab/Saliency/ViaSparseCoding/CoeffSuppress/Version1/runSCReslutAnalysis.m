clear all;
 
% library
addpath(genpath('..\..\..\SparseCodingLib'));
addpath(genpath('..\..\..\PatchTool'));
addpath(genpath('..\..\..\Images'));
addpath(genpath('..\..\..\ImageProcessing'));

% setting
patch_size = 8;
increment = 4;
load Data\boatmanfast_sc20blur10level4incrementsupressed.mat; % result is loaded
image = imread('boatman.jpg');

% get number of patch (hardway)
A = dense_sampling(image, patch_size, increment);
num_patch = size(A,2);

% analysis
saliency = zeros(size(image,1) + 10, size(image,2) + 10);

for i = 1:1:num_patch             
    [y x] = patch_position(i, patch_size, increment, size(image));     
    saliency(x:x+patch_size, y:y+patch_size) = saliency(x:x+patch_size, y:y+patch_size) + calculate_saliency_sc(result(i), 'mse',[]);
end

saliency = saliency ./ num_patch;

% display
real_img = saliency;
min_value = min(min(real_img));
max_value = max(max(real_img));
imshow(abs(real_img), [min_value max_value]);