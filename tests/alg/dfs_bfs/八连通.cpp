#include <stdio.h>
int lake[105][105];
int N,M;
void input(){
    scanf("%d%d",&N,&M);
    for(int i=0;i<N;i++)
        for(int j=0;j<M;j++)
        {
            lake[i][j]=getchar();
            if(lake[i][j]=='\n'||lake[i][j]=='\r')
                lake[i][j]=getchar();
        }
}
/*
10
12
w........ww.
.www.....www
....ww...ww.
.........ww.
.........w..
..w......w..
.w.w.....ww.
ww.ww.....w.
.w.w.....ww.
..w......w..
*/
void dfs(int x,int y)
{
    if(x<0||x>=N||y<0||y>=M||lake[x][y]!='w')
        return;
    lake[x][y]='.';
    for(int i=-1;i<=1;i++)
        for(int j=-1;j<=1;j++)
            if(i==0&&j==0)
                continue;
            else
                dfs(x+i,y+j);
}  
int main(){
    input();
    int cnt=0;
    for(int i=0;i<N;i++)
        for(int j=0;j<M;j++)
        {
            if(lake[i][j]=='w')
            {
                dfs(i,j);
                cnt++;
            }
        }
    printf("%d",cnt);
    return 0;
}