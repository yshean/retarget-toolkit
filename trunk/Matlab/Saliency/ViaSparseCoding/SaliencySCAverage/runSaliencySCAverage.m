clear all;

addpath('..\SampleImage');
addpath('..\PatchTool');
addpath('..\SparseCoding');
addpath('..\SparseCoding\ICL');
addpath('..\SparseCoding\fast_sparsecoding\code');
addpath('..\SparseCoding\sparsecode\l1magic');
addpath('..\SparseCoding\sparsecode\spgl1-1.6');
addpath('..\Visual');
addpath('..\SparseCoding\sparsecode');

% setting
patch_size = 8;
increment = 2;

image = imread('boatman.jpg');
figure(1);
imshow(image);

% taking mouse position
patch_num = 3;
[mouse_x, mouse_y] = ginput(patch_num);

% get patch_col
patch_col = col_convert(image(mouse_y(1):mouse_y(1) + patch_size - 1, mouse_x(1):mouse_x(1) + patch_size - 1,:));
for i = 2:patch_num    
    patch_col = [patch_col col_convert(image(mouse_y(i):mouse_y(i) + patch_size - 1, mouse_x(i):mouse_x(i) + patch_size - 1,:))];        
end

 non_zero = zeros(1, patch_num);
 

% create an average patch
A = dense_sampling(image, patch_size, increment);
avg_patch = mean(A,2);

for p = 1:1:patch_num    
    % diff = patch_col(p) - avg_patch;
    exclude = image;  
    exclude(mouse_y(i):mouse_y(i) + patch_size - 1,mouse_x(i):mouse_x(i) + patch_size - 1, :) = 0;
    A = dense_sampling(exclude, patch_size, increment);
    % [x, result] = DoSCDivideDictionary(A, diff, part_num);    
    [x, result] = DoSparseCoding(A, patch_col(p), 'bpdn', 50);
    DisplaySparseCodingResult(result, p);
end