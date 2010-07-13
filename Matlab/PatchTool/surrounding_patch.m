% Get surrounding patches in column
function A = surrounding_patch(image, x, y, patch_size, increment)
    %get surrouding patch
    surrounding = get_surrounding_patches(image, x, y, patch_size, overlap);
    % convert to col
    [w, h, d, n] = size(surrounding);
    col_surrounding = ones(patch_size * patch_size * channel, n);
    % col_surrounding = uint8(col_surrounding);
    for i = 1:1:n
        col_surrounding(:, i) = col_convert(surrounding(:,:,:,i));
    end
end