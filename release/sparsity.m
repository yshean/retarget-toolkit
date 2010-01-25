extraction_feature;
addpath('./l1magic');
addpath('./spgl1-1.6');

feature = dlmread('./data/2.desc', ' ', 2, 0);
feature = feature(:,1:128)';
img1 = imread('./data/2.jpg');
self_sparsity(img1,feature,32);