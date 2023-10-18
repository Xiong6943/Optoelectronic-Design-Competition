function [] = Output_path(input)
global distance_mat;
global ALL_path;
%deposits_name为宝藏序列
%true_deposits为真宝藏
deposits_list=int8(zeros(8,1));
for temp=1:1:8
    deposits_list(temp)=input(temp,1)+(input(temp,2)-1)*10;
end
node_now=1;

min_path=zeros(1,10,'uint8');
distance_min=single(999999);
distance_temp=single(0);
%列出所有路径A(8,8)个，使用全排序可优化
%ALL_path=perms(deposits_list);
for qq=1:1:length(ALL_path)
    node_list=[1 ALL_path(qq,:) 20];
    distance_temp=single(0);
    for ww=1:1:length(node_list)-1
        distance_temp=distance_temp+distance_mat(node_list(ww),node_list(ww+1));
    end
    if distance_temp<distance_min
        distance_min=distance_temp;
        min_path=node_list;
    end
end
end