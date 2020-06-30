/*
    给定整数a1,a2...an,判断是否可以从中选出若干数，是他们的和刚好为k
    输入:
    n=4
    a={1 2 4 7}
    k=13
    输出:
    Yes, 13=2+4+7
*/
#include <stdio.h>
int a[20];
int k;
int n;
int dfs(int sum,int i)
{
    if(i==n)
    {
        if(sum==k)
            printf("\n");
        return sum==k; 
    }
    if(dfs(sum+a[i],i+1))
    {
        printf("+%d",a[i]);
        return true;
    }    
    else if(dfs(sum,i+1))
        return true;
    return false;
}
int main(){
    scanf("%d",&n);
    for(int i=0;i<n;i++)
        scanf("%d",a+i);
    scanf("%d",&k);
    if(dfs(0,0))
        printf("Yes");
    else
        printf("No");
    return 0;
}