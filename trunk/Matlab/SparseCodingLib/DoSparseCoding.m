function [x, result] = DoSparseCoding(A, y, method, param)
% solving x for Ax = y with using l1-minization
% for 'fast_sc': path to fast sparse coding required
switch method
    case 'fast_sc'
        x = l1ls_featuresign(A, y, param);    
    case 'lasso'
        opts = spgSetParms('verbosity',1);
        x = spg_lasso(A, y, param, opts); 
    case 'bpdn'
         opts = spgSetParms('verbosity',1);
         x = spg_bpdn(A, y, param, opts);     
    case 'lasso2'
         x = SolveLasso(A, y, size(A,2), 'nnlasso', [], param);
    case 'bp'
         opts = spgSetParms('verbosity',1);
         x = spg_bp(A, y, opts);
end
result.x = x;
err = (A*x - y)' * (A*x - y);
result.mse = sqrt(err);
result.method = method;
result.param = param;
end