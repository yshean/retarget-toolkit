function bd_summary_3d(img_folder)
% Main entry of bidirectional summary algorithm
%
%
tic
eval('config_file');

img_files = dir([data_folder,img_folder,'/*',img_ext]);
img_names = cell(length(img_files),1);
origins = cell(length(img_files),1);
sources = cell(3,1);
for i = 1 : length(img_files)
    img_names{i} = img_files(i).name;
    img = imread([data_folder,img_folder,'/',img_files(i).name]);
    origins{i} = img;
    scaling_img = RGB2Lab(imresize(img,[size(img,1)*scaling_factor,size(img,2)*scaling_factor]));
    sources{1}(:,:,i) = scaling_img(:,:,1);
    sources{2}(:,:,i) = scaling_img(:,:,2);
    sources{3}(:,:,i) = scaling_img(:,:,3);
end

%% extract spatial-temporal patches
sources = bi_interp1(sources,length(img_files)*(1-scaling_factor));
for i = 1 : size(sources{1},3)
    sources_name = [result_folder,img_folder,'/',img_names{i}];
    sources_frame = zeros(size(sources{1},1),size(sources{1},2),3);
    sources_frame(:,:,1) = sources{1}(:,:,i);
    sources_frame(:,:,2) = sources{2}(:,:,i);
    sources_frame(:,:,3) = sources{3}(:,:,i);
    imwrite(uint8(Lab2RGB(sources_frame)),sources_name,'bmp');
end
source_patches = extract_3d_patches(sources,patch_size);

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

while (resize_gap(1)>resize_target(1) | resize_gap(2)>resize_target(2) | resize_gap(3)>resize_target(3))    
    fprintf('Processing %f of original video and resize it to [%f,%f,%f] of its size \n',scaling_factor,resize_gap(1),resize_gap(2),resize_gap(3));
    diff = 100; old_diff = 0;    
    while (abs(diff-old_diff)>converge_thresh)
        target_patches = extract_3d_patches(targets,patch_size);      
        [source_matches,target_matches] = nn_match(source_patches,target_patches);
                
        new_targets = cell(3,1);
        new_targets{1} = zeros(size(targets{1}));
        new_targets{2} = zeros(size(targets{2}));
        new_targets{3} = zeros(size(targets{3}));
        new_targets = bidirect_update_3d(new_targets,source_patches,ones(size(source_patches.features,1)),source_matches,target_patches,target_matches);
        
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
    
    %% save result;
%     for i = 1 : length(img_files)
%         retargeted_name = [result_folder,img_folder,'_retargeted/',img_names{i},'_retarget_',num2str(resize_gap),'.jpg'];
%         imwrite(Lab2RGB(targets{i}),retargeted_name,'jpg');
%     end
    
    if (resize_gap(1)>resize_target(1))
        resize_gap(1) = resize_gap(1)*resize_increase_factor;
    end
    if (resize_gap(2)>resize_target(2))
        resize_gap(2) = resize_gap(2)*resize_increase_factor;
    end
    if (resize_gap(3)>resize_target(3))
        resize_gap(3) = resize_gap(3)*resize_increase_factor;
    end
    
    %         scaling_factor = scaling_factor*2;
%     for i = 1 : length(img_files)
%         sources{i} = imresize(origins{i},[size(targets{i},1)/resize_gap,size(targets{i},2)/resize_gap]);
%         source_patches{i} = extract_patches(RGB2Lab(sources{i}),patch_size);
%     end
    
    if (resize_gap(1)>resize_target(1) | resize_gap(2)>resize_target(2) | resize_gap(3)>resize_target(3))
        targets = resize_3d(targets,resize_gap);
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
    fprintf('Upsampling the image to %f of its original size ... \n',scaling_factor);
    old_t_sizes = size(targets{1});
    targets = resize_3d(targets,upsample_factor.*ones(1,3));
    
    old_source_patches = source_patches;
    old_s_sizes = size(sources{1});
    sources = resize_3d(sources,upsample_factor.*ones(1,3));
    source_patches = extract_3d_patches(sources,patch_size);
    
    diff = 100; old_diff = 0;
    while (abs(diff-old_diff)>converge_thresh)
        old_target_patches = target_patches;        
        target_patches = extract_3d_patches(targets,patch_size);
        
        if (old_diff>0)
            old_source_matches = source_matches;
            old_target_matches = target_matches;
        end
 

        if (~exist('old_source_matches') || ~exist('old_target_matches'))
            [source_matches,target_matches] = nn_match(source_patches,target_patches);
        else
            [source_matches,target_matches] = nn_match(source_patches,target_patches,old_source_matches,old_target_matches);
        end
        
        new_targets = cell(3,1);
        new_targets{1} = zeros(size(targets{1}));
        new_targets{2} = zeros(size(targets{2}));
        new_targets{3} = zeros(size(targets{3}));
        new_targets = bidirect_update_3d(new_targets,source_patches,ones(size(source_patches.features,1)),source_matches,target_patches,target_matches);
        
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