function extraction_feature

%% Load config file
eval('config_file_feature');

jpgFiles = dir([img_path, '/*.jpg']);

for idx = 1 : length(jpgFiles)

    [pathstr, name, ext, versn] = fileparts(jpgFiles(idx).name);
    fprintf('\nimage: %d %s\n', idx, name);

    img_type = ext;
    outfileName = [pathstr, filesep, name];

    if(~exist([feature_path, pathstr],'dir'))
        mkdir([feature_path, pathstr]);
    end;
    %change the image type if necessary
    if(~strcmp(img_type, '.pgm')&&~strcmp(img_type, '.png'))
        I = imread([img_path,'/',jpgFiles(idx).name]);
        imwrite(I, 'tmp.pgm', 'pgm');
        image = 'tmp.pgm';
    else
        image = [img_path,'/',jpgFiles(idx).name];
    end;



    %% use the code from UK && Inria
    if(dense_sample)
        if(is_generate_dense)
            I = imread(image);
            [H, W, P] = size(I);
            denseSampling(W, H, dense_step, dense_scale,'points_dense');
        else
            if(idx == 1)
                I = imread(image);
                [H, W, P] = size(I);
                denseSampling(W, H, dense_step, dense_scale,'points_dense');
            end;
        end;
    end;

    dos([extractor, detector, descriptor, option, '-i ', image, out_type, feature_path,outfileName,'.desc']);


end;


