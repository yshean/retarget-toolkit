function [x,r,g,info] = spg_mmv( A, B, sigma, options )
%SPG_MMV  Solve multi-measurement basis pursuit denoise (BPDN)
%
%   SPG_MMV is designed to solve the basis pursuit denoise problem
%
%   (BPDN)  minimize  ||X||_1,2  subject to  ||A X - B||_2,2 <= SIGMA,
%
%   where A is an M-by-N matrix, B is an M-by-G matrix, and SIGMA is a
%   nonnegative scalar.  In all cases below, A can be an explicit M-by-N
%   matrix or matrix-like object for which the operations  A*x  and  A'*y
%   are defined (i.e., matrix-vector multiplication with A and its
%   adjoint.)
%
%   Also, A can be a function handle that points to a function with the
%   signature
%
%   v = A(w,mode)   which returns  v = A *w  if mode == 1;
%                                  v = A'*w  if mode == 2. 
%   
%   X = SPG_MMV(A,B,SIGMA) solves the BPDN problem.  If SIGMA=0 or
%   SIGMA=[], then the basis pursuit (BP) problem is solved; i.e., the
%   constraints in the BPDN problem are taken as AX=B.
%
%   X = SPG_MMV(A,B,SIGMA,OPTIONS) specifies options that are set using
%   SPGSETPARMS.
%
%   [X,R,G,INFO] = SPG_BPDN(A,B,SIGMA,OPTIONS) additionally returns the
%   residual R = B - A*X, the objective gradient G = A'*R, and an INFO
%   structure.  (See SPGL1 for a description of this last output argument.)
%
%   See also spgl1, spgSetParms, spg_bp, spg_lasso.

%   Copyright 2008, Ewout van den Berg and Michael P. Friedlander
%   http://www.cs.ubc.ca/labs/scl/spgl1
%   $Id$

if ~exist('options','var'), options = []; end
if ~exist('sigma','var'), sigma = 0; end
if ~exist('B','var') || isempty(B)
    error('Second argument cannot be empty.');
end
if ~exist('A','var') || isempty(A)
    error('First argument cannot be empty.');
end

groups = size(B,2);

if isa(A,'function_handle')
   A = @(x,mode) blockDiagonalImplicit(A,size(B,1),groups,x,mode);
else
   A = @(x,mode) blockDiagonalExplicit(A,size(B,1),groups,x,mode);
end

% Set projection specific functions
options.project     = @(x,weight,tau) NormL12_project(groups,x,weight,tau);
options.primal_norm = @(x,weight    ) NormL12_primal(groups,x,weight);
options.dual_norm   = @(x,weight    ) NormL12_dual(groups,x,weight);

tau = 0;
x0  = [];
[x,r,g,info] = spgl1(A,B(:),tau,sigma,x0,options);

n = round(length(x) / groups);
m = size(B,1);
x = reshape(x,n,groups);
y = reshape(r,m,groups);
g = reshape(g,n,groups);


function y = blockDiagonalImplicit(A,m,g,x,mode)

if mode == 1
   n = round(length(x) / g);
   y = zeros(m*g,1);
   for i=1:g
      y(1+(i-1)*m:i*m) = A(x(1+(i-1)*n:i*n),mode);
   end
else
   y = A(x(1+(i-1)*m:i*m),mode);
   n = length(y);
   y(n*g) = 0;
   for i=2:g
      y(1+(i-1)*n:i*n) = A(x(1+(i-1)*m:i*m),mode);
   end   
end


function y = blockDiagonalExplicit(A,m,g,x,mode)

n = size(A,2);

if mode == 1
   y = A * reshape(x,n,g);
   y = y(:);
else
   x = reshape(x,m,g);
   y = (x' * A)';
   y = y(:);
end
