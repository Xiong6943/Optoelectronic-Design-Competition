times=1;
average_min_distance=0;
for i=1:1:times
% deposits_list=uint8(randperm(18,4)+1);
% deposits_list(5:8)=deposits_list(1:4)+19;
% blue_deposits=randperm(8,3);
deposits_list=uint8(zeros(8,1));
for temp=1:1:8
    deposits_list(temp)=input(temp,1)+(input(temp,2)-1)*10;
end
myplot=plot(allpoint_graph,'XData',x,'YData',y,'linewidth',2);
highlight(myplot,deposits_list, 'NodeColor', 'green');
%highlight(myplot,deposits_list(blue_deposits), 'NodeColor', 'magenta');
title("绿色为宝藏的位置","/ /");
%deposits_name为宝藏序列
%true_deposits为真宝藏
min_path=zeros(1,10,'uint8');
distance_min=single(999999);
distance_temp=single(0);
%列出所有路径A(8,8)个，使用全排序可优化
ALL_path=perms(deposits_list);
for qq=1:1:length(ALL_path)
    node_list=[1 ALL_path(qq,:) 100];%1和100为起点和终点
    distance_temp=0;
    for ww=1:1:length(node_list)-1%计算最短路径
        distance_temp=distance_temp+distance_mat(node_list(ww),node_list(ww+1));
    end
    if distance_temp<distance_min
        distance_min=distance_temp;
        min_path=node_list;
    end
end
%exportgraphics(gca,"anime.gif","Append",true)
title("绿色为宝藏的位置","此次最短路径距离为"+num2str(distance_min));
%anime
clear move_path;
for qq=1:1:9
    P=shortestpath(allpoint_graph,min_path(qq),min_path(qq+1));
    highlight(myplot,P,'EdgeColor',[0.8500*(qq/9) 0.3250 0.0980],'LineWidth',8);
    if qq==1
        move_path=[uint8(P)];
    else
        P(1)=[];
        move_path=[move_path uint8(P)];
    end
    pause(0.3);
    %exportgraphics(gca,"anime_allpoint.gif","Append",true);
end


%node_now为小车当前处在的位置，direction_now为方向，右下左上对应1234
%node_now=1;
direction_now=int8(1);
direction_temp=int8(1);
output_movepath="";
for qq=1:1:length(move_path)-1
    %node_now=qq;
    direction_temp=int8(move_path(qq+1))-int8(move_path(qq));%获得接下来的方向向量
    if direction_temp==1%向右
        direction_temp=1;
    elseif direction_temp==-10%向下
        direction_temp=2;
    elseif direction_temp==10%向上
        direction_temp=4;
    elseif direction_temp==-1%向左
        direction_temp=3;
    end
    if direction_temp~=direction_now%如果方向改变
        direction_subtrac=direction_temp-direction_now;
        if direction_subtrac==1%右转
            output_movepath=[output_movepath," 右转"];
        elseif direction_subtrac==-1%左转
            output_movepath=[output_movepath," 左转"];
        elseif abs(direction_subtrac)==2%掉头
            output_movepath=[output_movepath," 掉头"];
        elseif direction_subtrac==3%左转
            output_movepath=[output_movepath," 左转"];
        elseif direction_subtrac==-3%右转
            output_movepath=[output_movepath," 右转"];
        end
    else
        if degree(allpoint_graph,move_path(qq))>=3
            output_movepath=[output_movepath," 直行"];
        end
    end

    direction_now=direction_temp;
end
%pause(0.5);
average_min_distance=distance_min+average_min_distance;
end
%title("淡紫色为蓝色宝藏的位置","此算法的平均最短路径为"+num2str(average_min_distance/times));