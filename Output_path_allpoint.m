function [output_movepath] = Output_path_allpoint(input)

    load('all_point.mat','distance_mat');
    load('all_point.mat','allpoint_graph');
    %deposits_name为宝藏序列
    %true_deposits为真宝藏
    % deposits_list=uint8(randperm(18,4)+1);
    % deposits_list(5:8)=deposits_list(1:4)+19;
    % blue_deposits=randperm(8,3);
    deposits_list=zeros(1,8,'uint8');
    for temp=1:1:8
        deposits_list(temp)=input(temp,1)+(input(temp,2)-1)*10;
    end
    %highlight(myplot,deposits_list(blue_deposits), 'NodeColor', 'magenta');
    %deposits_name为宝藏序列
    %true_deposits为真宝藏
    min_path=zeros(1,10,'uint8');
    distance_min=uint8(255);
    distance_temp=uint8(0);
    %列出所有路径A(8,8)个，使用全排序可优化
    %ALL_path=zeros(40320,8,'uint8');
    deposits_list=sort(deposits_list);
%     ALL_path=perms(deposits_list);
%     for qq=1:1:length(ALL_path)
%         node_list=[1 ALL_path(qq,:) 100];%1和100为起点和终点
%         distance_temp=0;
%         for ww=1:1:length(node_list)-1%计算最短路径
%             distance_temp=distance_temp+distance_mat(node_list(ww),node_list(ww+1));
%         end
%         if distance_temp<distance_min
%             distance_min=distance_temp;
%             min_path=node_list;
%         end
%     end
    node_list=[1 deposits_list 100];
    for ww=1:1:length(node_list)-1%计算最短路径
        distance_temp=distance_temp+distance_mat(node_list(ww),node_list(ww+1));
    end
    if distance_temp<distance_min
        distance_min=distance_temp;
        min_path=node_list;
    end
    while 1

        i=int8(8-1);
        temp=int8(0);
        while i>=1&&(deposits_list(i)>=deposits_list(i+1))
            i=i-1;
        end
        if i<1
            break
        end
        j=int8(8-0);
        while j > i && deposits_list(j) <= deposits_list(i)
            j=j-1;
        end
        temp=deposits_list(i);
        deposits_list(i)=deposits_list(j);
        deposits_list(j)=temp;
        k = int8(i + 1);
        l = int8(8 - 0);
        while k < l
            temp=deposits_list(k);
            deposits_list(k)=deposits_list(l);
            deposits_list(l)=temp;
            k=k+1;
            l=l-1;
        end
        
        
        node_list=[1 deposits_list 100];
        distance_temp=uint8(0);
        for ww=1:1:length(node_list)-1%计算最短路径
            distance_temp=distance_temp+distance_mat(node_list(ww),node_list(ww+1));
        end
        if distance_temp<distance_min
            distance_min=distance_temp;
            min_path=node_list;
        end
    end

    %获得总路程
    move_path=zeros(1,200,'uint8');
    for qq=1:1:9
        P=shortestpath(allpoint_graph,min_path(qq),min_path(qq+1));
        if qq==1
            move_path=[uint8(P)];
        else
            P(1)=[];
            move_path=[move_path uint8(P)];
        end
    end
    %node_now为小车当前处在的位置，direction_now为方向，右下左上对应1234
    %node_now=1;
    direction_now=int8(1);
    direction_temp=int8(1);
    output_movepath=zeros(1,100,'uint8');
    i=1;
    for qq=1:1:length(move_path)-1
        %node_now=qq;
        direction_temp=int8(move_path(qq+1))-int8(move_path(qq));%获得接下来的方向向量
        if direction_temp==1%向右
            direction_temp=int8(1);
        elseif direction_temp==-10%向下
            direction_temp=int8(2);
        elseif direction_temp==10%向上
            direction_temp=int8(4);
        elseif direction_temp==-1%向左
            direction_temp=int8(3);
        end
        %右转1掉头2左转3直行4
        if direction_temp~=direction_now%如果方向改变
            direction_subtrac=direction_temp-direction_now;
            if direction_subtrac==1%右转
                output_movepath(i)=1;
            elseif direction_subtrac==-1%左转
                output_movepath(i)=3;
            elseif abs(direction_subtrac)==2%掉头
                output_movepath(i)=2;
            elseif direction_subtrac==3%左转
                output_movepath(i)=3;
            elseif direction_subtrac==-3%右转
                output_movepath(i)=1;
            end
            i=i+1;
        else
            if degree(allpoint_graph,move_path(qq))>=3
                output_movepath(i)=4;
                i=i+1;
            end
        end
        direction_now=direction_temp;
    end
end