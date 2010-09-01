% result is the result of sparse coding, generaly should have following
% structure:
% result.x : coeff (result of Ax = y)
% result.mse: mean square error
% result.method: name of method used
% result.param: parameter of the method
% ------
% figure_num is the figure number use in method figure(figure_num) - for
% convenience
function DisplaySparseCodingResult(result, figure_num)
    num = size(result); % number of result we have
    % stack all result row by row
    figure(figure_num);    
    for i = 1:num(2)
        subplot(num(2), 1, i);
        plot(result(i).x);
        graph_title = strcat('Sparse Coding method:d ', result.method, ' dwith mse: ', num2str(result(i).mse));
        title(graph_title);
    end
end