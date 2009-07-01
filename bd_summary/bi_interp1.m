function targets=bi_interp1(sources,dt)
dt=round(dt);
S=size(sources{1},3);
x1 = zeros(S,size(sources{1},1)*size(sources{1},2));
x2 = zeros(S,size(sources{1},1)*size(sources{1},2));
x3 = zeros(S,size(sources{1},1)*size(sources{1},2));
for i=1:S
    tmp1=sources{1}(:,:,i);
    x1(i,:)=tmp1(:);

    tmp2=sources{2}(:,:,i);
    x2(i,:)=tmp2(:);

    tmp3=sources{3}(:,:,i);
    x3(i,:)=tmp3(:);
end
nS=S/(S-dt):S/(S-dt):(S-dt)*S/(S-dt);
nS(nS<1)=1;
nS = round(nS);
ns=1:S;
y1 = zeros(size(x1,2),length(nS));
y2 = zeros(size(x1,2),length(nS));
y3 = zeros(size(x1,2),length(nS));
for j=1:size(x1,2)
    y1(j,:)=interp1(ns,x1(:,j),nS);
    y2(j,:)=interp1(ns,x2(:,j),nS);
    y3(j,:)=interp1(ns,x3(:,j),nS);
end

targets = cell(3,1);
for i=1:S-dt
    targets{1}(:,:,i)=reshape(y1(:,i),size(tmp1));
    targets{2}(:,:,i)=reshape(y2(:,i),size(tmp2));
    targets{3}(:,:,i)=reshape(y3(:,i),size(tmp3));
end