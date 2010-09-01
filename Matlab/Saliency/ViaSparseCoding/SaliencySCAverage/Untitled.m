clear all;
addpath('..\SampleImage');
addpath('..\PatchTool');
addpath('..\SparseCoding');
addpath('..\SparseCoding\ICL');
addpath('..\SparseCoding\fast_sparsecoding\code');
addpath('..\SparseCoding\sparsecode\l1magic');
addpath('..\SparseCoding\sparsecode\spgl1-1.6');
addpath('..\Visual');
addpath('..\SparseCoding\sparsecode');
patch_size = 8;
increment = 2;
load boatmanlasso20cosAW.mat;
image = imread('boatman.jpg');
A = dense_sampling(image, patch_size, increment);
num_patch = size(A,2);
avg_coeff = zeros(num_patch, 1);
for i = 1:1:num_patch
    avg_coeff = avg_coeff + result(i).x;
end
avg_coeff = avg_coeff ./ num_patch;

% analysis
saliency = zeros(size(image,1) + 10, size(image,2) + 10);
for i = 1:1:num_patch     
    % t = (result(i).x - avg_coeff)' * (result(i).x - avg_coeff);    

    % t = result(i).x - avg_coeff;
    %     for j = 1:1:size(t,1)
    %         if t(j) < 0
    %             t(j) = 0;
    %         end
    %     end

%     inverse = ones(size(avg_coeff,1), 1);
%     inverse = inverse .* max(abs(avg_coeff));
%     t = abs(result(i).x)' * (inverse - abs(avg_coeff));     
    t = result(i).mse;
    % t = t + 0.1 / sum(result(i).x);
    
    [y x] = PatchPosition(i, patch_size, increment, size(image));     
    saliency(x:x+patch_size, y:y+patch_size) = saliency(x:x+patch_size, y:y+patch_size) + sum(t);
end
saliency = saliency ./ num_patch;
real_img = saliency;
min_value = min(min(real_img));
max_value = max(max(real_img));
imshow(abs(real_img), [min_value max_value]);