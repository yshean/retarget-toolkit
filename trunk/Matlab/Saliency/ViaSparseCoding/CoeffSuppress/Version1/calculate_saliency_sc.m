% calcualte the saliency base on the sparse coding result
% param: extra param if necessary
% method:
% - mse: result.mse;
% - avg_coeff_abs_norm1: 
%   * param is the avg_coeff, 
%   * calculated by: coeff - avg_coeff, then taking norm 1 of only positive part
% - avg_coeff_mse: 
%   * param is the avg_coeff, 
%   * calculated by: mse between coeff & avg_coeff
function saliency = calculate_saliency_sc(result, method, param)
    switch method
        case 'mse'
            saliency = result.mse;
        case 'avg_coeff_abs_norm1'
            t = result.x - param;
            for j = 1:1:size(t,1)
                if t(j) < 0
                    t(j) = 0;
                end
            end
            saliency = t;
        case 'avg_coeff_mse'
            saliency = (result.x - param)' * (result.x - param);
    end
end