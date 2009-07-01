function patches=extract_patches(img,patch_size,R)

% patches.centers = [];
% patches.features = [];
% for i = (patch_size(1)-1)/2+1 : size(img,1)-(patch_size(1)-1)/2
%     for j = (patch_size(2)-1)/2+1 : size(img,2)-(patch_size(2)-1)/2
%         patches.centers = [patches.centers;[i,j]];
%         patches.features = [patches.features;reshape(img(i-(patch_size(1)-1)/2:i+(patch_size(1)-1)/2,...
%                             j-(patch_size(2)-1)/2:j+(patch_size(2)-1)/2,:),[1,patch_size(1)*patch_size(2)*3])];
%     end
% end

Limg = img(:,:,1);
Aimg = img(:,:,2);
Bimg = img(:,:,3);

x_range = [(patch_size(1)-1)/2+1 : size(img,1)-(patch_size(1)-1)/2];
y_range = [(patch_size(2)-1)/2+1 : size(img,2)-(patch_size(2)-1)/2];

[x,y] = meshgrid(x_range,y_range);
% [y,x] = meshgrid(y_range,x_range);
x = x(:);
y = y(:);
patches.centers=[x,y];

% calculate NN
patches.nn = cell(size(patches.centers,1),1);
for i = 1 : size(patches.centers,1)
    center = patches.centers(i,:);
    nn_x_range = [max((patch_size(1)-1)/2+1,center(1)-R):min(size(img,1)-(patch_size(1)-1)/2,center(1)+R)];
    nn_y_range = [max((patch_size(2)-1)/2+1,center(2)-R):min(size(img,2)-(patch_size(2)-1)/2,center(2)+R)];
    [nn_Y,nn_X]=meshgrid(nn_y_range,nn_x_range);
    nn_points = [nn_X(:),nn_Y(:)];
    patches.nn{i} = (nn_points(:,1)-x_range(1)).*length(y_range)+nn_points(:,2)-y_range(1)+1;
    % patches.nn{i} = (nn_points(:,2)-y_range(1)).*length(x_range)+nn_points(:,1)-x_range(1)+1;
end

offset_x_range = [-(patch_size(1)-1)/2 : (patch_size(1)-1)/2];
offset_y_range = [-(patch_size(2)-1)/2 : (patch_size(2)-1)/2];
[offset_y,offset_x] = meshgrid(offset_y_range,offset_x_range);
offset_x = offset_x(:)';
offset_y = offset_y(:)';

x_pixel = repmat(x(:),[1,length(offset_x)])+repmat(offset_x,[length(x),1]);
y_pixel = repmat(y(:),[1,length(offset_y)])+repmat(offset_y,[length(y),1]);

ind = sub2ind(size(Limg),x_pixel(:),y_pixel(:));
Lfeatures = reshape(Limg(ind),[size(patches.centers,1),length(offset_x)]); 
Afeatures = reshape(Aimg(ind),[size(patches.centers,1),length(offset_x)]); 
Bfeatures = reshape(Bimg(ind),[size(patches.centers,1),length(offset_x)]); 

patches.features = [Lfeatures,Afeatures,Bfeatures];