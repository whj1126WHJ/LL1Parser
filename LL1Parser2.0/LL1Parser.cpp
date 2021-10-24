//
// Created by 土旦木的小世界 on 2021/10/19.
//

#include "LL1Parser.h"

/*1 -> startSymbol;
  2 -> non_terminal_map;
  3 -> terminal_symbols_map;
 */

//todo 未考虑num、id这种类别型的终结符，只是简单的用string处理，若出现终结符a和Ha则会误判

vector<string> terminal;
unordered_set<string> terminal_set;
vector<string> nonterminal;
string start;
string s;   //待分析表达式

unordered_map<string, vector<string>> non_terminal_map;
unordered_set<string> terminal_symbols_map;
unordered_map<string, string> First;//todo string改为vector<string>
unordered_map<string, string> Follow;//todo string改为vector<string>
unordered_map<string, string> still_empty_follow;
struct StringPairHasher
{
    size_t operator()(
            const std::pair<std::string, std::string> &obj) const
    {
        static const std::hash<std::string> hasher;
        return (hasher (obj.first) << 16) + hasher (obj.second);
    }
};
unordered_map <
        pair<string, string>,
        vector<string>,
        StringPairHasher
> table;
        //unordered_map<vector<vector<string>>, string> table;

//vector<vector<string>> table;

int nums_nontermial = 0;

//读入源文件
void read_resource() {

}

//消除左递归
void fix_left_recursion() {

}

//消除左公因子
void fix_left_common_factor() {

}


/*读入已经处理过不包含回溯和左递归的文法输入文件，-> |等符号均省略为空格表示
 * 格式如下
 * 起始符
 * 终结符
 * 文法
*/
void read_input_out_text() {
    ifstream infile;
    infile.open("input.out.txt", ios::in);
    if(!infile.is_open()) {
        cout << "文件读取失败" << endl;
        return ;
    }
    string buf;
    getline(infile, buf);   //读入终结符号
    terminal = split_blank(buf);
    getline(infile, buf);   //读入起始符号
    start = buf;
    //放入set集中，便于后续判断
    for(int i = 0; i < terminal.size(); i++) {
        terminal_set.emplace(terminal[i]);
    }
    //读入文法
    while(getline(infile, buf)) {
        nums_nontermial++;
        vector<string> buff = split_blank(buf); //整行读取
        nonterminal.push_back(buff[0]); //获取左式
        string tmp = buff[0];
        buff.assign(buff.begin() + 1, buff.end());  //截取第二至最后一个字符串
        non_terminal_map.emplace(tmp, buff);    //放入map中
    }
    cout << "whj";
}

void find_terminal(const string& left, const string& right, int times) {
    if(terminal_set.find(right.substr(0, 1)) != terminal_set.end()) {    //找到了终结符
        //non_terminal_map[left].push_back(right);
        //First.emplace(left, right);
        //if(terminal_set.find(right.substr(0, 1)) != terminal_set.end())
        First[left] += right.substr(0, 1);
        return;
    }
    else if(right == "epsilon")
        First[left] += right;
    else if(times < nums_nontermial) {
        for(auto & i : non_terminal_map[right]) {
            find_terminal(left, i, times+1);
        }
        return;
    }
}

//获取FIRST集
void getFirst() {
    for(auto &i : non_terminal_map) {
        for(int j = 0; j < i.second.size(); j++) {
            vector<string> tem = i.second;
            string buf_sub;
            buf_sub = i.second[j].substr(0, 1);//右边最左字符
            if(terminal_set.find(buf_sub) != terminal_set.end()) {  //若为终结符，则加入First
                First[i.first] += buf_sub;
                //First.emplace(i.first, buf_sub);
            }
            else if(i.second[j] == "epsilon")
                First[i.first] += "epsilon";
            else {  //  若为非终结符
                find_terminal(i.first, buf_sub, 0);
            }
        }
    }
    cout << "First success" << endl;
}

//获取FOLLOW集
//todo epsilon未处理
void getFollow() {
    for(auto &i : non_terminal_map) {
        for(auto &j : non_terminal_map) {
            for(int k = 0; k < j.second.size(); k++) {
                int idx = j.second[k].find(i.first);
                if(i.first == start) {  //如果是起始符，则加入Follow
                    Follow[i.first] += "$";
                }
                if(idx != string::npos) { //如果该字符串存在i->first
                    //if(idx != i.first.size()-1) {    //若不是最后一个字符，则加入Follow
                    if(idx != j.second[k].size()-1) {
                        if(terminal_set.find(j.second[k].substr(idx+1, 1)) != terminal_set.end())
                            Follow[i.first] += j.second[k].substr(idx+1, 1);
                        else {
                            if(First[j.second[k].substr(idx+1, 1)].find("epsilon") != string::npos) {
                                still_empty_follow[i.first] += j.first;
                            }
                            Follow[i.first] += First[j.second[k].substr(idx+1, 1)];
                        }
                    }
                    else {  //为最后字符，则把左式的Follow集全部加入
                        if(Follow[j.first].empty())
                            still_empty_follow[i.first] += j.first;
                        else
                            Follow[i.first] += Follow[j.first];
                    }
                }
            }
        }
    }
    for(auto i : still_empty_follow) {
//        int pos = i.second.find("epsilon");
//        if(pos != string::npos)
//            i.second.erase(pos, 7);
        for(auto j : i.second) {
            string idx;
            idx = j;
            Follow[i.first] += Follow[idx];
        }
    }
    cout << "Follow_set succeed!" << endl;
}

void make_analyze_table() {
    for(auto &i : non_terminal_map) {
        for(int j = 0; j < terminal.size(); j++) {
            auto key = make_pair(i.first, terminal[j]);
            if(First[i.first].find(terminal[j]) != string::npos) {
                table[key] = i.second;
            }
            if(First[i.first].find("epsilon") != string::npos) {
                for(auto k : Follow) {
                    table[key].push_back(k.second);
                }
            }
        }
    }
    cout << "table make success" << endl;
    //todo 错误处理
}

//
void analyz() {
    stack<string> status, input;
    status.push("$");
    reverse(s.begin(), s.end());
    input.push("$");
    for(auto i : s) {
        string t;
        t = i;
        input.push(t);
    }

    while(!status.empty() && !input.empty()) {
        string sta = status.top(), inp = input.top();
        auto key = make_pair(sta, inp);
        status.pop();
        if(!table[key].empty()) {
            //逆序压栈
            for(int i = table[key][0].size()-1; i >= 0; i--) {
                string t;
                t = table[key][0][i];
                status.push(t);
            }
        }
        else if(sta == inp) {
            input.pop();
        }
    }

}

void output() {

}

int main() {
    cout << "请输入待分析的表达式：";
    cin >> s;
    read_input_out_text();
    getFirst();
    getFollow();
    make_analyz_table();
    analyz();
}