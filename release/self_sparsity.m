function self_sparsity(img,feature,offset)

for y = 32 : 8 : size(img,1)-32
    for x = 32 : 8 : size(img,2)-32
        pix_idx = (x-1)*(size(img,1)-1)+y;
        nn_y = [max(y-offset,1):min(y+offset,size(img,1)-1)];
        nn_x = [max(x-offset,1):min(x+offset,size(img,2)-1)];
        
        [nn_x,nn_y] = meshgrid(nn_x,nn_y);
        
        nn_idx = sub2ind([size(img,1)-1,size(img,2)-1],nn_y(:)',nn_x(:)');
        
        codebook_idx = setdiff([1:size(feature,2)],nn_idx);
        A=feature(:,codebook_idx)./repmat(sqrt(sum(feature(:,codebook_idx).^2)),[size(feature,1),1]);
        coef=sparse_coding(A,feature(:,pix_idx)./norm(feature(:,pix_idx)),0.01,[]);
        clear A;
        sci = sum(abs(coef));
        
        figure(1);
        subplot(1,2,1),imshow(img);
        hold on;
        plot(x+1,y+1,'+r');
        hold off;
        subplot(1,2,2),imshow(img);
        hold on;
        nonzero_idx = find(coef>0);
        for (k = 1 : length(nonzero_idx))
            match_x = floor(codebook_idx(nonzero_idx(k))/(size(img,1)-1));
            match_y = codebook_idx(nonzero_idx(k))-match_x*(size(img,1)-1);
            plot(match_x+1,match_y+1,'+g');
        end
        hold off;
        title(num2str(length(nonzero_idx)));
        % pause();
    end
end