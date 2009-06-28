function bd_summary(img_folder)
% Main entry of bidirectional summary algorithm
%
%
tic
eval('config_file');

img_files = dir([data_folder,img_folder,'/*',img_ext]);
img_names = cell(length(img_files),1);
origins = cell(length(img_files),1);
sources = cell(length(img_files),1);
source_patches = cell(length(img_files),1);
for i = 1 : length(img_files)
    img_names{i} = img_files(i).name;
    img = imread([data_folder,img_folder,'/',img_files(i).name]);
    origins{i} = img;
    sources{i} = imresize(img,[size(img,1)*scaling_factor,size(img,2)*scaling_factor]);
    fprintf('Starting patch extraction ... ');
    tic
    source_patches{i} = extract_patches(RGB2Lab(sources{i}),patch_size);
    t = toc;
    fprintf('%f seconds\n',t);
    clear img;
end

targets = cell(length(img_files),1);
target_patches = cell(length(img_files),1);
for i = 1 : length(img_files)
    img = imresize(sources{i},[size(sources{i},1)*resize_gap,size(sources{i},2)*resize_gap]);    
    targets{i} = RGB2Lab(img);
    target_patches{i} = extract_patches(targets{i},patch_size);
end

figure(1);
for i = 1 : length(img_files)
    subplot(1,length(img_files),i), imshow(Lab2RGB(targets{i}));
end

while (resize_gap>resize_target)    
    fprintf('Processing %f of original image and resize it to %f of size \n',scaling_factor,resize_gap);
    diff = 100; old_diff = 0;    
    while (abs(diff-old_diff)>converge_thresh)    
        target_patches = cell(length(img_files),1);
        for i = 1 : length(img_files)
            target_patches{i} = extract_patches(targets{i},patch_size);
        end
        
        sources_match_array = cell(length(img_files),1);
        targets_match_array = cell(length(img_files),1);
        for i = 1 : length(img_files)
            [s_matchId,t_matchId] = nn_match(source_patches{i},target_patches{i});
            sources_match_array{i} = [sources_match_array{i};s_matchId];
            targets_match_array{i} = [targets_match_array{i};t_matchId];
        end
        
        source_patches_weights = cell(length(img_files),1);
        for i = 1 : length(img_files)
            source_patches_weights{i} = calc_edge_energy(sources{i},patch_size);%ones(size(source_patches{i}.features,1),1);
        end
                
        new_targets = cell(length(img_files),1);
        for i = 1 : length(img_files)
            new_targets{i} = zeros(size(targets{i}));
            target = zeros(size(targets{i}));
            new_targets{i} = bidirect_update_image(target,source_patches{i},source_patches_weights{i},sources_match_array{i},target_patches{i},targets_match_array{i});
        end
        
        old_diff = diff;
        diff = 0;
        for i = 1 : length(img_files)
            diff = diff+norm(targets{i}(:,:,1)-new_targets{i}(:,:,1),'fro');
            diff = diff+norm(targets{i}(:,:,2)-new_targets{i}(:,:,2),'fro');
            diff = diff+norm(targets{i}(:,:,3)-new_targets{i}(:,:,3),'fro');
        end
        fprintf('Difference is %f\n',diff);
        targets = new_targets;
        
        figure(1);
        for i = 1 : length(img_files)
            subplot(1,length(img_files),i), imshow(Lab2RGB(targets{i}));
        end
        
    end
    
    % save result;
    for i = 1 : length(img_files)
        retargeted_name = [result_folder,img_folder,'_retargeted/',img_names{i},'_retarget_',num2str(resize_gap),'.jpg'];
        imwrite(Lab2RGB(targets{i}),retargeted_name,'jpg');
    end
    
    resize_gap = resize_gap*resize_increase_factor;
    %         scaling_factor = scaling_factor*2;
%     for i = 1 : length(img_files)
%         sources{i} = imresize(origins{i},[size(targets{i},1)/resize_gap,size(targets{i},2)/resize_gap]);
%         source_patches{i} = extract_patches(RGB2Lab(sources{i}),patch_size);
%     end
    
    if (resize_gap>resize_target)
        for i = 1 : length(img_files)
            targets{i} = RGB2Lab(imresize(Lab2RGB(targets{i}),[size(sources{i},1)*resize_gap,size(sources{i},2)*resize_gap]));       
        end
    end
end

