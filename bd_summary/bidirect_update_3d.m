function frames = bidirect_update_3d(frames,patch_size,complete_weight,cohere_weight,s_patch_weights,s_patches,s_matchId,t_patches,t_matchId)

weight_frames = zeros(size(frames{1}));
for i = 1 : size(s_patches.features,1)
    patch_center = t_patches.centers(s_matchId(i),:);
    x_range = (patch_center(1)-(patch_size(1)-1)/2):(patch_center(1)+(patch_size(1)-1)/2);
    y_range = (patch_center(2)-(patch_size(2)-1)/2):(patch_center(2)+(patch_size(2)-1)/2);
    t_range = (patch_center(3)-(patch_size(3)-1)/2):(patch_center(3)+(patch_size(3)-1)/2);
    frames{1}(x_range,y_range,t_range) = frames{1}(x_range,y_range,t_range)+reshape((s_patch_weights(i)*complete_weight*(1/size(s_patches.features,1))).*...
                                                s_patches.features(i,1:patch_size(1)*patch_size(2)*patch_size(3)),[patch_size(1),patch_size(2),patch_size(3)]);
    frames{2}(x_range,y_range,t_range) = frames{2}(x_range,y_range,t_range)+reshape((s_patch_weights(i)*complete_weight*(1/size(s_patches.features,1))).*...
                                                s_patches.features(i,patch_size(1)*patch_size(2)*patch_size(3)+1:2*patch_size(1)*patch_size(2)*patch_size(3)),[patch_size(1),patch_size(2),patch_size(3)]);
    frames{3}(x_range,y_range,t_range) = frames{3}(x_range,y_range,t_range)+reshape((s_patch_weights(i)*complete_weight*(1/size(s_patches.features,1))).*...
                                                s_patches.features(i,2*patch_size(1)*patch_size(2)*patch_size(3)+1:end),[patch_size(1),patch_size(2),patch_size(3)]);
                                            
    weight_frames(x_range,y_range,t_range) = weight_frames(x_range,y_range,t_range)+s_patch_weights(i)*complete_weight*(1/size(s_patches.features,1));
end

for i = 1 : size(t_patches.features,1)
    patch_center = t_patches.centers(i,:);
    x_range = (patch_center(1)-(patch_size(1)-1)/2):(patch_center(1)+(patch_size(1)-1)/2);
    y_range = (patch_center(2)-(patch_size(2)-1)/2):(patch_center(2)+(patch_size(2)-1)/2);
    t_range = (patch_center(3)-(patch_size(3)-1)/2):(patch_center(3)+(patch_size(3)-1)/2);
    
    frames{1}(x_range,y_range,t_range) = frames{1}(x_range,y_range,t_range)+reshape((s_patch_weights(t_matchId(i))*cohere_weight*(1/size(t_patches.features,1))).*...
                                            s_patches.features(t_matchId(i),1:patch_size(1)*patch_size(2)*patch_size(3)),[patch_size(1),patch_size(2),patch_size(3)]);
    frames{2}(x_range,y_range,t_range) = frames{2}(x_range,y_range,t_range)+reshape((s_patch_weights(t_matchId(i))*cohere_weight*(1/size(t_patches.features,1))).*...
                                            s_patches.features(t_matchId(i),patch_size(1)*patch_size(2)*patch_size(3)+1:2*patch_size(1)*patch_size(2)*patch_size(3)),[patch_size(1),patch_size(2),patch_size(3)]);
    frames{3}(x_range,y_range,t_range) = frames{3}(x_range,y_range,t_range)+reshape((s_patch_weights(t_matchId(i))*cohere_weight*(1/size(t_patches.features,1))).*...
                                            s_patches.features(t_matchId(i),2*patch_size(1)*patch_size(2)*patch_size(3)+1:end),[patch_size(1),patch_size(2),patch_size(3)]);
    weight_frames(x_range,y_range,t_range) = weight_frames(x_range,y_range,t_range)+s_patch_weights(t_matchId(i))*cohere_weight*(1/size(t_patches.features,1));
end

weight_frames(weight_frames==0) = 0.1;
frames{1} = frames{1}./weight_frames;
frames{2} = frames{2}./weight_frames;
frames{3} = frames{3}./weight_frames;






