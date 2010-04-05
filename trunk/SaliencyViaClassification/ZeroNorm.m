% calculate the zero norm of a vector
function norm = ZeroNorm(a)
   [w, h] = size(a);
   norm = 0;
   for i = 1:1:w
       if(a(i,1) > 0)
           norm = norm + 1;
       end
   end
end