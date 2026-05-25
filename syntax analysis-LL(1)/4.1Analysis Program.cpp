#include "tools.h"

void GramIn() {
    int i = 0;
    printf("请输入终结符号：\n");
    scanf("%c", Vt + i);
    while (*(Vt + i) != '\n') {
        i++;
        scanf("%c", Vt + i);
    }
    Vt[i] = '#';
    i++;
    VtNum = i;//输入结束存储终结符号个数

    i = 0;
    printf("请输入非终结符号：\n");
    scanf("%c", Vn + i);
    while (*(Vn + i) != '\n') {
        i++;
        scanf("%c", Vn + i);
    }
    VnNum = i;//输入结束存储非终结符号个数

    i = 0;
    printf("请输入文法产生式：\n");
    char ch;
    while (cin >> Generative[i]) {
        i++;
        if ((ch = getchar()) == '\n')
            break;
    }
    GenNum = i;//输入结束存储文法产生式个数
}

/************分解文法产生式*************/
void GramF() {
    // 拆解用“|”隔开的文法产生式
    int j, z = 0, x;
    for (int i = 0; i < GenNum; i++) {
        x = 0;
        while (1) {
            char ch1 = Generative[i][x];
            if (ch1 == '\0' || ch1 == '|')
                break;
            GenerativeNew[z] += Generative[i][x];
            x++;
        }
        z++;
        j = 0;
        while (Generative[i][j] != '\0') {
            if (Generative[i][j] == '|') {
                j++;
                for (x = 0; x < 3; x++) {
                    GenerativeNew[z] += Generative[i][x];
                }
                while (1) {
                    char ch2 = Generative[i][j];
                    if (ch2 == '\0' || ch2 == '|')
                        break;
                    GenerativeNew[z] += Generative[i][j];
                    j++;
                }
                z++;
            }
            else {
                j++;
            }
        }
    }
    GenNumNew = z;
}

void GetFirst(char a) {
    int k = 0;
    for (int i = 0; i < GenNumNew; i++) {
        if (GenerativeNew[i][0] != a)
            continue;
        if (Is(GenerativeNew[i][3], Vt, VtNum)) {
            //如果该非终结符产生式右部第一个字符是终结符号,则直接将其计入左部非终结符的FIRST集
            first[Back(GenerativeNew[i][0])] += GenerativeNew[i][3];
        }
        else if (Is(GenerativeNew[i][3], Vn, VnNum)) {
            //如果该非终结符号右部第一个字符是非终结符号,则对该右部第一个字符的FIRST进行求解,并将其加入左部字符的FIRST集
            GetFirst(GenerativeNew[i][3]);
            ADD(first[Back(GenerativeNew[i][0])], first[Back(GenerativeNew[i][3])]);
        }
        else if (GenerativeNew[i][3] == '@') {
            //如果该非终结符产生式是个空,则将空加入左部字符的FIRST集
            int j = 0;
            while (first[Back(GenerativeNew[i][0])][j] != '\0') {
                if (first[Back(GenerativeNew[i][0])][j] == '@') {
                    k = 1;
                    break;
                }
                j++;
            }
            if (!k)
                first[Back(GenerativeNew[i][0])] += '@';
        }
    }
}

void GetFollow(char a) {
    int i = Back(a), j;
    if (i == 0) {
        // 如果待求解字符是开始字符,则把'#'加入其FOLLOW集
        follow[Back(a)] += '#';
    }
    for (j = 0; j < GenNumNew; j++) {
        if (GenerativeNew[j][3] == a && GenerativeNew[j][4] != '\0') {//如果是A->Bb
            if (Is(GenerativeNew[j][4], Vt, VtNum)) {//如果b是终结符号，直接加入follow(B)
                if (IsChar(GenerativeNew[j][4], follow[Back(a)]))//判断b是否在follow(B)中
                    continue;
                else
                    follow[Back(a)] += GenerativeNew[j][4];
            }
            else if (Is(GenerativeNew[j][4], Vn, VnNum)) {//如果b是非终结符号，需要判断
                if (IsChar('@', first[Back(GenerativeNew[j][4])])) {//如果b可以推出空'@'，则需要将follow(A)加入follow(B)
                    GetFollow(GenerativeNew[j][0]);
                    ADDfollow(follow[Back(a)], follow[Back(GenerativeNew[j][0])]);
                }
                ADD(follow[Back(a)], first[Back(GenerativeNew[j][4])]);
            }
        }
        else if (GenerativeNew[j][4] == a && GenerativeNew[j][5] != '\0') {//如果是A->aBb
            if (Is(GenerativeNew[j][5], Vt, VtNum)) {//如果b是终结符号，直接加入follow(B)
                if (IsChar(GenerativeNew[j][5], follow[Back(a)]))//判断b是否在follow(B)中
                    continue;
                else
                    follow[Back(a)] += GenerativeNew[j][5];
            }
            else if (Is(GenerativeNew[j][5], Vn, VnNum)) {//如果b是非终结符号，需进行判断
                if (IsChar('@', first[Back(GenerativeNew[j][5])])) {//如果b可以推出空'@'，则需要将follow(A)加入follow(B)
                    GetFollow(GenerativeNew[j][0]);
                    ADDfollow(follow[Back(a)], follow[Back(GenerativeNew[j][0])]);
                }
                ADD(follow[Back(a)], first[Back(GenerativeNew[j][5])]);
            }
        }
        else if (GenerativeNew[j][4] == a && GenerativeNew[j][5] == '\0') {//如果是A->aB
            GetFollow(GenerativeNew[j][0]);//直接将follow(A)加入follow(B)
            ADDfollow(follow[Back(a)], follow[Back(GenerativeNew[j][0])]);
        }
    }
}

