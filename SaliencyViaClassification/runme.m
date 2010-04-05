% perform classification to detect saliency


% using adaptive K-mean

image = imread('car2.jpg');

% =============================
% GROUP INFORMATION: 
% group: each group(index, 1) will store number of elments in the group
% 'index'
% =============================
% PATCH INFORMATION: patch_group
% each patch_group(index, 1) will store number of group the patch 'index'
% belong to

patch_size = 8;
overlap = 2;
load 'AW.mat'; % A and W will be loaded, W = A^-1

B = random_sampling(image, patch_size, overlap); % A is a list of patch col

C = W * B;

% classification 
[group, patch_group] = adaptiveKmean(C, 5000);

% assign the saliency based on number of item in group
d = size(group);
group_saliency = 1 ./ group;

%%
% accumulate saliency to patch
image_size = size(image);
saliency = zeros(image_size(1), image_size(2));
d2 = size(C);
for i = 1:d2(2)
    % get saliency of this patch
    group_id = patch_group(i);
    patch_saliency = group_saliency(group_id);    
    [x,y] = PatchPosition(i, patch_size, overlap, image_size);
    % 
    saliency(y:y+patch_size-1, x:x+patch_size-1) = saliency(y:y+patch_size-1, x:x+patch_size-1) + patch_saliency;
end

% stretch saliency to 0-255 range
max_element = max(max(saliency));
min_element = min(min(saliency));
saliency_size = size(saliency);

for i = 1:saliency_size(1)
    for j = 1:saliency_size(2)
        saliency(i,j) = (saliency(i,j) - min_element) / max_element * 255;
    end
end

