function bd_summary_3d(img_folder,config_file)
% Main entry of bidirectional summary algorithm
%
%
tic
eval(config_file);

img_files = dir([data_folder,img_folder,'/*',img_ext]);
img_names = cell(length(img_files),1);
origins = cell(length(img_files),1);
sources = cell(3,1);
for i = 1 : length(img_files)
    img_names{i} = img_files(i).name;
    img = imread([data_folder,img_folder,'/',img_files(i).name]);
    origins{i} = img;
    scaling_img = RGB2Lab(imresize(img,[size(img,1)*scaling_factor(1),size(img,2)*scaling_factor(2)]));
    sources{1}(:,:,i) = scaling_img(:,:,1);
    sources{2}(:,:,i) = scaling_img(:,:,2);
    sources{3}(:,:,i) = scaling_img(:,:,3);
end

%% extract spatial-temporal patches
sources = bi_interp1(sources,length(img_files)*(1-scaling_factor(3)));
for i = 1 : size(sources{1},3)
    sources_name = [result_folder,img_folder,'/',img_names{i}];
    sources_frame = zeros(size(sources{1},1),size(sources{1},2),3);
    sources_frame(:,:,1) = sources{1}(:,:,i);
    sources_frame(:,:,2) = sources{2}(:,:,i);
    sources_frame(:,:,3) = sources{3}(:,:,i);
    imwrite(uint8(Lab2RGB(sources_frame)),sources_name,'bmp');
end
source_patches = extract_3d_patches(sources,patch_size,R);

if (resize_gap(1)>resize_target(1))
    resize_gap(1) = resize_gap(1)*resize_increase_factor;
end
if (resize_gap(2)>resize_target(2))
    resize_gap(2) = resize_gap(2)*resize_increase_factor;
end
if (resize_gap(3)>resize_target(3))
    resize_gap(3) = resize_gap(3)*resize_increase_factor;
end

targets = resize_3d(sources,resize_gap);
for i = 1 : size(targets{1},3)
    target_name = [result_folder,img_folder,'_retargeted/',img_names{i}];
    target_frame = zeros(size(targets{1},1),size(targets{1},2),3);
    target_frame(:,:,1) = targets{1}(:,:,i);
    target_frame(:,:,2) = targets{2}(:,:,i);
    target_frame(:,:,3) = targets{3}(:,:,i);
    imwrite(uint8(Lab2RGB(target_frame)),target_name,'bmp');
end

while (resize_gap(1)>resize_target(1) || resize_gap(2)>resize_target(2) || resize_gap(3)>resize_target(3))    
    fprintf('Processing [%f,%f,%f] of original video and resize it to [%f,%f,%f] of its size \n',...
            scaling_factor(1),scaling_factor(2),scaling_factor(3),resize_gap(1),resize_gap(2),resize_gap(3));
    diff = 100; old_diff = 0;    
    while (diff>converge_thresh)
        target_patches = extract_3d_patches(targets,patch_size,R);      
        [source_matches,target_matches] = nn_match(source_patches,target_patches);
                
        new_targets = cell(3,1);
        new_targets{1} = zeros(size(targets{1}));
        new_targets{2} = zeros(size(targets{2}));
        new_targets{3} = zeros(size(targets{3}));
        new_targets = bidirect_update_3d(new_targets,patch_size,complete_weight,cohere_weight,ones(size(source_patches.features,1),1),...
                                         source_patches,source_matches,target_patches,target_matches);
        
        old_diff = diff;
        diff = 0;
        for i = 1 : size(targets{1},3)
            diff = diff+norm(targets{1}(:,:,i)-new_targets{1}(:,:,i),'fro');
            diff = diff+norm(targets{2}(:,:,i)-new_targets{2}(:,:,i),'fro');
            diff = diff+norm(targets{3}(:,:,i)-new_targets{3}(:,:,i),'fro');
        end
        fprintf('Difference is %f\n',diff);
        targets = new_targets;
        
        for i = 1 : size(targets{1},3)
            target_name = [result_folder,img_folder,'_retargeted/',img_names{i}];
            target_frame = zeros(size(targets{1},1),size(targets{1},2),3);
            target_frame(:,:,1) = targets{1}(:,:,i);
            target_frame(:,:,2) = targets{2}(:,:,i);
            target_frame(:,:,3) = targets{3}(:,:,i);
            imwrite(uint8(Lab2RGB(target_frame)),target_name,'bmp');
        end       
    end
        
    if (resize_gap(1)>resize_target(1))
        resize_gap(1) = resize_gap(1)*resize_increase_factor;
    end
    if (resize_gap(2)>resize_target(2))
        resize_gap(2) = resize_gap(2)*resize_increase_factor;
    end
    if (resize_gap(3)>resize_target(3))
        resize_gap(3) = resize_gap(3)*resize_increase_factor;
    end
        
    if (resize_gap(1)>resize_target(1) || resize_gap(2)>resize_target(2) || resize_gap(3)>resize_target(3))
        targets = resize_3d(targets,[resize_gap(1),resize_gap(2),resize_increase_factor]);
    end
