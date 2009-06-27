% function [complete_matchId,cohere_matchId]=nn_match(S_patches,T_patches)
function [complete_matchId,cohere_matchId]=NN_match(S_patches,T_patches,pre_S_matchId,pre_T_matchId)

complete_matchId = []; complete_min_val = [];
for i = 1 : 10000 : size(S_patches.features,1)
    if (i+9999>size(S_patches.features,1))
        pidx = [i:size(S_patches.features,1)];
    else
        pidx = [i:i+9999];
    end
    
    if (~exist('pre_S_matchId'))
        dist = distSqr_fast((S_patches.features(pidx,:))',(T_patches.features)');
        [p_min_val,p_complete_matchId] = min(dist,[],2);
    else
        [p_min_val,p_complete_matchId] = distSqr_knn((S_patches.features(pidx,:))',(T_patches.features)',T_patches.nn,pre_S_matchId(pidx));
    end
        
    
    complete_matchId = [complete_matchId;p_complete_matchId];
    complete_min_val = [complete_min_val;p_min_val];
end

cohere_matchId = []; cohere_min_val = [];
for i = 1 : 10000 : size(T_patches.features,1)
    if (i+9999>size(T_patches.features,1))
        pidx = [i:size(T_patches.features,1)];
    else
        pidx = [i:i+9999];
    end
    if (~exist('pre_T_matchId'))
        dist = distSqr_fast((T_patches.features(pidx,:))',(S_patches.features)');
        [p_min_val,p_cohere_matchId] = min(dist,[],2);
    else
        nn_ids = S_patches.nn{pre_T_matchId(pidx)};
        [p_min_val,p_cohere_matchId] = distSqr_knn((T_patches.features(pidx,:))',(S_patches.features)',S_patches.nn,pre_T_matchId(pidx));
    end
    
    cohere_matchId = [cohere_matchId;p_cohere_matchId];
    cohere_min_val = [cohere_min_val;p_min_val];
end

% dist = distSqr_fast((S_patches.features)',(T_patches.features)');
% [complete_min_val,complete_matchId] = min(dist,[],2);
% [cohere_min_val,cohere_matchId] = min(dist,[],1);
% cohere_matchId = cohere_matchId;
% cohere_min_val = cohere_min_val';
% clear dist;
