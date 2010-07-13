% get overlapped patches with size w * w
% return patches = w * w * d * n matrix, n is the size
function patches = get_overlapped_patches_vertical(img, x, y, w, h, increment)
    [width, height, d] = size(img);    
    % invalid case
    if(x < 0 || y < 0 || x + w > width || y + h > height)
        patches = 0;
        return;
    end
    
    % counting number of patches
    n = floor( (h - w) / increment ) + 1;
    patches = ones(w, w, d, n);
    
    current_y = y;    
    for i = 1:1:n
        patches(:,:,:,i) = img(current_y:1:current_y+w-1, x:1:x+w-1, :);
        current_y = current_y + increment;
    end
end