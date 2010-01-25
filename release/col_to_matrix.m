% convert a col of pixel to matrix
function matrix = col_to_matrix(col, width, height, channel)
    matrix = ones(height, width, channel);    
    
    for ch = 1:1:channel
        for i = 1:1:width
            for j = 1:1:height                        
                index = col_index(i, j, ch, width, height, channel);
                matrix(j, i, ch) = col(index);
            end
        end
    end
    
return;