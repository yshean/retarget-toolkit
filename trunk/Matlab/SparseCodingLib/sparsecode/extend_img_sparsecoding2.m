% extend the image to the left
% using patch_size and overlap
function img = extend_img_sparsecoding2(input_img, A, patch_size, overlap)
    addpath('CVPR08SR/Sparse Coding');    
    lamba = 0.1;
    [height, width, channel] = size(input_img);    
    % create new image
    img = ones(height, width + 1, channel);
    img(1:1:height, 1:1:width, 1:1:channel) = input_img(1:1:height, 1:1:width, 1:1:channel);
    
    vertical_pos = 1;
    % first patch
    mask_matrix = ones(patch_size, patch_size, channel);
    for ch = 1:1:channel
        mask_matrix(:,:,ch) = [ 
            1 1 1 1 1 1 1 0;
            1 1 1 1 1 1 1 0;
            1 1 1 1 1 1 1 0;
            1 1 1 1 1 1 1 0;
            1 1 1 1 1 1 1 0;
            1 1 1 1 1 1 1 0;
            1 1 1 1 1 1 1 0;
            1 1 1 1 1 1 1 0;        
            ];
    end
    col_mask = extraction_matrix(mask_matrix);   
    patch = img(vertical_pos:1:vertical_pos+patch_size-1, width - patch_size + 2:1:width + 1, 1:1:channel);    
    patch_col = col_convert(patch);
    patch_masked = col_mask * patch_col; 
    coeff = sparse_coding2(col_mask * A, patch_masked, lamba, []);  
    result = A * coeff;
    patch = col_to_matrix(result, patch_size, patch_size, channel);
    
    % img(vertical_pos:1:vertical_pos + patch_size - 1, width - patch_size + 2:1:width + 1, 1:1:channel) = patch(1:1:patch_size, 1:1:patch_size, 1:1:channel);
    img(vertical_pos:1:vertical_pos + patch_size - 1, width + 1, 1:1:channel) = patch(1:1:patch_size, patch_size, 1:1:channel);
    % from second patch
     for ch = 1:1:channel
        mask_matrix(:,:,ch) = [ 
            1 1 1 1 1 1 1 0;
            1 1 1 1 1 1 1 0;
            1 1 1 1 1 1 1 0;
            1 1 1 1 1 1 1 0;
            1 1 1 1 1 1 1 0;
            1 1 1 1 1 1 1 0;
            1 1 1 1 1 1 1 0;
            1 1 1 1 1 1 1 0;        
            ];
    end
    mask_matrix(1:1:overlap, patch_size, 1:1:channel) = 1;    
    col_mask = extraction_matrix(mask_matrix);     
    masked_A = col_mask * A;
    vertical_pos = vertical_pos + patch_size - overlap;
    
    while vertical_pos + patch_size - 1 <= height
        patch = img(vertical_pos:1:vertical_pos + patch_size - 1,width - patch_size + 2:1:width + 1, 1:1:channel);
        patch_col = col_convert(patch);
        patch_masked = col_mask * patch_col; 
        coeff = sparse_coding2(masked_A, patch_masked, lamba, []);
        result = A * coeff;
        patch = col_to_matrix(result, patch_size, patch_size, channel);
        
%         figure(1);
%         imshow(uint8(patch));
%         figure(2);
%         imshow(uint8(img(vertical_pos:1:vertical_pos + patch_size - 1, width - patch_size + 2:1:width + 1, 1:1:channel)));
        
        % img(vertical_pos:1:vertical_pos + patch_size - 1, width - patch_size + 2:1:width + 1, 1:1:channel) = patch(1:1:patch_size, 1:1:patch_size, 1:1:channel);
        img(vertical_pos:1:vertical_pos + patch_size - 1, width + 1, 1:1:channel) = patch(1:1:patch_size, patch_size, 1:1:channel);
        vertical_pos = vertical_pos + patch_size - overlap;
    end
    
    
return;