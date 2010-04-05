image_size = size(image);
saliency = zeros(image_size(1), image_size(2));
d2 = size(A);
for i = 1:d2(2) -300
    % get saliency of this patch
    group_id = patch_group(i);
    patch_saliency = group_saliency(group_id);    
    [x,y] = PatchPosition(i, patch_size, overlap, image_size);
    % 
    saliency(y:y+patch_size-1, x:x+patch_size-1) = saliency(y:y+patch_size-1, x:x+patch_size-1) + patch_saliency;
end

% stretch saliency to 0-255 range
max_element = max(max(saliency));
min_element = min(min(saliency));
saliency_size = size(saliency);

for i = 1:saliency_size(1)
    for j = 1:saliency_size(2)
        saliency(i,j) = (saliency(i,j) - min_element) / max_element * 255;
    end
end
