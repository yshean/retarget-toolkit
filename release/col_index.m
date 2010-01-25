% calculate the col_index of a pixel after converting to col
% - width/height/channel: width, height and number of channels of a patch
% - x, y, ch: position and channel of the pixel (start from 1)
function index = col_index(x, y, ch, width, height, channel)
    index = 0;
    index = index + width * height * (ch - 1);
    index = index + height * (x - 1);
    index = index + y;
return;