resize_gap = resize_gap/resize_increase_factor;
while (scaling_factor*upsample_factor<1)
    scaling_factor = scaling_factor*upsample_factor;
    fprintf('Upsampling the image to %f of its original size ... \n',scaling_factor);
    old_t_sizes = zeros(length(img_files),2);
    for i = 1 : length(img_files)
        old_t_sizes(i,:) = [size(targets{i},1),size(targets{i},2)];
        targets{i} = RGB2Lab(imresize(Lab2RGB(targets{i}),[size(targets{i},1)*upsample_factor,size(targets{i},2)*upsample_factor]));
    end
    
    old_source_patches = source_patches;
    old_s_sizes = zeros(length(img_files),2);
    for i = 1 : length(img_files)
        old_s_sizes(i,:) = [size(sources{i},1),size(sources{i},2)];
        sources{i} = imresize(origins{i},[size(origins{i},1)*scaling_factor,size(origins{i},2)*scaling_factor]);
        source_patches{i} = extract_patches(RGB2Lab(sources{i}),patch_size);
    end 
    
    diff = 100; old_diff = 0;
    while (abs(diff-old_diff)>converge_thresh)
        old_target_patches = target_patches;        
        target_patches = cell(length(img_files),1);
        for i = 1 : length(img_files)
            target_patches{i} = extract_patches(targets{i},patch_size);
        end
        
        for i = 1 : length(img_files)
            if (old_diff==0)
%                 old_sources_match_array{i} = interpolate_matches(sources_match_array{i},old_source_patches{i},source_patches{i},old_target_patches{i},target_patches{i},upsample_factor);
%                 old_targets_match_array{i} = interpolate_matches(targets_match_array{i},old_target_patches{i},target_patches{i},old_source_patches{i},source_patches{i},upsample_factor);
                
                % interpolate the match id
%                 new_s_size = [size(sources{i},1),size(sources{i},2)];
%                 new_t_size = [size(targets{i},1),size(targets{i},2)];
%                 
%                 old_sources_match_array{i} = interpolate_matches2(sources_match_array{i},source_patches{i},old_target_patches{i},...
%                     old_s_sizes(i,:),new_t_size,upsample_factor);
%                 old_targets_match_array{i} = interpolate_matches2(targets_match_array{i},target_patches{i},old_source_patches{i},...
%                     old_t_sizes(i,:),new_s_size,upsample_factor);
            else
                old_sources_match_array{i} = sources_match_array{i};
                old_targets_match_array{i} = targets_match_array{i};
            end
        end

        sources_match_array = cell(length(img_files),1);
        targets_match_array = cell(length(img_files),1);
        for i = 1 : length(img_files)
            if (~exist('old_sources_match_array') || ~exist('old_targets_match_array'))
                [s_matchId,t_matchId] = nn_match(source_patches{i},target_patches{i});
            else
                [s_matchId,t_matchId] = nn_match(source_patches{i},target_patches{i},old_sources_match_array{i},old_targets_match_array{i});
            end
            sources_match_array{i} = [sources_match_array{i};s_matchId];
            targets_match_array{i} = [targets_match_array{i};t_matchId];
        end
        
        source_patches_weights = cell(length(img_files),1);
        for i = 1 : length(img_files)
            source_patches_weights{i} = calc_edge_energy(sources{i},patch_size);%ones(size(source_patches{i}.features,1),1);
        end        

        new_targets = cell(length(img_files),1);
        for i = 1 : length(img_files)
            new_targets{i} = zeros(size(targets{i}));
            target = zeros(size(targets{i}));
            new_targets{i} = bidirect_update_image(target,source_patches{i},source_patches_weights{i},sources_match_array{i},target_patches{i},targets_match_array{i});
        end
        
        old_diff = diff;
        diff = 0;
        for i = 1 : length(img_files)
            diff = diff+norm(targets{i}(:,:,1)-new_targets{i}(:,:,1),'fro');
            diff = diff+norm(targets{i}(:,:,2)-new_targets{i}(:,:,2),'fro');
            diff = diff+norm(targets{i}(:,:,3)-new_targets{i}(:,:,3),'fro');
        end
        fprintf('Difference is %f\n',diff);
        targets = new_targets;
        
        figure(1);
        for i = 1 : length(img_files)
            subplot(1,length(img_files),i), imshow(Lab2RGB(targets{i}));
        end
    end % end while
    
    clear old_sources_match_array old_targets_match_array
end

% save result;
for i = 1 : length(img_files)
    retargeted_name = [result_folder,img_folder,'_retargeted/',img_names{i},'_retarget_',num2str(resize_gap),'.jpg'];
    imwrite(Lab2RGB(targets{i}),retargeted_name,'jpg');
end
consumed_t=toc;
fprintf('%f seconds \n',consumed_t);