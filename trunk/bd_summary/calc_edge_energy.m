function patch_edge_energies = calc_edge_energy(img,patch_size)
% Calcuate the edge energies for every patch

edge_map = edge(rgb2gray(img),'canny');

x_range = [(patch_size(1)-1)/2+1 : size(img,1)-(patch_size(1)-1)/2];
y_range = [(patch_size(2)-1)/2+1 : size(img,2)-(patch_size(2)-1)/2];

[x,y] = meshgrid(x_range,y_range);
x = x(:);
y = y(:);
patch_edge_energies = zeros(length(x),1);
for j = -(patch_size(1)-1)/2 : (patch_size(1)-1)/2
    for i = -(patch_size(1)-1)/2 : (patch_size(1)-1)/2    
        x_pixel = x(:)+i;
        y_pixel = y(:)+j;
        ind = sub2ind([size(img,1),size(img,2)],x_pixel,y_pixel);
        
        patch_edge_energies = patch_edge_energies+edge_map(ind);
    end
end

patch_edge_energies = patch_edge_energies+0.01;
return;
