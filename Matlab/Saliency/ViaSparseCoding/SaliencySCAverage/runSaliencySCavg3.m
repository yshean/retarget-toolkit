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
load AW.mat
image = imread('cat.jpg');
patch_size = 8;
increment = 2;

% dense sampling to form dictionary A
P = dense_sampling(image, patch_size, increment);
 
num_coeff = size(A,2);
num_patch = size(P,2);

% init result
result(num_patch).x = 0;
result(num_patch).mse = 0;
result(num_patch).method = 0;
result(num_patch).param = 0;



% for each patch, calculate the sparse code.
for i = 1:1:size(P,2)
    p = P(:,i); 
    D = A;
    % change D a little bit Jul 2010
    cos = A' * p;
    cos = 1 - cos ./ norm(cos);
    cos = (cos - min(cos)) ./ (max(cos) - min(cos));
    for k = 1:1:size(A,1)
        D(k,:) = A(k,:) .* cos';
    end
    
    [x temp] = DoSparseCoding(D, p, 'fast_sc', 20);    
    result(i) = temp;
end

% save
save('catfast_sc20cosAW.mat','result');

% analysis
avg_coeff = zeros(num_coeff,1);
for i = 1:1:num_patch
    avg_coeff = avg_coeff + result(i).x;
end
avg_coeff = avg_coeff ./ num_patch;

% build saliency