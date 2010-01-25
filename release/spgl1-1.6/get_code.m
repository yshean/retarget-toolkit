% distance should be <= size
% code is an array of shifting all possible positions of 2 signals
% which have distance of 'distance'
function [code] = get_code(size, distance)  
freq = size / distance;
freq = floor(freq);
 
code = zeros(size, distance); 

for i = 1:1:distance          
    code(i, i) = 1; 
    for j = 1:1:freq
        index = i + j*distance;
        if(index <= size)
            code(index, i) = 1;
        end
    end
end

