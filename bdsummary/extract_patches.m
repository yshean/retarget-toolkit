function patches=extract_patches(img,patch_size)

% load config file
eval('config_file');

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
x = x(:);
y = y(:);
patches.centers=[x,y];

% calculate NN
patches.nn = cell(size(patches.centers,1),1);
for i = 1 : size(patches.centers,1)
    center = patches.centers(i,:);
    nn_x_range = [max((patch_size(1)-1)/2+1,center(1)-R):min(size(img,1)-(patch_size(1)-1)/2,center(1)+R)];
    nn_y_range = [max((patch_size(2)-1)/2+1,center(2)-R):min(size(img,2)-(patch_size(2)-1)/2,center(2)+R)];
    [nn_X,nn_Y]=meshgrid(nn_x_range,nn_y_range);
    nn_points = [nn_X(:),nn_Y(:)];
    patches.nn{i} = (nn_points(:,1)-x_range(1)).*length(y_range)+nn_points(:,2)-y_range(1)+1;
end

Lfeatures = []; Afeatures = []; Bfeatures = [];
for j = -(patch_size(1)-1)/2 : (patch_size(1)-1)/2
    for i = -(patch_size(1)-1)/2 : (patch_size(1)-1)/2    
        x_pixel = x(:)+i;
        y_pixel = y(:)+j;
        ind = sub2ind([size(img,1),size(img,2)],x_pixel,y_pixel);
        
        Lfeatures = [Lfeatures,Limg(ind)];
        Afeatures = [Afeatures,Aimg(ind)];
        Bfeatures = [Bfeatures,Bimg(ind)];
    end
end
patches.features = [Lfeatures,Afeatures,Bfeatures];

% x_lbound = x(:)-(patch_size(1)-1)/2;
% x_ubound = x(:)+(patch_size(1)-1)/2;
% y_lbound = y(:)-(patch_size(1)-1)/2;
% y_ubound = y(:)+(patch_size(1)-1)/2;
% 
% patches.centers = [];
% patches.features = [];
% for i = 1 : length(x_lbound)
%     patches.centers = [patches.centers;[x(i),y(i)]];
%     patches.features = [patches.features;reshape(img(x_lbound(i):x_ubound(i),y_lbound(i):y_ubound(i),:),[1,patch_size(1)*patch_size(2)*3])];
% end
