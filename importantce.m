graph_A=graph(s,t,weight);
graph_A=simplify(graph_A,'min');
myplot=plot(graph_A,'XData',x,'YData',y,'EdgeLabel', graph_A.Edges.Weight,'linewidth',2);
ucc = centrality(graph_A,'closeness');
myplot.NodeCData = ucc;
colormap jet
colorbar
wcc = centrality(graph_A,'closeness','Cost',graph_A.Edges.Weight);
myplot.NodeCData = wcc;

wbc = centrality(graph_A,'betweenness','Cost',graph_A.Edges.Weight);
n = numnodes(graph_A);
myplot.NodeCData = 2*wbc./((n-2)*(n-1));
colormap(flip(autumn,1));
title('importance');