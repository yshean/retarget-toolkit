function targets=bi_interp1(sources,dt)
dt=round(dt);
S=size(sources,2);
for i=1:S
    tmp1=sources{i}(:,:,1);
    x1(i,:)=tmp1(:);

    tmp2=sources{i}(:,:,2);
    x2(i,:)=tmp2(:);

    tmp3=sources{i}(:,:,3);
    x3(i,:)=tmp3(:);
end
nS=S/(S-dt):S/(S-dt):(S-dt)*S/(S-dt);
ns=1:size(sources,2);
for j=1:size(x1,2)
    y1(j,:)=interp1(ns,x1(:,j),nS);
    y2(j,:)=interp1(ns,x2(:,j),nS);
    y3(j,:)=interp1(ns,x3(:,j),nS);
end

for i=1:S-dt
    targets{i}(:,:,1)=reshape(y1(:,i),size(tmp1));
    targets{i}(:,:,2)=reshape(y2(:,i),size(tmp2));
    targets{i}(:,:,3)=reshape(y3(:,i),size(tmp3));
end