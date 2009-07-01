function new_matches = interpolate_matches2_2d(matches,patch_size,new_s_patches,old_t_patches,old_s_size,new_t_size,upsample_factor)

x_range = [(patch_size(1)-1)/2+1 : new_t_size(1)-(patch_size(1)-1)/2];
y_range = [(patch_size(2)-1)/2+1 : new_t_size(2)-(patch_size(2)-1)/2];

old2new_centers = round((old_t_patches.centers).*upsample_factor);
old2new_centers(:,1) = max(old2new_centers(:,1),(patch_size(1)-1)/2+1);
old2new_centers(:,1) = min(old2new_centers(:,1),new_t_size(1)-(patch_size(1)-1)/2);
old2new_centers(:,2) = max(old2new_centers(:,2),(patch_size(2)-1)/2+1);
old2new_centers(:,2) = min(old2new_centers(:,2),new_t_size(2)-(patch_size(2)-1)/2);
new_t_matches = (old2new_centers(:,1)-x_range(1)).*length(y_range)+old2new_centers(:,2)-y_range(1)+1;
% new_t_matches = (old2new_centers(:,2)-y_range(1)).*length(x_range)+old2new_centers(:,1)-x_range(1)+1;

x_range = [(patch_size(1)-1)/2+1 : old_s_size(1)-(patch_size(1)-1)/2];
y_range = [(patch_size(2)-1)/2+1 : old_s_size(2)-(patch_size(2)-1)/2];

new2old_centers = round((new_s_patches.centers)./upsample_factor);
new2old_centers(:,1) = max(new2old_centers(:,1),(patch_size(1)-1)/2+1);
new2old_centers(:,1) = min(new2old_centers(:,1),old_s_size(1)-(patch_size(1)-1)/2);
new2old_centers(:,2) = max(new2old_centers(:,2),(patch_size(2)-1)/2+1);
new2old_centers(:,2) = min(new2old_centers(:,2),old_s_size(2)-(patch_size(2)-1)/2);
new_s_matches = (new2old_centers(:,1)-x_range(1)).*length(y_range)+new2old_centers(:,2)-y_range(1)+1;
% new_s_matches = (new2old_centers(:,2)-y_range(1)).*length(x_range)+new2old_centers(:,1)-x_range(1)+1;

new_matches = new_t_matches(matches(new_s_matches));
return;