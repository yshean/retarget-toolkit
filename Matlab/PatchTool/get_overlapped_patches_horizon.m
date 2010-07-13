% get overlapped patches with size w * w
% return patches = w * w * d * n matrix, n is the size
function patches = get_overlapped_patches_horizon(img, x, y, w, h, increment)
    [width, height, d] = size(img);    
    % invalid case
    if(x < 0 || y < 0 || x + w > width || y + h > height)
        patches = 0;
        return;
    end
    
    % counting number of patches
    n = floor( (w - h) / increment ) + 1;   
    patches = ones(h, h, d, n);
    
    current_x = x;    
    for i = 1:1:n
        patches(:,:,:,i) = img(current_x:1:current_x+h-1, y:1:y+h-1, :);
        current_x = current_x + increment;
    end
end