#include<stdio.h>
int getint(){
	int n;
	scanf("%d",&n);
	return n;
}
int max(int a, int b)
{
//    if(a == 100){
//
//    }
//    if(){
//
//    }


    if (a >b){
        return a;
    }else{
        return b;
    }
    return 0;
}

// Return the length of LCS for X[0...m-1] and Y[0...n-1]
int lcs(int X[], int Y[], int m, int n)
{
    if (m == 0 || n == 0)
        return 0;
    if (X[m-1] == Y[n-1])
        return lcs(X, Y, m-1, n-1) + 1;
    else
        return max(lcs(X, Y, m, n-1), lcs(X, Y, m-1, n));
    return 1;
}

int main()
{
    int X[12] = {1,2,3,2,4,1,2,4,5,6,7,10};
    int Y[13] = {2,4,3,1,2,4,1,6,7,10,12,100000,23};
    int x= 12;
    int y = 13;
//    string X = "ABCBDAB";
//    string Y = "BDCABA";
    printf("The length of LCS is  %d", lcs(X, Y, x, y));
//    cout << "The length of LCS is " << lcs(X, Y, 7, 6);
//    cout << endl;

//    getchar();
    return 0;
}