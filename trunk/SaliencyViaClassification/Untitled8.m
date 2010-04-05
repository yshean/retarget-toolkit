% test
index = 200;
[x,y] = PatchPosition(index, 4, 1, size(image));
patch = image(y:y + 4 -1, x:x + 4 -1, :);
patch_col = col_convert(patch);
test =A(:,index) - patch_col;