// LR(1) 语法分析器（简洁 C++ 实现）
// 要求满足：
// (1) 构建 DFA 后，打印每个状态的项目集规范族
// (2) 构建 LR 分析表后，打印 ACTION / GOTO 表
// (3) 打印分析的每一步（状态栈、符号栈、剩余输入、动作）
//
// ===== 输入格式（全部以空格分隔，尽量简单）======
// 第一行: 产生式条数 N
// 接下来的 N 行: 形如  LHS -> RHS1 RHS2 ...  （右部用单个字符或字符串做符号，空串用 ε 或 epsilon）
//   例:  E -> E + T
//        E -> T
//        T -> T * F
//        T -> F
//        F -> ( E )
//        F -> id
// 第 N+2 行: 起始非终结符（未增广的，如 E）。程序自动增广为 S' -> E
// 第 N+3 行: 终结符数量 T
// 第 N+4 行: T 个终结符（不要包含 $，程序会自动添加 $）
// 第 N+5 行: 非终结符数量 V
// 第 N+6 行: V 个非终结符（需要包含起始符号）
// 第 N+7 行: 待分析串的长度 L
// 第 N+8 行: L 个记号（与前面定义的终结符一致），程序会自动在末尾加上 $
// ================================================
//
// 示例输入：
// 6
// E -> E + T
// E -> T
// T -> T * F
// T -> F
// F -> ( E )
// F -> id
// E
// 7
// id + * ( ) epsilon ε
// 3
// E T F
// 7
// id + id * id ) (
//
// 说明：上例中给了 epsilon / ε 两个写法，其实只需其一，不在语法中使用可忽略。
// 注意：句子中的右括号与左括号顺序特意打乱仅为演示报错，正常应为 "id + id * id ) (" -> 错误。

#include <bits/stdc++.h>
using namespace std;

struct Production { string lhs; vector<string> rhs; };
struct Item { int prod; int dot; string look; };

// for set/map of Item
static bool operator<(const Item& a, const Item& b){
    if(a.prod!=b.prod) return a.prod<b.prod;
    if(a.dot!=b.dot) return a.dot<b.dot;
    return a.look<b.look;
}
static bool operator==(const Item& a, const Item& b){
    return a.prod==b.prod && a.dot==b.dot && a.look==b.look;
}

// Globals for simplicity
vector<Production> PRODS; // 0 号为增广产生式 S' -> S
vector<string> TERMS, NONTERMS; // TERMS 包含 $, NONTERMS 包含 S 和 S'
set<string> ALLSYMS; // 终结符、非终结符和 $
map<string,set<string>> FIRST; // FIRST 集

// 工具函数
static bool isTerminal(const string& x){ return find(TERMS.begin(), TERMS.end(), x)!=TERMS.end(); }
static bool isNonterm(const string& x){ return find(NONTERMS.begin(), NONTERMS.end(), x)!=NONTERMS.end(); }

// FIRST 集计算
void compute_first(){
    FIRST.clear();
    for(const auto& t: TERMS) FIRST[t].insert(t);
    bool changed=true;
    while(changed){
        changed=false;
        for(const auto& p: PRODS){
            auto& Flhs = FIRST[p.lhs];
            size_t before = Flhs.size();
            bool nullable_prefix=true;
            if(p.rhs.empty()){
                Flhs.insert("ε");
            }else{
                for(const auto& s: p.rhs){
                    for(const auto& a: FIRST[s]) if(a!="ε") Flhs.insert(a);
                    if(!FIRST[s].count("ε")) { nullable_prefix=false; break; }
                }
                if(nullable_prefix) Flhs.insert("ε");
            }
            if(Flhs.size()>before) changed=true;
        }
    }
}

set<string> first_of_seq(const vector<string>& seq){
    set<string> res; bool all_nullable=true;
    if(seq.empty()){ res.insert("ε"); return res; }
    for(const auto& s: seq){
        for(const auto& a: FIRST[s]) if(a!="ε") res.insert(a);
        if(!FIRST[s].count("ε")) { all_nullable=false; break; }
    }
    if(all_nullable) res.insert("ε");
    return res;
}

