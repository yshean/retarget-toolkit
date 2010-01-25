function [img] = get_motion_image()
img1 = imread('0.jpg');
img2 = imread('2.jpg');
img3 = imread('4.jpg');
img1 = img1 ./ 3;
img2 = img2 ./ 3;
img3 = img3 ./ 3;
img = img1 + img2 + img3;
imwrite(img, 'merge.jpg', 'jpg');
