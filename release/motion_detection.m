% build dictionary of 100 dimensions
clear all;


img = imread('untitled.jpg'); 


[height, width, channel] = size(img);
code_size = width;

dictionary = zeros(code_size, 1);
for distance = 3:1:code_size - 1
    code = get_code(code_size, distance);
    
    dictionary = [dictionary, code];
%     repeat = 50 - distance;
%     for i = 1:1:repeat
%         dictionary = [dictionary, code];
%     end
end    
sumFinalX = zeros(code_size, 1);
for pos = 1:1:5

y = 1;
source = img(pos:pos + 1 - 1, y:y + code_size - 1, 1); 
 
% invert source
 
% white_img = ones(1, code_size) .* 255;
% white_img = uint8(white_img);
% source = white_img - source; 
b = zeros(code_size, 1);
for i=1:1:code_size
    b(i) = sum(source(:,i));
end
 
%a sample b to test
% for i=1:12:code_size
%     b(i) = b(i) + 60;
% end
% 
% for i=1:35:code_size
%     b(i) = b(i) + 60;
% end

%plot(b, 'r');

% minimize
sigma = 1;       % Desired ||Ax - b||_2
opts = spgSetParms('verbosity',1);
x = spg_bpdn(dictionary , b, sigma, opts);
%x = spg_lasso(dictionary, b, sigma, opts);

% sum freq in each bin
curr = 1;
bin = 3;
finalX = zeros(code_size, 1);
for distance = 3:1:code_size-1
    finalX(bin, 1) = sum(x(curr:curr+distance-1,1));
    curr = curr + distance;
    bin = bin + 1;
end    
sumFinalX(:, 1) = sumFinalX(:,1) + finalX(:,1);
end 
figure
plot(sumFinalX,'b');
legend('Recovered coefficients','Original coefficients');
title('(b) Basis Pursuit Denoise');
figure
imshow(img);
 