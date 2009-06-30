function new_frames = resize_3d(frames,resize_factor)

new_frames = cell(3,1);
for i = 1 : size(frames{1},3)
    source_frame = zeros(size(frames{1},1),size(frames{1},2),3);
    source_frame(:,:,1) = frames{1}(:,:,i);
    source_frame(:,:,2) = frames{2}(:,:,i);
    source_frame(:,:,3) = frames{3}(:,:,i);
    scaling_img = RGB2Lab(imresize(Lab2RGB(source_frame),[size(source_frame,1)*resize_factor(1),size(source_frame,2)*resize_factor(2)]));
    new_frames{1}(:,:,i) = scaling_img(:,:,1);
    new_frames{2}(:,:,i) = scaling_img(:,:,2);
    new_frames{3}(:,:,i) = scaling_img(:,:,3);
end
new_frames = bi_interp1(new_frames,size(frames{1},3)*(1-resize_factor(3)));