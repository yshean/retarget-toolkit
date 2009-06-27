function [min_vals,min_ids] = distSqr_knn(x,y,nn,yid)
% function z = distSqr(x,y)
%
% Return matrix of all-pairs squared distances between the vectors
% in the columns of x and y.
%
% INPUTS
% 	x 	dxn matrix of vectors
% 	y 	dxm matrix of vectors
%
% OUTPUTS
% 	z 	nxm matrix of squared distances
%
% This routine is faster when m<n than when m>n.
%
% David Martin <dmartin@eecs.berkeley.edu>
% March 2003

% Based on dist2.m code,
% Copyright (c) Christopher M Bishop, Ian T Nabney (1996, 1997)

[d,n] = size(x);
[d,m] = size(y);

x2 = sum(x.^2,1)';
y2 = sum(y.^2,1);

if size(x,1)~=size(y,1), 
  error('size(x,1)~=size(y,1)'); 
end

min_vals = zeros(n,1);
min_ids = zeros(n,1);
for i = 1:n,
  z = x(:,i)'*y(:,nn{yid(i)});
  [min_val,min_id] = min(x2(i) + y2(nn{yid(i)}) - 2*z);
  min_vals(i) = min_val;
  min_ids(i) = nn{yid(i)}(min_id);
end