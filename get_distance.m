distance_mat=uint8(zeros(100,100,'single'));
www=1;
for qq=1:1:100
    for ww=www:1:100
        [P,distance_0]=shortestpath(allpoint_graph, qq, ww);
        distance_mat(qq,ww)=distance_0;
        distance_mat(ww,qq)=distance_0;
    end
    www=www+1;
end