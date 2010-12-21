% addpath(genpath('..\ksvdbox13'));
% addpath('..\nips08');
% addpath('..\..\k-svd\ompbox10');
% addpath('..\..\..\Images');
% addpath('..\..\..\PatchTool');
% 
image = imread('boatman.jpg');
% image = img;
img_1 = imresize(image, 0.5);
% imshow(img_w);
img_2 = imresize(img_1, 0.5);
patch_size = 8;
patch_increment = 2;

patches1 = dense_sampling(image, patch_size, patch_increment);
patches2 = dense_sampling(img_1, patch_size, patch_increment);
patches3 = dense_sampling(img_2, patch_size, patch_increment);

patches = [patches1 patches2 patches3];
%patches = normcols(patches);
for i = 1:1:size(patches,2)
    m = mean(patches(:,i));
    patches(:,i) = patches(:,i) - m;
end

params.codemode = 'sparsity';
params.data = patches;
params.Edata = 0.2;
params.Tdata = 3;
params.dictsize = 20;
params.iternum = 30;
params.memusage = 'high';

[D, gamma, err] = ksvd(params, '');

avg = zeros(size(gamma,1), 1);
for i = 1:1:size(gamma,2)
    avg = avg + gamma(:,1);
end
avg = avg / size(gamma,2);
% plot(avg);

myEnergy = GetEnergy(gamma);

saliency = zeros(size(image,1), size(image,2));
for i = 1:1:size(patches1, 2)
    diff_vec = D * avg - D * gamma(:,i);
%     value = 0;
%     for k = 1:1:size(gamma,1)
%         if( gamma(k,i) > 0)
%             value = value + 1;
%         end
%     end
    %value = sum(gamma(:,1));
    value = diff_vec' * diff_vec;
    %value = 10;
    % value = myEnergy(i);
    
    [y x] = patch_position(i, patch_size, patch_increment, size(image));    
    for m = x:1:x+patch_size-1
        for n = y:1:y+patch_size-1
            count = overlap_number(n, m, patch_size, size(image, 2), size(image,1), patch_increment);
            saliency(m, n) = saliency(m, n) + value / count;
        end
    end     
end

% imshow(saliency, []);
saliency = saliency ./ size(patches1,2);
real_img = saliency;
min_value = min(min(real_img));
max_value = max(max(real_img));
imshow(abs(real_img), [min_value max_value]);