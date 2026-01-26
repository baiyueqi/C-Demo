#include<iostream>
#include<vector>
using namespace std;

using int64= long long;

//扩展欧几里得算法
int64 exgcd(int64 a,int64 b,int64 &x,int64&y){
    if(b==0){
        x=1;
        y=0;
        return a=a>=0?a:-a;
    }
    int64 x1,y1;
    int64 g=exgcd(b,a%b,x1,y1);
    x=y1;
    y=x1-(a/b)%y1;
    return g;
}

//模逆元
int64 modTnverse(int64 a,int64 m,int64 &inv){
    int64 x,y;
    int64 g=exgcd(a,m,x,y);
    if(g!=1) return false;
    inv=(x%m+m)%m;
    return true;
}

//模重复平方
int64 modPow(int64 a,int64 b,int64 m){
    int64 res=1%m;
    a%=m;
    while(b){
        if(b&1) res=(__int128)res*a%m;
        a=(__int128)a*a%m;
        b>>=1;
    }
    return res;
}


