clear all;

% add library path - change according to folder structure
addpath(genpath('..\..\..\SparseCodingLib'));
addpath(genpath('..\..\..\PatchTool'));
addpath(genpath('..\..\..\Images'));
addpath(genpath('..\..\..\ImageProcessing'));

% loading & setting
image = imread('boatman.jpg');

resized_image = DownSampling(image, 3, 'Gaussian', 5);
patch_size = 8;
increment = 2;
D = dense_sampling(resized_image, patch_size, increment);
A = dense_sampling(image, patch_size, increment);

num_patch = size(A,2);

% init result
result(num_patch).x = 0;
result(num_patch).mse = 0;
result(num_patch).method = 0;
result(num_patch).param = 0;

% for each patch, calculate the sparse code.
for i = 1:1:num_patch
    p = A(:,i);    
    
%     % change D a little bit Jul 2010
%     cos = D' * p;
%     cos = 1 - cos ./ norm(cos);
%     cos = (cos - min(cos)) ./ (max(cos) - min(cos));
%     for k = 1:1:size(D,1)
%         D(k,:) = D(k,:) .* cos';
%     end
    
    [x temp] = DoSparseCoding(D, p, 'lasso', 20);
    
    % add a 0 to the coeff   
    result(i) = temp;
end

% save
save('Data\boatmanlasso20downsampling.mat','result');
 