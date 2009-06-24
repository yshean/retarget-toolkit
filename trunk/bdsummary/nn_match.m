% function [complete_matchId,complete_min_val,cohere_matchId,cohere_min_val]=NN_match(S_patches,T_patches)
function [complete_matchId,cohere_matchId]=nn_match(S_patches,T_patches)
% function [complete_matchId,cohere_matchId]=NN_match(S_patches,T_patches,pre_S_matchId,pre_T_matchId)

complete_matchId = []; complete_min_val = [];
for i = 1 : 10000 : size(S_patches.features,1)
    if (i+9999>size(S_patches.features,1))
        pidx = [i:size(S_patches.features,1)];
    else
        pidx = [i:i+9999];
    end
    % tic
    dist = distSqr_fast((S_patches.features(pidx,:))',(T_patches.features)');
    % t=toc;
    % fprintf('%f\n',t);
        
    [min_val,p_complete_matchId] = min(dist,[],2);
    complete_matchId = [complete_matchId;p_complete_matchId];
    complete_min_val = [complete_min_val;min_val];
end

cohere_matchId = []; cohere_min_val = [];
for i = 1 : 10000 : size(T_patches.features,1)
    if (i+9999>size(T_patches.features,1))
        pidx = [i:size(T_patches.features,1)];
    else
        pidx = [i:i+9999];
    end
    dist = distSqr_fast((T_patches.features(pidx,:))',(S_patches.features)');

    [min_val,p_conhere_matchId] = min(dist,[],2);
    cohere_matchId = [cohere_matchId;p_conhere_matchId];
    cohere_min_val = [cohere_min_val;min_val];
end

% dist = distSqr_fast((S_patches.features)',(T_patches.features)');
% [complete_min_val,complete_matchId] = min(dist,[],2);
% [cohere_min_val,cohere_matchId] = min(dist,[],1);
% cohere_matchId = cohere_matchId;
% cohere_min_val = cohere_min_val';
% clear dist;
