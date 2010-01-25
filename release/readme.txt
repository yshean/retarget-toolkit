extraction_feature;
feature = dlmread('./data/1.desc', ' ', 2, 0);
feature = feature(:,1:128)';
img1 = imread('./data/1.jpg');
self_sparsity(img1,feature,32);