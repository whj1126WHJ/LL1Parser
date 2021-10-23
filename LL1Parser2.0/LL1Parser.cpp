//
// Created by 土旦木的小世界 on 2021/10/19.
//

#include "LL1Parser.h"

/*1 -> startSymbol;
  2 -> non_terminal_map;
  3 -> terminal_symbols_map;
 */

//todo 为考虑num、id这种类别型的终结符，只是简单的用string处理，若出现终结符a和Ha则会误判

vector<string> terminal;
unordered_set<string> terminal_set;
vector<string> nonterminal;
string start;
string s;   //待分析表达式

unordered_map<string, vector<string>> non_terminal_map; //
unordered_set<string> terminal_symbols_map;
unordered_map<string, string> First;//todo string改为vector<string>
unordered_map<string, string> Follow;//todo string改为vector<string>
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
    getline(infile, buf);   //读入起始符号
    start = buf;
    getline(infile, buf);   //读入终结符号
    terminal = split_blank(buf);
    //放入set集中，便于后续判断
    for(int i = 0; i < terminal.size(); i++) {
        terminal_set.emplace(terminal[i]);
    }
    //读入文法
    while(getline(infile, buf)) {
        nums_nontermial++;
        vector<string> buff = split_blank(buf); //整行读取
        nonterminal.push_back(buff[0]); //获取左式
        buff.assign(buff.begin() + 1, buff.end());  //截取第二至最后一个字符串
        non_terminal_map.emplace(buff[0], buff);    //放入map中
    }
}

void find_terminal(const string& left, const string& right, int times) {
    if(terminal_set.find(right) != terminal_set.end()) {    //找到了终结符
        non_terminal_map[left].push_back(right);
        return;
    }
    else if(times < nums_nontermial) {
        for(auto & i : non_terminal_map[right]) {
            find_terminal(left, i, times+1);
        }
        return;
    }
    return;
}

//获取FIRST集
void getFirst() {
    for(auto &i : non_terminal_map) {
        for(int j = 0; j < i.second.size(); j++) {
            vector<string> tem = i.second;
            string buf_sub;
            buf_sub = i.second[j].substr(0, 1);//右边最左字符
            if(terminal_set.find(buf_sub) != terminal_set.end()) {  //若为终结符，则加入First
                First.emplace(i.first, buf_sub);
            }
            else {  //  若为非终结符
                find_terminal(i.first, buf_sub, 0);
            }
        }
    }
}

//获取FOLLOW集
void getFollow() {
    for(auto &i : non_terminal_map) {
        for(auto &j : non_terminal_map) {
            for(int k = 0; k < i.second.size(); k++) {
                int idx = j.second[k].find(i.first);
                if(i.first == start) {  //如果是起始符，则加入Follow
                    Follow.emplace(i.first, "$");
                }
                else if(idx != string::npos) { //如果该字符串存在i->first
                    if(idx != i.first.size()-1) {    //若不是最后一个字符，则加入Follow
                        Follow.emplace(i.first, i.second[k].substr(idx, idx+1));
                    }
                    else {  //为最后字符，则把左式的Follow集全部加入
                        Follow.emplace(i.first, Follow[j.first]);
                    }
                }
            }
        }
    }
}


void make_analyz_table() {
    for(auto &i : non_terminal_map) {
        for(int j = 0; j < terminal.size(); j++) {
            auto key = make_pair(i.first, terminal[j]);
            if(First.find(terminal[j]) != First.end()) {
                table[key] = i.second;
            }
            if(First.find("epsilon") != First.end()) {
                for(auto k : Follow) {
                    table[key].push_back(k.second);
                }
            }
        }
    }
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
            for(auto i : table[key]) {
                status.push(i);
            }
        }


    }

}

void output() {

}

int main() {
    cout << "请输入待分析的表达式：";
    cin >> s;

}