% extract a left region of a patch 
% input:
% - size: size of the patch
% - channel: number of channel
% - reduce_size: new size ( < size)
% usage: create new (reduced) Dictionary
% matrix * D = new_D;
function matrix = roi_matrix_left(size, channel, reduce_size)
    old_size = size * size * channel;
    new_size = reduce_size * size * channel;
    matrix = zeros (new_size, old_size);
        
    index = 1;
    for ch = 1:1:channel
        for x = 1:1:reduce_size
            for y = 1:1:size           
                 pos = col_index(x, y, ch, size, size, channel);
                 matrix(index, pos) = 1;
                 index = index + 1;
            end
        end
    end
return;