// 算法4.2
/************预测分析表构建程序*************/
void FAtable() {
    int i, j;
    for (i = 0; i < VtNum; i++) {
        for (j = 0; j < GenNumNew; j++) {
            if (Vt[i] == GenerativeNew[j][3])
                //如果终结符Vt[i]在A->a的first(a)中,则将A->a放入table[A,Vt[i]]中
                table[Back(GenerativeNew[j][0])][i] = j;
            else if (Is(GenerativeNew[j][3], Vn, VnNum)) {
                if (IsChar(Vt[i], first[Back(GenerativeNew[j][3])])) {
                    table[Back(GenerativeNew[j][0])][i] = j;
                }
            }
            else if (GenerativeNew[j][3] == '@') {
                //如果当前的产生式是：A->a且，a='@'，则判断当前的Vt[i]是否在
                if (IsChar(Vt[i], follow[Back(GenerativeNew[j][0])])) {
                    table[Back(GenerativeNew[j][0])][i] = j;
                }
            }
        }
    }
}

/************文字输入程序*************/
void WordIn() {
    printf("请输入您需要测试的文字：\n");
    int i = 0;
    //getchar();
    scanf("%c", word + i);
    while (*(word + i) != '\n') {
        i++;
        scanf("%c", word + i);
    }
}

/************文法分析程序*************/
void GAnalysis() {
    int i = 0, x, y, k, error = 0, n = 1;
    char a;
    string chan = "";
    st.push('#');
    st.push(Vn[0]);
    a = st.top();
    while (!(a == word[i] && a == '#')) {
        if (Is(st.top(), Vn, VnNum)) {
            x = Back(st.top());
            y = BBack(word[i]);
            k = table[x][y];//获得产生式
            if (k == -1) {
                error++;
                cout << "步骤" << n << "：" << "\t" << "识别错误！跳过" << word[i] << "；\n";
                n++;
                i++;
                break;
            }
            else {
                chan = GenerativeNew[k];
                k = 0;
                st.pop();
                while (chan[k] != '\0') {
                    k++;
                }
                k--;
                if (chan[k] != '@') {
                    while (chan[k] != '>') {
                        st.push(chan[k]);
                        k--;
                    }
                    //i++;
                    cout << "步骤" << n << "：" << "\t" << "使用产生式" << chan << "入栈完成；\n";
                    n++;
                }
                else {
                    cout << "步骤" << n << "：" << "\t" << "使用产生式" << chan << "；\n";
                    n++;
                    //i++;
                }
            }
        }
        else if (Is(st.top(), Vt, VtNum)) {
            if (st.top() == word[i]) {
                cout << "步骤" << n << "：" << "\t" << "匹配栈顶和当前符号" << word[i] << "，匹配成功；\n";
                st.pop();
                i++;
                n++;
            }
            else {
                cout << "步骤" << n << "：" << "\t" << "识别失败！！\n";
                n++;
                break;
            }
        }
        a = st.top();
    }
    if (error) {
        cout << "步骤" << n << "：" << "\t" << "识别错误！！错误跳过次数：" << error << "\n";
        n++;
    }
    else {
        cout << "步骤" << n << "：" << "\t" << "识别成功！！\n";
        n++;
    }
}


int main() {
    //初始化预测表
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            table[i][j] = -1;
        }
    }
    GramIn();//输入非终结、终结字符和文法产生式
    GramF();//文法产生式分析完毕
    int i, j;
    for (i = 0; i < VnNum; i++) {//first集合
        GetFirst(Vn[i]);
    }
    cc(); // first集去重
    for (i = 0; i < VtNum; i++) {//follow集合
        GetFollow(Vn[i]);
    }
    clearF();//follow集去重
    FAtable();//预测分析表
    cout << "非终结符的first集为：\n";
    for (i = 0; i < VnNum; i++) {
        cout << "FIRST(" << Vn[i] << ")：" + first[i] << endl;
    }
    cout << "非终结符的follow集为：\n";
    for (i = 0; i < VnNum; i++) {
        cout << "FOLLOW(" << Vn[i] << ")：" + follow[i] << endl;
    }
    cout << "预测分析表为：\n";
    cout << "\t";
    cout.width(7);
    for (i = 0; i < VtNum; i++) {
        cout << Vt[i];
        cout.width(7);
    }
    cout << endl;

    for (i = 0; i < VnNum; i++) {
        cout << Vn[i];
        cout.width(7);
        for (j = 0; j < VtNum; j++) {
            cout << GenerativeNew[table[i][j]];
            cout.width(7);
        }
        cout << endl;
    }
    WordIn();
    cout << "识别结果如下：\n";
    GAnalysis();
    return 0;
}

// 输入格式：
// 终结符号：+ - * / ( ) n 用n表示num
// 非终结符号：E T F （默认第一个是开始符号）
// 产生式：E->E+T|E-T|T T->T*F|T/F|F F->(E)|n 

// 手动消除左递归后：用e表示E'，t表示T'，@表示空
// 终结符号：+ - * / ( ) n
// 非终结符号：E e T t F 
// 产生式：E->Te e->+Te|-Te|@ T->Ft t->*Ft|/Ft|@ F->(E)|n
// 语句样例：n+n*n-(n+n)/n