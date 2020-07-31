#include <iostream>
#include <vector>
#include <set>
#include <queue>
#include <string>
#include <utility>
#include <cctype>
using namespace std;

//根据算法生成的NFA只有ε有可能指向两个
//其他都只会指向一个
//0~25表示a-z
//26~27表示ε
class Regex
{
public:
    string regex;
    int index;
    int nfa_count;
    int NFA_transition[100][28];
    int DFA_transition[100][26];
    pair<int, int> NFA;
    set<int> DFA_final; //start state = 0
    int DFA_count;

    Regex(string regex);
    pair<int, int> S();
    pair<int, int> S_(pair<int, int> pre_nfa);
    pair<int, int> E();
    pair<int, int> E_(pair<int, int> pre_nfa);
    pair<int, int> T();
    pair<int, int> T_(pair<int, int> pre_nfa);
    pair<int, int> F();
    void epsilon_closure(set<int> &s);
    set<int> NFA_move(set<int> &s, int a);
    void NFA2DFA();
};

Regex::Regex(string s)
{
    regex = s + '$';
    index = 0;
    nfa_count = 0;
    for (int i = 0; i < 100; i++)
        for (int j = 0; j < 28; j++)
            NFA_transition[i][j] = -1;
    for (int i = 0; i < 100; i++)
        for (int j = 0; j < 26; j++)
            DFA_transition[i][j] = -1;
    NFA = S();
    NFA2DFA();
    // cout<<DFA_count<<" +++ ";
    // for(int i:DFA_final)
    //     cout<<i<<" ";
    // cout<<endl;
}
// pair<int, int> Regex::S()
// {
//     int start = index;
//     pair<int, int> E_nfa = E();
//     pair<int, int> temp = S_(E_nfa);
//     cout<<"S ----- "<<regex.substr(start, index-start)<<endl;
//     return temp;
// }
pair<int, int> Regex::S()
{
    pair<int, int> E_nfa = E();
    return S_(E_nfa);
}
pair<int, int> Regex::S_(pair<int, int> pre_nfa)
{
    if (regex[index] == '|')
    {
        index++;
        pair<int, int> E_nfa = E();
        pair<int, int> newNFA = make_pair(nfa_count, nfa_count + 1);
        nfa_count += 2;
        NFA_transition[newNFA.first][26] = pre_nfa.first;
        NFA_transition[newNFA.first][27] = E_nfa.first;
        NFA_transition[pre_nfa.second][26] = newNFA.second;
        NFA_transition[E_nfa.second][26] = newNFA.second;
        return S_(newNFA);
    }
    else
        return pre_nfa;
}
// pair<int, int> Regex::E()
// {
//     int start = index;
//     pair<int, int> T_nfa = T();
//     pair<int, int> temp = E_(T_nfa);
//     cout<<"E ----- "<<regex.substr(start, index-start)<<endl;
//     return temp;
// }
pair<int, int> Regex::E()
{
    pair<int, int> T_nfa = T();
    return E_(T_nfa);
}
pair<int, int> Regex::E_(pair<int, int> pre_nfa)
{
    if (regex[index] == '(' || isalpha(regex[index]))
    {
        pair<int, int> T_nfa = T();
        NFA_transition[pre_nfa.second][26] = T_nfa.first;
        return E_(make_pair(pre_nfa.first, T_nfa.second));
    }
    else
        return pre_nfa;
}
// pair<int, int> Regex::T()
// {
//     int start = index;
//     pair<int, int> F_nfa = F();
//     pair<int, int> temp = T_(F_nfa);
//     cout<<"T ----- "<<regex.substr(start, index-start)<<endl;
//     return temp;
// }
pair<int, int> Regex::T()
{
    pair<int, int> F_nfa = F();
    return T_(F_nfa);
}
pair<int, int> Regex::T_(pair<int, int> pre_nfa)
{
    pair<int, int> newNFA = make_pair(nfa_count, nfa_count + 1);
    switch (regex[index])
    {
    case '*':
        NFA_transition[newNFA.first][27] = newNFA.second;
        NFA_transition[pre_nfa.second][27] = pre_nfa.first;
        break;
    case '?':
        NFA_transition[newNFA.first][27] = newNFA.second;
        break;
    case '+':
        NFA_transition[pre_nfa.second][27] = pre_nfa.first;
        break;
    default:
        return pre_nfa;
    }
    index++;
    nfa_count += 2;
    NFA_transition[newNFA.first][26] = pre_nfa.first;
    NFA_transition[pre_nfa.second][26] = newNFA.second;
    return newNFA;
}
pair<int, int> Regex::F()
{
    if (regex[index] == '(')
    {
        index++;
        pair<int, int> S_nfa = S();
        if (regex[index] != ')')
            throw "The string is not compatible with the grammar!";
        index++;
        return S_nfa;
    }
    else if (isalpha(regex[index]))
    {
        pair<int, int> newNFA = make_pair(nfa_count, nfa_count + 1);
        nfa_count += 2;
        if (regex[index] == 'E')
            NFA_transition[newNFA.first][26] = newNFA.second;
        else
            NFA_transition[newNFA.first][regex[index] - 'a'] = newNFA.second;
        index++;
        return newNFA;
    }
    else
        throw "The string is not compatible with the grammar!";
}

