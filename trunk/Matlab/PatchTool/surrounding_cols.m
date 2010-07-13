% Get surrounding patches in column
function A = surrounding_cols(image, x, y, patch_size, overlap)
    [h, w, channel] = size(image);
    %get surrouding patch
    surrounding = get_surrouding_patches(image, x, y, patch_size, overlap);
    % convert to col
    [w, h, d, n] = size(surrounding);
    A = ones(patch_size * patch_size * channel, n);
    % col_surrounding = uint8(col_surrounding);
    for i = 1:1:n
        A(:, i) = col_convert(surrounding(:,:,:,i));
    end
end