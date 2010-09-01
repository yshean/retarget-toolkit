% find all index of x which is larger than threshold
function index = NonzeroIndex(x, threshold)
    d = size(x);
    curr = 1;
    index = zeros(1,1);
    for i = 1:1:d(1)
        if(x(i) > threshold)
            index(curr) = i;
            curr = curr + 1;
        end
    end
end