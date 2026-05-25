#ifndef HEAD_1  // 头文件保护，防止重复包含
#define HEAD_1

#include <iostream>
#include<stdio.h>
#include<cstdio>
#include<string.h>
#include<stdlib.h>
#include<stack>
#include <bits/stdc++.h>
using namespace std;

int table[100][100] = { 0 };//预测表
char Vt[100] = { "" };//终结符，额外加入“#”意为栈顶符号（“dollar符”）
char Vn[100] = { "" };//非终结符
string Generative[100] = { "" };//文法产生式存储
string GenerativeNew[100] = { "" };//文法产生式分解后的存储
// 文法产生式的输入(注意：文法产生式之间用空格隔开，不要换行)：A->BC C->+BC|@ B->DE E->DE|@ D->(A)|i ，空用@表示
// 文法产生式的存储：GenerativeNew中的每一个字符串都是一个产生式（A->BC）

string first[100] = { "" };//first集合
string follow[100] = { "" };//follow集合
// first中的下标与Vn中的一致，如果A在Vn中下标为1，则A的first下标为1，存的是它的first集组成的字符串
// 可以用Back（A）获得此下标

char word[100] = { "" };//待测试的文字
int VtNum = 0;//终结符号的个数
int VnNum = 0;//非终结符号的个数
int GenNum = 0;//文法产生式个数
int GenNumNew = 0;//文法产生式分解后的个数
stack<char> st;//预测分析栈


/************是否在集合中*************/
bool Is(char x, char a[], int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] == x)
            return true;
    }
    return false;
}

/************判断一个字符是否在一个集合中的程序*************/
bool IsChar(char a, string b) {
    int i = 0;
    while (b[i] != '\0') {
        if (a == b[i])
            return true;
        i++;
    }
    return false;
}

/************集合相加程序*************/
void ADD(string& a, string& b) {
    int i = 0, zk = 1, j = 0;
    while (b[j] != '\0') {
        i = 0;
        zk = 1;
        while (a[i] != '\0') {
            if (b[j] == a[i] || b[j] == '@') {
                zk = -1;
                break;
            }
            i++;
        }
        if (zk == 1)
            a += b[j];
        j++;
    }
}
/************集合(follow)相加程序*************/
void ADDfollow(string& a, string& b) {
    int i = 0, zk = 1, j = 0;
    while (b[j] != '\0') {
        i = 0;
        zk = 1;
        while (a[i] != '\0') {
            if (b[j] == a[i]) {
                zk = -1;
                break;
            }
            i++;
        }
        if (zk == 1)
            a += b[j];
        j++;
    }
}


/************根据字母返回终结符下标程序*************/
int BBack(char a) {
    for (int i = 0; i < VtNum; i++) {
        if (a == Vt[i])
            return i;
    }
    return -1;
}
/************根据字母返回非终结符下标程序*************/
int Back(char a) {
    for (int i = 0; i < VnNum; i++) {
        if (a == Vn[i])
            return i;
    }
    return -1;
}

/************ first集去重*************/
void cc() {
    int i, j, k, q, p;
    for (i = 0; i < VnNum; i++) {
        j = 0;
        while (first[i][j] != '\0') {
            k = j + 1;
            while (first[i][k] != '\0') {
                if (first[i][j] == first[i][k])
                    first[i][k] = ' ';
                k++;
            }
            j++;
        }
        q = 0;
        while (first[i][q] != '\0') {
            if (first[i][q] != ' ') {
                break;
            }
            else {
                p = q + 1;
                while (first[i][p] != ' ') {
                    if (first[i][p] == '\0')
                        break;
                    else {
                        first[i][q] = first[i][p];
                        first[i][p] = ' ';
                    }
                }
            }
            q++;
        }
        q = 0;
        while (first[i][q] != '\0') {
            if (first[i][q] == ' ')
                first[i][q] = '\0';
            q++;
        }
    }
}
/************follow集清除重复元素程序*************/
void clearF() {
    int i, j, k, q, p;
    //下面是清除follow集
    for (i = 0; i < VnNum; i++) {
        j = 0;
        while (follow[i][j] != '\0') {
            k = j + 1;
            while (follow[i][k] != '\0') {
                if (follow[i][j] == follow[i][k])
                    follow[i][k] = ' ';
                k++;
            }
            j++;
        }
        q = 0;
        p = 0;
        while (follow[i][q] != '\0') {
            if (follow[i][q] != ' ') {
                break;
            }
            else {
                p = q + 1;
                while (follow[i][p] != ' ') {
                    if (follow[i][p] == '\0')
                        break;
                    else {
                        follow[i][q] = follow[i][p];
                        follow[i][p] = ' ';
                    }
                }
            }
            q++;
        }
        q = 0;
        while (follow[i][q] != '\0') {
            if (follow[i][q] == ' ')
                follow[i][q] = '\0';
            q++;
        }
    }
}

/* --------------lab2 tools-------------- */ 


#endif 

