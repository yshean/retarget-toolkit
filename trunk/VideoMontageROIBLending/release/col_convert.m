% convert an matrix to col
% input number of channel of the matrix for efficiency
% matrix = [col1, col2 .. coln] x 3 channel
% output =
% col1 (channel1)
% col2
% ..
% coln
% col1 (channel2)
% etc
%
function col = col_convert(matrix)    
    d = size(matrix);
    width = d(2);
    height = d(1);
    channel = d(3);
    col = ones(width * height * channel, 1);
    index = 1;
    for ch = 1:1:channel
        for i = 1:1:width
            for j = 1:1:height                        
                col(index) = matrix(j, i, ch);
                index = index + 1;
            end
        end
    end
return;