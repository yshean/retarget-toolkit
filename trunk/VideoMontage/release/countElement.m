% count number of occurence of 'element' in matrix
function count = countElement(matrix, element)
    [height, width, channel] = size(matrix);
    count = 0;
    for x = 1:1:width
        for y = 1:1:height
            for z = 1:1:channel
                if matrix(y, x, z) == element
                    count = count + 1;
                end
            end
        end
    end
return;