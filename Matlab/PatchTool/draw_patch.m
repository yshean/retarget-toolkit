% depends on ..\Visual\drawRectangle.mat
function result = draw_patch(image, patch_size, increment, patch_index, color)
addpath('..\Visual');
addpath('..\PatchTool');
d = size(image);
[x,y] = PatchPosition(patch_index, patch_size, increment, d);
result = DrawRectangle(x, y, patch_size, patch_size, image, color); 
end