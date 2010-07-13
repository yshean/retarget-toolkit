% slide 2 image extract only the overlap part
img1 = imread('0.jpg');
img2 = imread('2.jpg');
[height, width, channel] = size(img1);

bin = zeros(width-1,1);
% img1 to slide
for slide = 1:1:width-1
    overlap_width = width - slide;
    overlap1 = img1(:, 1:overlap_width, 1);
    overlap2 = img2(:, slide+1:width, 1);    
    count = get_identical_pixel(overlap1, overlap2);
    bin(slide, 1) = count;
end

figure
plot(bin, 'b');