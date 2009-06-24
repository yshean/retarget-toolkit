function img = bidirect_update_image(img,s_patches,s_patch_weights,s_matchId,t_patches,t_matchId)

eval('config_file');

weight_img = zeros(size(img,1),size(img,2));
for i = 1 : size(s_patches.features,1)
    patch_center = t_patches.centers(s_matchId(i),:);
    x_range = (patch_center(1)-(patch_size(1)-1)/2):(patch_center(1)+(patch_size(1)-1)/2);
    y_range = (patch_center(2)-(patch_size(2)-1)/2):(patch_center(2)+(patch_size(2)-1)/2);
    img(x_range,y_range,:) = img(x_range,y_range,:)+reshape((s_patch_weights(i)*complete_weight*(1/size(s_patches.features,1))).*s_patches.features(i,:),[patch_size(1),patch_size(2),3]);
    weight_img(x_range,y_range) = weight_img(x_range,y_range)+s_patch_weights(i)*complete_weight*(1/size(s_patches.features,1));
end

for i = 1 : size(t_patches.features,1)
    patch_center = t_patches.centers(i,:);
    x_range = (patch_center(1)-(patch_size(1)-1)/2):(patch_center(1)+(patch_size(1)-1)/2);
    y_range = (patch_center(2)-(patch_size(2)-1)/2):(patch_center(2)+(patch_size(2)-1)/2);
    img(x_range,y_range,:) = img(x_range,y_range,:)+reshape((s_patch_weights(t_matchId(i))*cohere_weight.*(1/size(t_patches.features,1))).*s_patches.features(t_matchId(i),:),[patch_size(1),patch_size(2),3]);
    weight_img(x_range,y_range) = weight_img(x_range,y_range)+s_patch_weights(t_matchId(i))*cohere_weight*(1/size(t_patches.features,1));
end

weight_img(weight_img==0) = 0.1;
img(:,:,1) = img(:,:,1)./weight_img;
img(:,:,2) = img(:,:,2)./weight_img;
img(:,:,3) = img(:,:,3)./weight_img;






