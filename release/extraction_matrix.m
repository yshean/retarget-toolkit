% form a extraction matrix base on a mask
% e.g mask:
% [ 1 1 1 1 1 
%   1 1 1 0 0
%   1 1 1 0 0 ]
% -> omit all 0's entries
% extraction matrix is used to extract from column
% by cascade col of the target matrix
% @param: number of channel
function matrix = extraction_matrix(mask_matrix)
   [height, width, channel] = size(mask_matrix);
   % count the number of 1's in mask_matrix
   row = countElement(mask_matrix, 1); 
   matrix = zeros(row, width * height * channel);
   index = 1;
   for ch = 1:1:channel
        for x = 1:1:width
            for y = 1:1:height                
                pos = col_index(x, y, ch, width, height, channel);
                if mask_matrix(y, x, ch) == 1
                    matrix(index, pos) = 1;
                    index = index + 1;
                end
            end
        end
   end
   
return;