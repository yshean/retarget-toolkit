clear all;
addpath(genpath('..\..\..\SparseCodingLib'));
addpath(genpath('..\..\..\PatchTool'));
addpath(genpath('..\..\..\Images'));
addpath(genpath('..\..\..\ImageProcessing'));
 
load AW.mat
patch_size = 8;
increment = 2;
load boyfast_sc20nocosDownsampling2AW.mat;
image = imread('boy.jpg');
P = dense_sampling(image, patch_size, increment);
img = DownSampling(image, 2, 'Gaussian', 5);
% A = dense_sampling(image, 8, 4);
Down = dense_sampling(img, 8, 4);

num_patch = size(P,2);
% load AW.mat
% num_coeff = size(A,2);
num_coeff = size(A,2) + size(Down,2);
avg_coeff = zeros(num_coeff, 1);
for i = 1:1:num_patch
    avg_coeff = avg_coeff + result(i).x;
end
avg_coeff = avg_coeff ./ num_patch;

% analysis
saliency = zeros(size(image,1) + 10, size(image,2) + 10);
for i = 1:1:num_patch     
    % t = (result(i).x - avg_coeff)' * (result(i).x - avg_coeff);    

    t = result(i).x - avg_coeff;
        for j = 1:1:size(t,1)
            if t(j) < 0
                t(j) = 0;
            end
        end

%     inverse = ones(size(avg_coeff,1), 1);
%     inverse = inverse .* max(abs(avg_coeff));
%     t = abs(result(i).x)' * (inverse - abs(avg_coeff));     
    
    %simply taking mse
    %t = result(i).mse;
    
    % taking reverse of norm1
    %t = 0.1 / sum(result(i).x);
 
    [y x] = patch_position(i, patch_size, increment, size(image));     
    saliency(x:x+patch_size, y:y+patch_size) = saliency(x:x+patch_size, y:y+patch_size) + sum(t);
end
saliency = saliency ./ num_patch;
real_img = saliency;
min_value = min(min(real_img));
max_value = max(max(real_img));
imshow(abs(real_img), [min_value max_value]);