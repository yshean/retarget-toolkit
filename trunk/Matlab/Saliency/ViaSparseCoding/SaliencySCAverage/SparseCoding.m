addpath c:\documents and settings\kidphys\my documents\matlab\cvx\cvx
addpath c:\documents and settings\kidphys\my documents\matlab\cvx\cvx\structures
addpath c:\documents and settings\kidphys\my documents\matlab\cvx\cvx\lib
addpath c:\documents and settings\kidphys\my documents\matlab\cvx\cvx\functions
addpath c:\documents and settings\kidphys\my documents\matlab\cvx\cvx\commands
addpath c:\documents and settings\kidphys\my documents\matlab\cvx\cvx\builtins


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
image = imread('car2small.jpg');
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
    
    
        cvx_begin
        variable x_l1(size(A,2))
        minimize( norm( x_l1, 1 ) )
        subject to
        A*x_l1 == p;
        x_l1 > 0;
        cvx_end

 
end

% save
save('car2smallbpdn20.mat','result');

% analysis
avg_coeff = zeros(num_patch,1);
for i = 1:1:num_patch
    avg_coeff = avg_coeff + result(i).x;
end
avg_coeff = avg_coeff ./ num_patch;



