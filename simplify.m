row=21;
line=21;
line_time=1;
bend_time=1;
l=line_time;
b=bend_time;
%weight_L=[];
%weight_B;
weight=weight_L*line_time+weight_R*bend_time;
graph_A=graph(s,t,weight);
graph_A=simplify(graph_A,'min');
myplot=plot(graph_A,'XData',x,'YData',y,'EdgeLabel', graph_A.Edges.Weight,'linewidth',2);

[P,distance]=shortestpath(graph_A, 26, 10);
% 在图中高亮我们的最短路径
highlight(myplot, P, 'EdgeColor', 'r');
title(['shortest distance=',num2str(distance)]);