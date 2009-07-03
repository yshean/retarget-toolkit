function frames = bidirect_update_3d(frames,patch_size,complete_weight,cohere_weight,s_patch_weights,s_patches,s_matchId,t_patches,t_matchId)
weight_frames = zeros(size(frames{1}));

offset_x_range = [-(patch_size(1)-1)/2 : (patch_size(1)-1)/2];
offset_y_range = [-(patch_size(2)-1)/2 : (patch_size(2)-1)/2];
offset_t_range = [-(patch_size(3)-1)/2 : (patch_size(3)-1)/2];
[offset_y,offset_x,offset_z] = meshgrid(offset_y_range,offset_x_range,offset_t_range);
offset_x = offset_x(:)';
offset_y = offset_y(:)';
offset_z = offset_z(:)';

complete_centers = t_patches.centers(s_matchId,:);
x_pixel = repmat(complete_centers(:,1),[1,length(offset_x)])+repmat(offset_x,[size(complete_centers,1),1]);
y_pixel = repmat(complete_centers(:,2),[1,length(offset_y)])+repmat(offset_y,[size(complete_centers,1),1]);
t_pixel = repmat(complete_centers(:,3),[1,length(offset_z)])+repmat(offset_z,[size(complete_centers,1),1]);
ind = sub2ind(size(frames{1}),x_pixel(:),y_pixel(:),t_pixel(:));
val = (complete_weight*(1/size(s_patches.features,1))).*s_patches.features(:,1:patch_size(1)*patch_size(2)*patch_size(3));
A = accumarray(ind,val(:));
frames{1}(:) = A;
val = (complete_weight*(1/size(s_patches.features,1))).*s_patches.features(:,patch_size(1)*patch_size(2)*patch_size(3)+1:2*patch_size(1)*patch_size(2)*patch_size(3));
A = accumarray(ind,val(:));
frames{2}(:) = A;
val = (complete_weight*(1/size(s_patches.features,1))).*s_patches.features(:,2*patch_size(1)*patch_size(2)*patch_size(3)+1:end);
A = accumarray(ind,val(:));
frames{3}(:) = A;
A = accumarray(ind,complete_weight*(1/size(s_patches.features,1)).*ones(length(ind),1));
weight_frames(:) = A;

cohere_centers = t_patches.centers(:,:);
x_pixel = repmat(cohere_centers(:,1),[1,length(offset_x)])+repmat(offset_x,[size(cohere_centers,1),1]);
y_pixel = repmat(cohere_centers(:,2),[1,length(offset_y)])+repmat(offset_y,[size(cohere_centers,1),1]);
t_pixel = repmat(cohere_centers(:,3),[1,length(offset_z)])+repmat(offset_z,[size(cohere_centers,1),1]);
ind = sub2ind(size(frames{1}),x_pixel(:),y_pixel(:),t_pixel(:));
val = (cohere_weight*(1/size(t_patches.features,1))).*s_patches.features(t_matchId,1:patch_size(1)*patch_size(2)*patch_size(3));
A = accumarray(ind,val(:));
frames{1}(:) = frames{1}(:)+A;
val = (cohere_weight*(1/size(t_patches.features,1))).*s_patches.features(t_matchId,patch_size(1)*patch_size(2)*patch_size(3)+1:2*patch_size(1)*patch_size(2)*patch_size(3));
A = accumarray(ind,val(:));
frames{2}(:) = frames{2}(:)+A;
val = (cohere_weight*(1/size(t_patches.features,1))).*s_patches.features(t_matchId,2*patch_size(1)*patch_size(2)*patch_size(3)+1:end);
A = accumarray(ind,val(:));
frames{3}(:) = frames{3}(:)+A;
A = accumarray(ind,cohere_weight*(1/size(t_patches.features,1)).*ones(length(ind),1));
weight_frames(:) = weight_frames(:)+A;

weight_frames(weight_frames==0) = 0.1;
frames{1} = frames{1}./weight_frames;
frames{2} = frames{2}./weight_frames;
frames{3} = frames{3}./weight_frames;













