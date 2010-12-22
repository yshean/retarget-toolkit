% addpath(genpath('..\ksvdbox13'));
% addpath('..\nips08');
% addpath('..\..\k-svd\ompbox10');
% addpath('..\..\..\Images');
% addpath('..\..\..\PatchTool');
% 
image = imread('boatman.jpg');
img_bw = rgb2gray(image);

patch_size = 8;
patch_increment = 4;

[patches1 count1 count2 count3] = gatherPatches(image, patch_size, patch_increment);
patches1 = normcols(patches1);

h = fspecial('Sobel');
img_bw = imfilter(img_bw, h); 

patches2 = gatherPatches(img_bw, patch_size, patch_increment);
patches2 = normcols(patches2);
 

%patches = [patches1; patches2];
patches = patches1;
for i = 1:1:size(patches,2)
    m = mean(patches(:,i));
    patches(:,i) = patches(:,i) - m;
end

params.codemode = 'sparsity';
params.data = patches;
params.Edata = 0.2;
params.Tdata = 5;
params.dictsize = 100;
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


% how excited each code
% weight = zeros(size(gamma,1), 1);
% for i = 1:1:size(gamma,2)
%     weight(:,1) = weight(:,1) + abs(gamma(:,i));
% end
% plot(weight);

saliency = zeros(size(image,1), size(image,2));
for i = 1:1:count1
    diff_vec = D * avg - D * gamma(:,i);
%     value = 0;
%     for k = 1:1:size(gamma,1)
%         if( gamma(k,i) > 0)
%             value = value + 1;
%         end
%     end
    %value = sum(gamma(:,1));
    % value = diff_vec' * diff_vec;
    value = weight' * abs(gamma(:,1));
    % value = (avg - gamma(:,1))' * (avg - gamma(:,1));
    %value = 10;
    % value = myEnergy(i);
    
    [y x] = patch_position(i, patch_size, patch_increment, size(image));    
    for m = x:1:x+patch_size-1
        for n = y:1:y+patch_size-1 
            if m == 108
                m = 1;
            end
            count = overlap_number(n, m, patch_size, size(image, 2), size(image,1), patch_increment);
            saliency(m,n) = saliency(m,n) + value / count;
        end
    end     
end

% imshow(saliency, []);
saliency = saliency ./ size(patches1,2);
real_img = saliency;
min_value = min(min(real_img));
max_value = max(max(real_img));
imshow(abs(real_img), [min_value max_value]);