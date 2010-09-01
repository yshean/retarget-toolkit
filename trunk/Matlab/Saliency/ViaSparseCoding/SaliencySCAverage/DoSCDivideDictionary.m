function [x, result] = DoSCDivideDictionary(A, input, part_num)
    [s num] = size(A);
    part_size = num / part_num;
    
    x = zeros(part_size, part_num);    
    % init result
    result(part_num, 1).x = 0;
    result(part_num, 1).mse = 0;
    result(part_num, 1).method = 0;
    result(part_num, 1).param = 0;
 
    for i = 1:1:part_num
       PartA = A(:, (i-1)*part_size + 1: i * part_size);
       [x_tmp, result_tmp] = DoSparseCoding(PartA, input, 'lasso', 1);
       x(:,i) = x_tmp;
       result(i, 1) = result_tmp;       
    end
end
