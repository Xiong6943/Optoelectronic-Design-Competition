function [flag,point_list] = next_perm(point_list,n)
%UNTITLED3 Summary of this function goes here
%   Detailed explanation goes here
    i=int8(n-2);
    temp=uint8(0);
    while i>=0&&point_list(i) >= point_list(i + 1)
        i=i-1;
    end
    if i<0
        flag=0;
        return;
    end
    j=uint8(n-1);
    while j > i && point_list(j) <= point_list(i)
        j=j-1;
    end
    temp=point_list(i);
    point_list(i)=point_list(j);
    point_list(j)=temp;
    k = uint8(i + 1);
    l = uint8(n - 1);
    while k < l
        temp=point_list(k);
        point_list(k)=point_list(l);
        point_list(l)=temp;
        k=k+1;
        l=l-1;
    end
    flag=1;
    return
end