end

if (resize_gap(1)<=resize_target(1))
    resize_gap(1) = resize_gap(1)/resize_increase_factor;
end
if (resize_gap(2)<=resize_target(2))
    resize_gap(2) = resize_gap(2)/resize_increase_factor;
end
if (resize_gap(3)<=resize_target(3))
    resize_gap(3) = resize_gap(3)/resize_increase_factor;
end
while (scaling_factor*upsample_factor<1)
    scaling_factor = scaling_factor*upsample_factor;
    fprintf('Upsampling the image to [%f,%f,%f] of its original size ... \n',scaling_factor(1),scaling_factor(2),scaling_factor(3));
    old_t_sizes = size(targets{1});
    targets = resize_3d(targets,upsample_factor.*ones(1,3));
    
    old_source_patches = source_patches;
    old_s_sizes = size(sources{1});
    sources = resize_3d(sources,upsample_factor.*ones(1,3));
    source_patches = extract_3d_patches(sources,patch_size,R);
    
    diff = 100; old_diff = 0;
    while (diff>converge_thresh)
        tic;
        old_target_patches = target_patches;        
        target_patches = extract_3d_patches(targets,patch_size,R);        
        
        if (old_diff==0)
            new_s_size = [size(sources{1},1),size(sources{1},2),size(sources{1},3)];
            new_t_size = [size(targets{1},1),size(targets{1},2),size(targets{1},3)];
            
            old_source_matches = interpolate_matches2_3d(source_matches,patch_size,source_patches,old_target_patches,...
                                                         old_s_sizes,new_t_size,upsample_factor);
            old_target_matches = interpolate_matches2_3d(target_matches,patch_size,target_patches,old_source_patches,...
                                                         old_t_sizes,new_s_size,upsample_factor);
        else
            old_source_matches = source_matches;
            old_target_matches = target_matches;
        end
        fprintf('Time spent: %f secs.\n',toc);
 
        tic;
        if (~exist('old_source_matches') || ~exist('old_target_matches'))
            [source_matches,target_matches] = nn_match(source_patches,target_patches);
        else
            [source_matches,target_matches] = nn_match(source_patches,target_patches,old_source_matches,old_target_matches);
        end
        fprintf('Time spent: %f secs.\n',toc);
        
        tic;
        new_targets = cell(3,1);
        new_targets{1} = zeros(size(targets{1}));
        new_targets{2} = zeros(size(targets{2}));
        new_targets{3} = zeros(size(targets{3}));
        new_targets = bidirect_update_3d(new_targets,patch_size,complete_weight,cohere_weight,ones(size(source_patches.features,1),1),...
                                         source_patches,source_matches,target_patches,target_matches);
        fprintf('Time spent: %f secs.\n',toc);
        
        old_diff = diff;
        diff = 0;
        for i = 1 : size(targets{1},3)
            diff = diff+norm(targets{1}(:,:,i)-new_targets{1}(:,:,i),'fro');
            diff = diff+norm(targets{2}(:,:,i)-new_targets{2}(:,:,i),'fro');
            diff = diff+norm(targets{3}(:,:,i)-new_targets{3}(:,:,i),'fro');
        end
        fprintf('Difference is %f\n',diff);
        targets = new_targets;
        
        for i = 1 : size(targets{1},3)
            target_name = [result_folder,img_folder,'_retargeted/',img_names{i}];
            target_frame = zeros(size(targets{1},1),size(targets{1},2),3);
            target_frame(:,:,1) = targets{1}(:,:,i);
            target_frame(:,:,2) = targets{2}(:,:,i);
            target_frame(:,:,3) = targets{3}(:,:,i);
            imwrite(uint8(Lab2RGB(target_frame)),target_name,'bmp');
        end    
    end % end while
    
    clear old_source_matches old_target_matches
end

% save result;
for i = 1 : size(targets{1},3)
    target_name = [result_folder,img_folder,'_retargeted/',img_names{i}];
    target_frame = zeros(size(targets{1},1),size(targets{1},2),3);
    target_frame(:,:,1) = targets{1}(:,:,i);
    target_frame(:,:,2) = targets{2}(:,:,i);
    target_frame(:,:,3) = targets{3}(:,:,i);
    imwrite(uint8(Lab2RGB(target_frame)),target_name,'bmp');
end
consumed_t=toc;
fprintf('%f seconds \n',consumed_t);