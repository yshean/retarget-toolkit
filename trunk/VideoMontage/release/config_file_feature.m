%%
root_path = '.';
img_path = [root_path,'/data'];
feature_path = [root_path,'/data'];

if(~exist(feature_path,'dir'))
    mkdir(feature_path);
end;

extractorID = 2;
extractor_path = '.\extractor\';


fprintf('run extract_features_32bit.exe \n');
curExtractor = 'extract_features_32bit.exe ';
%detector = '-harlap ';%
detector = '-p1 points_dense ';
descriptor = '-sift ';
out_type = ' -o1 ';
option = ' -noangle ';

extractor = [extractor_path, curExtractor];

%%
%this is for dense sample points
dense_sample = 1; 
is_generate_dense = 1;%0 for generate only the first-- 1 for generate all
dense_step = 1;
dense_scale = 16;

%%


