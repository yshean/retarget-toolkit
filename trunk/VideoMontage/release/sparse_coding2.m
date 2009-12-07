function x = sparse_coding2(A,b,epsilon,inv_A)

%% Using L1-minimization to optimize sparse G
% 
% fprintf(1,'Computing the optimal sparse solution of Ax=b ... ');
% 
% H = A'*A;
% f = (-2).*(b'*A);
% n = size(A,2);
% 
% % cvx_begin
%     variable x(n)
%     minimize( sum(abs(x)) );
%     % constraints
%     quad_form(x,H)+f*x <= epsilon-b'*b;
% % cvx_end
% 
% obj = cvx_optval;
% fprintf(1,'Done! \n');

%% Using L1Magic to solve L1 minimization
% path(path, './l1magic/Optimization');
% 
% sigma = 0.005;
% x0 = inv_A*A'*b;
% % epsilon =  sigma*sqrt(size(A,1))*sqrt(1 + 2*sqrt(2)/sqrt(size(A,1)));
% 
% tic
% x = l1qc_logbarrier(x0, A, [], b, epsilon, 1e-3);
% toc

%% Using SPGL to solve L1 minimization 
% sigma = 0.1;                                     % Desired ||Ax - b||_2
opts = spgSetParms('verbosity',1);
x = spg_bpdn(A, b, epsilon, opts);
% x = spg_lasso(A, b, epsilon, opts);
return;