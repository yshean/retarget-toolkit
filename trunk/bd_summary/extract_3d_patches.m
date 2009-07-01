function patches=extract_3d_patches(frames,patch_size)

% load config file
eval('config_file');

Limg = frames{1};
Aimg = frames{2};
Bimg = frames{3};

x_range = [(patch_size(1)-1)/2+1 : size(Limg,1)-(patch_size(1)-1)/2];
y_range = [(patch_size(2)-1)/2+1 : size(Limg,2)-(patch_size(2)-1)/2];
t_range = [(patch_size(3)-1)/2+1 : size(Limg,3)-(patch_size(3)-1)/2];

[x,y,z] = meshgrid(x_range,y_range,t_range);
% [y,x,z] = meshgrid(y_range,x_range,t_range);
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
    patches.nn{i} = (nn_points(:,1)-x_range(1)).*(length(y_range)*length(t_range))+...
                    (nn_points(:,2)-y_range(1)).*length(t_range)+nn_points(:,3)-t_range(1)+1;
%     patches.nn{i} = (nn_points(:,3)-t_range(1)).*(length(x_range)*length(y_range))+...
%                     (nn_points(:,2)-y_range(1)).*length(x_range)+nn_points(:,1)-x_range(1)+1;
end

Lfeatures = []; Afeatures = []; Bfeatures = [];
for t = -(patch_size(3)-1)/2 : (patch_size(3)-1)/2
    for j = -(patch_size(2)-1)/2 : (patch_size(2)-1)/2
        for i = -(patch_size(1)-1)/2 : (patch_size(1)-1)/2                     
            x_pixel = x(:)+i;
            y_pixel = y(:)+j;
            t_pixel = z(:)+t;
            ind = sub2ind(size(Limg),x_pixel,y_pixel,t_pixel);
        
            Lfeatures = [Lfeatures,Limg(ind)];
            Afeatures = [Afeatures,Aimg(ind)];
            Bfeatures = [Bfeatures,Bimg(ind)];
        end
    end
end
patches.features = [Lfeatures,Afeatures,Bfeatures];
