clear all;

% add library path - change according to folder structure
addpath(genpath('..\..\..\SparseCodingLib'));
addpath(genpath('..\..\..\PatchTool'));
addpath(genpath('..\..\..\Images'));
addpath(genpath('..\..\..\ImageProcessing'));

% loading & setting
image = imread('boatman.jpg');

resized_image = DownSampling(image, 2, 'Gaussian', 3);
patch_size = 8;
increment = 4;

A = dense_sampling(image, patch_size, increment);
D_origin = dense_sampling(image, patch_size, increment);
num_patch = size(A,2);

% init result
result(num_patch).x = 0;
result(num_patch).mse = 0;
result(num_patch).method = 0;
result(num_patch).param = 0;

% for each patch, calculate the sparse code.
for i = 1:1:num_patch
    p = A(:,i);    
    %% don't change the Dictionary
    D = D_origin;
    
    %% suppress similar patch in the Dictionary
        % D = dense_sampling(resized_image, patch_size, increment);
        % change D a little bit Jul 2010
    D = SuppressDictionary(D, p, 'sigmoid');    
    
    
    [x temp] = DoSparseCoding(D, p, 'lasso', 20);
         
    result(i) = temp;
end

fprintf('finished');
% save
save('Data\car2small.jpgSigmoid20lasso20.mat','result');
 