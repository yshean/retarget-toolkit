% take 2 images, measure the number of pixels which are identical
function [id_pixel] = get_identical_pixel(img1, img2)
[height, width, channel] = size(img1);
id_pixel = 0;
for w = 1:1:width
    for h = 1:1:height
        diff = img1(h,w,1) - img2(h,w,1);
        if(diff == 0)
            id_pixel = id_pixel + 1;
        end
    end
end

id_pixel = double(id_pixel);
id_pixel = id_pixel / (height*width);