// 闭包
set<Item> closure(const set<Item>& I){
    set<Item> C = I; bool changed=true;
    while(changed){
        changed=false;
        vector<Item> to_add;
        for(const auto& it: C){
            const auto& p = PRODS[it.prod];
            if(it.dot < (int)p.rhs.size()){
                string B = p.rhs[it.dot];
                if(isNonterm(B)){
                    vector<string> beta;
                    for(int k=it.dot+1;k<(int)p.rhs.size();++k) beta.push_back(p.rhs[k]);
                    beta.push_back(it.look);
                    set<string> la = first_of_seq(beta);
                    la.erase("ε");
                    for(int j=0;j<(int)PRODS.size();++j){
                        if(PRODS[j].lhs==B){
                            for(const auto& a: la){
                                Item ni{j,0,a};
                                if(!C.count(ni)) to_add.push_back(ni);
                            }
                        }
                    }
                }
            }
        }
        for(const auto& x: to_add){ if(!C.count(x)){ C.insert(x); changed=true; } }
    }
    return C;
}

set<Item> GOTO(const set<Item>& I, const string& X){
    set<Item> moved;
    for(const auto& it: I){
        const auto& p = PRODS[it.prod];
        if(it.dot<(int)p.rhs.size() && p.rhs[it.dot]==X){
            moved.insert(Item{it.prod, it.dot+1, it.look});
        }
    }
    if(moved.empty()) return {};
    return closure(moved);
}

// canonical collection & transitions
void canonical_collection(vector< set<Item> >& C, map<pair<int,string>,int>& trans){
    C.clear(); trans.clear();
    set<Item> I0 = closure({ Item{0,0,"$"} });
    C.push_back(I0);
    bool added=true;
    while(added){
        added=false;
        for(int i=0;i<(int)C.size();++i){
            for(const auto& X: ALLSYMS){
                if(X=="S'") continue;
                set<Item> J = GOTO(C[i], X);
                if(J.empty()) continue;
                int found=-1;
                for(int k=0;k<(int)C.size();++k){ if(C[k]==J){ found=k; break; } }
                if(found==-1){
                    C.push_back(J);
                    trans[{i,X}] = (int)C.size()-1;
                    added=true;
                }else{
                    trans[{i,X}] = found;
                }
            }
        }
    }
}

// ACTION/GOTO 表（字符串简写："s5"、"r3"、"acc"）
void build_table(const vector< set<Item> >& C, const map<pair<int,string>,int>& trans,
                 map<pair<int,string>,string>& ACTION,
                 map<pair<int,string>,int>& GOTOtbl){
    ACTION.clear(); GOTOtbl.clear();
    // 移入与 GOTO
    for(auto &e: trans){
        int i = e.first.first; string X = e.first.second; int j=e.second;
        if(isTerminal(X)) ACTION[{i,X}] = string("s") + to_string(j);
        else GOTOtbl[{i,X}] = j;
    }
    // 规约与 acc
    for(int i=0;i<(int)C.size();++i){
        for(const auto& it: C[i]){
            const auto& p = PRODS[it.prod];
            bool at_end = (it.dot==(int)p.rhs.size());
            if(p.lhs=="S'" && at_end && it.look=="$"){
                ACTION[{i,"$"}] = "acc";
            }else if(at_end){
                string key = it.look;
                string act = string("r") + to_string(it.prod);
                auto kk = make_pair(i,key);
                if(ACTION.count(kk) && ACTION[kk]!=act){
                    cerr << "冲突: state "<<i<<" look "<<key<<" has "<<ACTION[kk]<<" vs "<<act<<"\n";
                }
                ACTION[kk]=act;
            }
        }
    }
}

// 打印工具
void print_itemset(const set<Item>& I){
    vector<Item> v(I.begin(), I.end());
    sort(v.begin(), v.end(), [](const Item&a,const Item&b){
        if(a.prod!=b.prod) return a.prod<b.prod;
        if(a.dot!=b.dot) return a.dot<b.dot;
        return a.look<b.look;
    });
    for(const auto& it: v){
        const auto& p = PRODS[it.prod];
        cout << "[" << p.lhs << " -> ";
        for(int k=0;k<(int)p.rhs.size();++k){
            if(k==it.dot) cout << "· ";
            cout << p.rhs[k] << ' ';
        }
        if(it.dot==(int)p.rhs.size()) cout << "· ";
        cout << ", " << it.look << "]\n";
    }
}

