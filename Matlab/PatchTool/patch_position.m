% recover the top left position of the patch
% assuming the scan is from  top->bottom and left-right
function [x, y] = patch_position(patch_index, patch_size, overlap, image_size)
    % how many patches are there in the row?
    row_no = floor((image_size(2) - patch_size + 2)/overlap);
    % how many patches are there in the col?
    col_no = floor((image_size(1) - patch_size + 2)/overlap);
    
    % calculating patch_index
    col = floor((patch_index - 1) / col_no) + 1;
    row = patch_index - (col - 1) * col_no;
    
    % calculating the actual patch position
    x = (col - 1) * overlap + 1;
    y = (row - 1) * overlap + 1; 
end