void Regex::epsilon_closure(set<int> &s)
{
    queue<int> q;
    for (int i : s)
        q.push(i);

    int state, nextState;
    while (!q.empty())
    {
        state = q.front();
        q.pop();
        for (int i : {26, 27})
        {
            nextState = NFA_transition[state][i];
            if (nextState != -1 && s.count(nextState) == 0)
            {
                s.insert(nextState);
                q.push(nextState);
            }
        }
    }
    return;
}
set<int> Regex::NFA_move(set<int> &s, int a)
{
    set<int> nextStates;
    for (int i : s)
        if (NFA_transition[i][a] != -1)
            nextStates.insert(NFA_transition[i][a]);
    return nextStates;
}
//如果set已存在，则返回下标
//如果set不存在，则插入vector中并返回下标
int insertSet(vector<set<int>> &Dstates, set<int> &states)
{
    bool flag;
    for (int i = 0; i < Dstates.size(); i++)
    {
        if (Dstates[i].size() != states.size())
            continue;
        flag = true;
        for (int j : Dstates[i])
            if (states.count(j) == 0)
                flag = false;
        if (flag)
            return i;
    }
    Dstates.push_back(states);
    return Dstates.size() - 1;
}
void Regex::NFA2DFA()
{
    set<int> StartStates = {NFA.first};
    set<int> nextStates;
    epsilon_closure(StartStates);
    vector<set<int>> Dstates = {StartStates};
    int index;
    for (int i = 0; i < Dstates.size(); i++)
    {
        for (int j = 0; j < 26; j++)
        {
            nextStates = NFA_move(Dstates[i], j);
            if (!nextStates.empty())
            {
                epsilon_closure(nextStates);
                index = insertSet(Dstates, nextStates);
                DFA_transition[i][j] = index;
            }
        }
        if (Dstates[i].count(NFA.second) != 0)
            DFA_final.insert(i);
    }
    DFA_count = Dstates.size();
    return;
}
//0 --- r1 = r2
//1 --- r1 < r2
//2 --- r1 > r2
//3 --- r1 ! r2
int relationOfRegex(Regex r1, Regex r2)
{
    int visited[100][100] = {0};
    visited[1][1] = 1; //0,0
    queue<pair<int, int>> q;
    q.push(make_pair(0, 0));
    pair<int, int> state;
    int result = 0, nextS1, nextS2, curS1, curS2;
    while (!q.empty())
    {
        state = q.front();
        curS1 = state.first;
        curS2 = state.second;
        q.pop();
        for (int i = 0; i < 26; i++)
        {
            nextS1 = (curS1 == -1) ? -1 : r1.DFA_transition[curS1][i];
            nextS2 = (curS2 == -1) ? -1 : r2.DFA_transition[curS2][i];
            if (visited[nextS1 + 1][nextS2 + 1] == 0)
            {
                visited[nextS1 + 1][nextS2 + 1] = 1;
                q.push(make_pair(nextS1, nextS2));
            }
        }
        if (r1.DFA_final.count(curS1) == 0 && r2.DFA_final.count(curS2) != 0)
            result |= 1;
        if (r1.DFA_final.count(curS1) != 0 && r2.DFA_final.count(curS2) == 0)
            result |= 2;
    }
    return result;
}

char results[] = {'=', '<', '>', '!'};
void test()
{
    string s1, s2;
    cin >> s1 >> s2;
    Regex r1(s1), r2(s2);
    int index = relationOfRegex(r1, r2);
    cout << results[index] << endl;
    return;
}
int main()
{
    int n;
    cin >> n;
    for (int i = 0; i < n; i++)
        test();
    return 0;
    // for(int i=0;i<tempR.DFA_count;i++)
    // {
    //     printf("%4d --- ", i);
    //     for(int j=0;j<3;j++)
    //         printf("%4d", tempR.DFA_transition[i][j]);
    //     printf("\n");
    // }
}
