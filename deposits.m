times=1;
average_min_distance=0;
for i=1:1:times
deposits_list=uint8(randperm(18,4)+1);
deposits_list(5:8)=deposits_list(1:4)+19;
blue_deposits=randperm(8,3);
myplot=plot(graph_A,'XData',x,'YData',y,'EdgeLabel', graph_A.Edges.Weight,'linewidth',2);
highlight(myplot,deposits_list, 'NodeColor', 'green');
highlight(myplot,deposits_list(blue_deposits), 'NodeColor', 'magenta');
title("淡紫色为蓝色宝藏的位置","/ /");
%deposits_name为宝藏序列
%true_deposits为真宝藏
node_now=1;
min_path=zeros(1,10,'uint8');
distance_min=single(999999);
distance_temp=single(0);
%列出所有路径A(8,8)个，使用全排序可优化
ALL_path=perms(deposits_list);
for qq=1:1:length(ALL_path)
    node_list=[1 ALL_path(qq,:) 20];
    distance_temp=0;
    for ww=1:1:length(node_list)-1
        distance_temp=distance_temp+distance_mat(node_list(ww),node_list(ww+1));
    end
    if distance_temp<distance_min
        distance_min=distance_temp;
        min_path=node_list;
    end
end
%exportgraphics(gca,"anime.gif","Append",true)
title("淡紫色为蓝色宝藏的位置","此次最短路径距离为"+num2str(distance_min));
%anime
for qq=1:1:9
    P=shortestpath(graph_A,min_path(qq),min_path(qq+1));
    %pause(0.1);
    highlight(myplot,P,'EdgeColor',[0.8500*(qq/9) 0.3250 0.0980],'LineWidth',8);
    %exportgraphics(gca,"anime.gif","Append",true);
end
%pause(0.5);
average_min_distance=distance_min+average_min_distance;
end
%title("淡紫色为蓝色宝藏的位置","此算法的平均最短路径为"+num2str(average_min_distance/times));