function patches=extract_3d_patches(frames,patch_size)

% load config file
eval('config_file');

Limg = frames{1};
Aimg = frames{2};
Bimg = frames{3};

x_range = [(patch_size(1)-1)/2+1 : size(Limg,1)-(patch_size(1)-1)/2];
y_range = [(patch_size(2)-1)/2+1 : size(Limg,2)-(patch_size(2)-1)/2];
t_range = [(patch_size(3)-1)/2+1 : size(Limg,3)-(patch_size(3)-1)/2];

% [x,y,z] = meshgrid(x_range,y_range,t_range);
[y,x,z] = meshgrid(y_range,x_range,t_range);
x = x(:);
y = y(:);
z = z(:);
patches.centers=[x,y,z];

% calculate NN
patches.nn = cell(size(patches.centers,1),1);
for i = 1 : size(patches.centers,1)
    center = patches.centers(i,:);
    nn_x_range = [max((patch_size(1)-1)/2+1,center(1)-R):min(size(Limg,1)-(patch_size(1)-1)/2,center(1)+R)];
    nn_y_range = [max((patch_size(2)-1)/2+1,center(2)-R):min(size(Limg,2)-(patch_size(2)-1)/2,center(2)+R)];
    nn_t_range = [max((patch_size(3)-1)/2+1,center(3)-R):min(size(Limg,3)-(patch_size(3)-1)/2,center(3)+R)];
    [nn_Y,nn_X,nn_T]=meshgrid(nn_y_range,nn_x_range,nn_t_range);
    nn_points = [nn_X(:),nn_Y(:),nn_T(:)];
%     patches.nn{i} = (nn_points(:,1)-x_range(1)).*(length(y_range)*length(t_range))+...
%                     (nn_points(:,2)-y_range(1)).*length(t_range)+nn_points(:,3)-t_range(1)+1;
    patches.nn{i} = (nn_points(:,3)-t_range(1)).*(length(x_range)*length(y_range))+...
                    (nn_points(:,2)-y_range(1)).*length(x_range)+nn_points(:,1)-x_range(1)+1;
end

offset_x_range = [-(patch_size(1)-1)/2 : (patch_size(1)-1)/2];
offset_y_range = [-(patch_size(2)-1)/2 : (patch_size(2)-1)/2];
offset_t_range = [-(patch_size(3)-1)/2 : (patch_size(3)-1)/2];
[offset_y,offset_x,offset_z] = meshgrid(offset_y_range,offset_x_range,offset_t_range);
offset_x = offset_x(:)';
offset_y = offset_y(:)';
offset_z = offset_z(:)';

x_pixel = repmat(x(:),[1,length(offset_x)])+repmat(offset_x,[length(x),1]);
y_pixel = repmat(y(:),[1,length(offset_y)])+repmat(offset_y,[length(y),1]);
t_pixel = repmat(z(:),[1,length(offset_z)])+repmat(offset_z,[length(z),1]);

ind = sub2ind(size(Limg),x_pixel(:),y_pixel(:),t_pixel(:));
Lfeatures = Limg(ind); 
Afeatures = Aimg(ind); 
Bfeatures = Bimg(ind);

patches.features = [Lfeatures,Afeatures,Bfeatures];