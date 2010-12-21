% testing
width = 20;
height = 30;
saliency = zeros(height, width);
patch_size = 8;
incre = 2;
value = 40;
for x= 1:incre:width - patch_size + 1
    for y = 1:incre:height - patch_size + 1        
       for i = x:1:x+patch_size-1
           for j = y:1:y+patch_size-1
               count = overlap_number(i, j, patch_size, width, height, incre);
               
%                if count == 0
%                    count = 1;
%                end
            
               saliency(j, i) = saliency(j, i) + value / count;
           end
       end
         
    end
end

real_img = saliency;
min_value = min(min(real_img));
max_value = max(max(real_img));
imshow(abs(real_img), [min_value max_value]);