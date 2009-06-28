function new_matches = interpolate_matches(matches,old_s_patches,new_s_patches,old_t_patches,new_t_patches,upsample_factor)

old2new_centers = round((old_t_patches.centers(matches,:)).*upsample_factor);

dist = distSqr_fast((old2new_centers)',(new_t_patches.centers)');
[match_dist,new_t_matches] = min(dist,[],2);

new2old_centers = round((new_s_patches.centers)./upsample_factor);
dist = distSqr_fast((new2old_centers)',(old_s_patches.centers)');
[match_dist,new_s_matches] = min(dist,[],2);

new_matches = new_t_matches(new_s_matches);
return;