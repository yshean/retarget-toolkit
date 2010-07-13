% get surrouding patches inform of size*size*n matrix
% extending the patches by 'size' and overlap
% n is number of patches
% bouding IS NOT handle, 
function patches = get_surrouding_patches(img, x, y, size, increment)
    % calculate the starting position of the surrounding
    x_surrouding = x - size;
    y_surrouding = y - size;
    
    patches1 = get_overlapped_patches_horizon(img, x_surrouding, y_surrouding, size * 3, size, increment);
    patches2 = get_overlapped_patches_horizon(img, x_surrouding, y + size, size * 3, size, increment);
    patches3 = get_overlapped_patches_vertical(img, x_surrouding, y, size, size, increment);
    patches4 = get_overlapped_patches_vertical(img, x + size, y, size, size, increment);
 
    patches = cat(4, patches1, patches2, patches3, patches4);
end