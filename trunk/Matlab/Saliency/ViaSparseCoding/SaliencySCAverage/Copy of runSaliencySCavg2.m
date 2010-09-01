clear all;

% add library path
addpath('..\SampleImage');
addpath('..\PatchTool');
addpath('..\SparseCoding');
addpath('..\SparseCoding\ICL');
addpath('..\SparseCoding\fast_sparsecoding\code');
addpath('..\SparseCoding\sparsecode\l1magic');
addpath('..\SparseCoding\sparsecode\spgl1-1.6');
addpath('..\Visual');
addpath('..\SparseCoding\sparsecode');

% loading & setting
image = imread('boatman.jpg');
patch_size = 8;
increment = 2;

% dense sampling to form dictionary A
A = dense_sampling(image, patch_size, increment);
num_patch = size(A,2);

% init result
result(num_patch).x = 0;
result(num_patch).mse = 0;
result(num_patch).method = 0;
result(num_patch).param = 0;

% for each patch, calculate the sparse code.
for i = 1:1:size(A,2)
    p = A(:,i);
    D = A; % duplicate the dictionary
    D(:,i) = []; % reject the patch from the dictionary
    
    % change D a little bit Jul 2010
    cos = D' * p;
    cos = 1 - cos ./ norm(cos);
    cos = (cos - min(cos)) ./ (max(cos) - min(cos));
    for k = 1:1:size(D,1)
        D(k,:) = D(k,:) .* cos';
    end
    
    [x temp] = DoSparseCoding(D, p, 'lasso', 20);
    
    % add a 0 to the coeff
    temp.x = zeros(num_patch,1);
    temp.x(1:i-1,1) = x(1:i-1,1);
    temp.x(i,1) = 0;
    temp.x(i+1:num_patch,1) = x(i:num_patch-1,1);    
    result(i) = temp;
end

% save
save('boatmanlasso20cos.mat','result');

% analysis
avg_coeff = zeros(num_patch,1);
for i = 1:1:num_patch
    avg_coeff = avg_coeff + result(i).x;
end
avg_coeff = avg_coeff ./ num_patch;

% build saliency