void print_collection(const vector< set<Item> >& C, const map<pair<int,string>,int>& trans){
    for(int i=0;i<(int)C.size();++i){
        cout << "State "<< i << ":\n";
        print_itemset(C[i]);
        // outgoing
        vector<pair<string,int>> outs;
        for(const auto& e: trans){ if(e.first.first==i) outs.push_back({e.first.second,e.second}); }
        sort(outs.begin(), outs.end());
        if(!outs.empty()){
            cout << "Transitions: ";
            for(size_t k=0;k<outs.size();++k){
                if(k) cout << ", ";
                cout << outs[k].first << "->" << outs[k].second;
            }
            cout << "\n";
        }
        cout << "\n";
    }
}

void print_table(const map<pair<int,string>,string>& ACTION, const map<pair<int,string>,int>& GOTOtbl){
    // ACTION
    cout << "ACTION table:\n";
    cout << left << setw(6) << "st";
    for(const auto& t: TERMS) cout << left << setw(8) << t; cout << "\n";
    // gather states
    set<int> states;
    for(const auto& e: ACTION) states.insert(e.first.first);
    for(const auto& e: GOTOtbl) states.insert(e.first.first);
    for(int s: states){
        cout << left << setw(6) << s;
        for(const auto& t: TERMS){
            auto it = ACTION.find({s,t});
            cout << left << setw(8) << (it==ACTION.end()?"":it->second);
        }
        cout << "\n";
    }
    cout << "\nGOTO table:\n";
    cout << left << setw(6) << "st";
    vector<string> nts; for(const auto& A: NONTERMS) if(A!="S'") nts.push_back(A);
    for(const auto& A: nts) cout << left << setw(8) << A; cout << "\n";
    for(int s: states){
        cout << left << setw(6) << s;
        for(const auto& A: nts){
            auto it = GOTOtbl.find({s,A});
            if(it==GOTOtbl.end()) cout << left << setw(8) << "";
            else cout << left << setw(8) << it->second;
        }
        cout << "\n";
    }
}

// 解析步骤打印
struct Step{ vector<int> st; vector<string> sy; vector<string> in; string act; };

vector<int> parse_run(const vector<string>& input_tokens,
                      const map<pair<int,string>,string>& ACTION,
                      const map<pair<int,string>,int>& GOTOtbl,
                      vector<Step>& steps){
    vector<int> S; S.push_back(0);
    vector<string> Y; Y.push_back("$");
    vector<string> inp = input_tokens; // 已包含 $
    vector<int> reductions; int pos=0; int step=0;

    auto snapshot=[&](){ Step x; x.st=S; x.sy=Y; x.in=vector<string>(inp.begin()+pos, inp.end()); steps.push_back(x); };

    while(true){
        int s = S.back(); string a = inp[pos];
        snapshot(); // fill st, sy, in first
        string act = ""; auto it = ACTION.find({s,a}); if(it!=ACTION.end()) act=it->second; else act="";
        steps.back().act = act;
        if(act.size()==0){
            cerr << "语法错误: 在状态 "<<s<<" 读入 '"<<a<<"'"<<"\n";
            break;
        }
        if(act=="acc"){ break; }
        if(act[0]=='s'){
            int t = stoi(act.substr(1));
            Y.push_back(a); S.push_back(t); pos++;
        }else if(act[0]=='r'){
            int pid = stoi(act.substr(1));
            const auto& p = PRODS[pid];
            reductions.push_back(pid);
            for(size_t k=0;k<p.rhs.size();++k){ if(!Y.empty()) Y.pop_back(); if(!S.empty()) S.pop_back(); }
            int s2 = S.back();
            Y.push_back(p.lhs);
            auto go = GOTOtbl.find({s2, p.lhs});
            if(go==GOTOtbl.end()){ cerr << "GOTO 未定义: ("<<s2<<","<<p.lhs<<")\n"; break; }
            S.push_back(go->second);
        }else{
            cerr << "未知动作: "<<act<<"\n"; break;
        }
        step++;
        if(step>10000){ cerr << "超出步数上限\n"; break; }
    }
    return reductions;
}

