function img = bidirect_update_2d(img,patch_size,complete_weight,cohere_weight,s_patch_weights,s_patches,s_matchId,t_patches,t_matchId)

weight_img = zeros(size(img,1),size(img,2));

offset_x_range = [-(patch_size(1)-1)/2 : (patch_size(1)-1)/2];
offset_y_range = [-(patch_size(2)-1)/2 : (patch_size(2)-1)/2];
[offset_y,offset_x] = meshgrid(offset_y_range,offset_x_range);
offset_x = offset_x(:)';
offset_y = offset_y(:)';

complete_centers = t_patches.centers(s_matchId,:);
x_pixel = repmat(complete_centers(:,1),[1,length(offset_x)])+repmat(offset_x,[size(complete_centers,1),1]);
y_pixel = repmat(complete_centers(:,2),[1,length(offset_y)])+repmat(offset_y,[size(complete_centers,1),1]);
ind = sub2ind([size(img,1),size(img,2)],x_pixel(:),y_pixel(:));
val = (complete_weight*(1/size(s_patches.features,1))).*repmat(s_patch_weights,[1,patch_size(1)*patch_size(2)]).*s_patches.features(:,1:patch_size(1)*patch_size(2));
A = accumarray(ind,val(:));
A = [A;zeros(size(img,1)*size(img,2)-length(A),1)];
img(:,:,1) = reshape(A,[size(img,1),size(img,2)]);
val = (complete_weight*(1/size(s_patches.features,1))).*repmat(s_patch_weights,[1,patch_size(1)*patch_size(2)]).*s_patches.features(:,patch_size(1)*patch_size(2)+1:2*patch_size(1)*patch_size(2));
A = accumarray(ind,val(:));
A = [A;zeros(size(img,1)*size(img,2)-length(A),1)];
img(:,:,2) = reshape(A,[size(img,1),size(img,2)]);
val = (complete_weight*(1/size(s_patches.features,1))).*repmat(s_patch_weights,[1,patch_size(1)*patch_size(2)]).*s_patches.features(:,2*patch_size(1)*patch_size(2)+1:end);
A = accumarray(ind,val(:));
A = [A;zeros(size(img,1)*size(img,2)-length(A),1)];
img(:,:,3) = reshape(A,[size(img,1),size(img,2)]);
val = (complete_weight*(1/size(s_patches.features,1))).*repmat(s_patch_weights,[1,patch_size(1)*patch_size(2)]);
A = accumarray(ind,val(:));
A = [A;zeros(size(img,1)*size(img,2)-length(A),1)];
weight_img = reshape(A,[size(img,1),size(img,2)]);

cohere_centers = t_patches.centers(:,:);
x_pixel = repmat(cohere_centers(:,1),[1,length(offset_x)])+repmat(offset_x,[size(cohere_centers,1),1]);
y_pixel = repmat(cohere_centers(:,2),[1,length(offset_y)])+repmat(offset_y,[size(cohere_centers,1),1]);
ind = sub2ind([size(img,1),size(img,2)],x_pixel(:),y_pixel(:));
val = (cohere_weight*(1/size(t_patches.features,1))).*repmat(s_patch_weights(t_matchId),[1,patch_size(1)*patch_size(2)]).*s_patches.features(t_matchId,1:patch_size(1)*patch_size(2));
A = accumarray(ind,val(:));
A = [A;zeros(size(img,1)*size(img,2)-length(A),1)];
img(:,:,1) = img(:,:,1)+reshape(A,[size(img,1),size(img,2)]);
val = (cohere_weight*(1/size(t_patches.features,1))).*repmat(s_patch_weights(t_matchId),[1,patch_size(1)*patch_size(2)]).*s_patches.features(t_matchId,patch_size(1)*patch_size(2)+1:2*patch_size(1)*patch_size(2));
A = accumarray(ind,val(:));
A = [A;zeros(size(img,1)*size(img,2)-length(A),1)];
img(:,:,2) = img(:,:,2)+reshape(A,[size(img,1),size(img,2)]);
val = (cohere_weight*(1/size(t_patches.features,1))).*repmat(s_patch_weights(t_matchId),[1,patch_size(1)*patch_size(2)]).*s_patches.features(t_matchId,2*patch_size(1)*patch_size(2)+1:end);
A = accumarray(ind,val(:));
A = [A;zeros(size(img,1)*size(img,2)-length(A),1)];
img(:,:,3) = img(:,:,3)+reshape(A,[size(img,1),size(img,2)]);
val = (cohere_weight*(1/size(t_patches.features,1))).*repmat(s_patch_weights(t_matchId),[1,patch_size(1)*patch_size(2)]);
A = accumarray(ind,val(:));
A = [A;zeros(size(img,1)*size(img,2)-length(A),1)];
weight_img = weight_img+reshape(A,[size(img,1),size(img,2)]);

weight_img(weight_img==0) = 0.1;
img(:,:,1) = img(:,:,1)./weight_img;
img(:,:,2) = img(:,:,2)./weight_img;
img(:,:,3) = img(:,:,3)./weight_img;




