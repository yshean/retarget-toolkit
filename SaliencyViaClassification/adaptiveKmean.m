function [group, patch_group] = adaptiveKmean(A, threshold)
    d = size(A);
    % starting with 1 group
    patch_col_size = d(1);
    group = zeros(1);
    patch_group = zeros(d(2), 1);
    centroid = ones(patch_col_size, 1);
    centroid(:,1) = A(:,1);
    for patch_index = 1:d(2)
        patch_col = A(:,patch_index);                
        centroid_size = size(centroid);
        min_distance = 1000000;
        min_centroid = 1;
        
        % calculate min distance between the patch and the
        % centroid
        for centroid_index = 1:centroid_size(2)            
            temp_distance = RotateMinDistance(patch_col, centroid(:,centroid_index));
            if temp_distance < min_distance
                min_distance = temp_distance;
                min_centroid = centroid_index;
            end                                    
        end             

        % check if the vector is too far
        if(min_distance > threshold)
            % creating new centroid
            centroid = [centroid, ones(patch_col_size, 1)];                    
            group = [group; 0];
            min_centroid = centroid_size(2) + 1;  
            size(group)
        end
        group(min_centroid) = group(min_centroid) + 1;
        patch_group(patch_index,:) = min_centroid;   
        
        % update centroid
        centroid(:, min_centroid) = centroid(:,min_centroid) .* ((group(min_centroid,1) - 1) / group(min_centroid,1)) + patch_col ./ group(min_centroid,1);
        patch_index
    end
end