int main(){
    ios::sync_with_stdio(false); cin.tie(nullptr);

    int N; if(!(cin>>N)) return 0; string arrow;
    vector<Production> input_prods; input_prods.reserve(N);
    for(int i=0;i<N;++i){
        string lhs; cin>>lhs; cin>>arrow; // '->'
        vector<string> rhs; string tok; string line;
        getline(cin,line); // 读到换行
        stringstream ss(line);
        while(ss>>tok){ if(tok=="epsilon"||tok=="ε") continue; rhs.push_back(tok); }
        input_prods.push_back({lhs,rhs});
    }
    string start; cin>>start;
    int T; cin>>T; TERMS.clear(); TERMS.reserve(T+1);
    for(int i=0;i<T;++i){ string t; cin>>t; if(t!="$") TERMS.push_back(t); }
    TERMS.push_back("$");
    int V; cin>>V; NONTERMS.clear(); NONTERMS.reserve(V+1);
    for(int i=0;i<V;++i){ string A; cin>>A; NONTERMS.push_back(A); }
    if(find(NONTERMS.begin(),NONTERMS.end(),"S'")==NONTERMS.end()) NONTERMS.push_back("S'");

    int L; cin>>L; vector<string> sent; sent.reserve(L+1);
    for(int i=0;i<L;++i){ string a; cin>>a; sent.push_back(a); }
    sent.push_back("$");

    // 构造增广文法
    PRODS.clear(); PRODS.push_back({"S'", {start}});
    for(auto&p: input_prods) PRODS.push_back(p);

    // ALLSYMS and FIRST base
    ALLSYMS.clear(); for(auto&t:TERMS) ALLSYMS.insert(t); for(auto&A:NONTERMS) ALLSYMS.insert(A);
    for(auto&t:TERMS) FIRST[t].insert(t);

    compute_first();

    // 规范族 & 解析表
    vector< set<Item> > C; map<pair<int,string>,int> trans; canonical_collection(C,trans);

    // (1) 打印项目集规范族
    cout << "==== Canonical LR(1) Item Sets (DFA states) ====\n\n";
    print_collection(C,trans);

    map<pair<int,string>,string> ACTION; map<pair<int,string>,int> GOTOtbl;
    build_table(C,trans,ACTION,GOTOtbl);

    // (2) 打印 LR 表
    cout << "\n==== LR Parsing Tables ====\n\n";
    print_table(ACTION,GOTOtbl);

    // (3) 逐步分析
    cout << "\n==== Parsing Trace ====\n";
    vector<Step> trace; vector<int> reds = parse_run(sent, ACTION, GOTOtbl, trace);

    cout << left << setw(18) << "StateStack" << setw(20) << "SymbolStack" << setw(20) << "Input" << "Action" << "\n";
    for(const auto& st: trace){
        // state stack
        string a=""; for(size_t i=0;i<st.st.size();++i){ if(i) a+=' '; a+=to_string(st.st[i]); }
        string b=""; for(size_t i=0;i<st.sy.size();++i){ if(i) b+=' '; b+=st.sy[i]; }
        string c=""; for(size_t i=0;i<st.in.size();++i){ if(i) c+=' '; c+=st.in[i]; }
        cout << left << setw(18) << a << setw(20) << b << setw(20) << c << st.act << "\n";
    }

    if(!trace.empty() && trace.back().act=="acc"){
        if (!trace.empty() && trace.back().act == "acc") {
        cout << "\nAccept. Reductions in order:\n";
        for (int x : reds) {
        cout << "r" << x << ": " << PRODS[x].lhs << " -> ";
        if (PRODS[x].rhs.empty()) cout << "ε";
        else {
            for (size_t k = 0; k < PRODS[x].rhs.size(); ++k) {
                if (k) cout << ' ';
                cout << PRODS[x].rhs[k];
            }
        }
        cout << '\n';
    }
}

    }else{
        cout << "\nParsing failed (see trace).\n";
    }

    return 0;
}
