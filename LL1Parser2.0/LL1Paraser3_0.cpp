//
// Created by 土旦木的小世界 on 2021/10/24.
//
#include "utils.h"
#include <unordered_map>
#include <iomanip>

using namespace std;

set<string> non_terminal, terminal;
map<string, set<string>> first, follow;
map<string, vector<string>> G;
map<string, string> still_empty;
string s;   //待分析表达式
string start;   //起始非终结符

struct StringPairHash
{
    size_t operator()(
            const pair<std::string, string> &obj) const
    {
        static const hash<string> hash;
        return (hash (obj.first) << 16) + hash (obj.second);
    }
};
unordered_map<pair<string, string>, vector<string>, StringPairHash> table;

void read_input_out_txt();
void get_first();
void get_follow();
void make_table();
void analyze();
//void output();
void find_terminal(const string &left, const string &right, int times, const string& init);

int main() {
    cout << "请输入需要分析的表达式：" << endl;
    cin >> s;

    read_input_out_txt();
    get_first();
    //get_follow_2();
    get_follow();
    make_table();
    analyze();
    //output();
    return 0;
}



void read_input_out_txt() {
    ifstream infile;
    infile.open("input.out.txt", ios::in);
    if(!infile.is_open()) {
        cout << "文件读取失败" << endl;
        return ;
    }
    string buf;
    vector<string> tem;

    //首行读入起始非终结符
    getline(infile, buf);
    start = buf;

    //二行读入非终结符
    getline(infile, buf);
    tem = split_blank(buf);
    for(const auto& i : tem) {
        non_terminal.emplace(i);
    }

    //三行读入终结符
    getline(infile, buf);
    tem = split_blank(buf);
    for(const auto& i : tem) {
        terminal.emplace(i);
    }

    //后续读入文法
    while(getline(infile, buf)) {
        tem = split_blank(buf);
        for(int i = 1; i < tem.size(); i++) {
            G[tem[0]].push_back(tem[i]);
        }
    }
    cout << "文件读取完成" << endl;
}

void find_terminal(const string &left, const string &right, int times, const string& init) {
    if(terminal.find(right.substr(0, 1)) != terminal.end()) {
        first[left].insert(right.substr(0, 1));
        first[init].insert(right.substr(0, 1));
        return;
    }
    else if(times < non_terminal.size()){
        for(auto &i : G[right.substr(0, 1)]) {
            find_terminal(left, i, times+1, init);
        }
    }
    else return;
}

void get_first() {
    for(const auto& i : non_terminal) { //遍历每个非终结符
        for(const auto& j : G[i]) { //遍历每个非终结对应右句
            if(terminal.find(j.substr(0, 1)) != terminal.end()) {   //如果右式当前遍历项为非终结符，则加入first
                first[i].insert(j.substr(0, 1));
                first[j].insert(j.substr(0, 1));
                //first[j] += j.substr(0, 1);
            }
            else {
                find_terminal(i, j, 0, j);
            }
        }
    }
    cout << "first get" << endl;
}

void get_follow() {
    //规则1
    follow[start].insert("$");
    int pre_size = -1, now_size = 0;
    while(pre_size != now_size) {
        pre_size = now_size;
        for(const auto& i : G) {
            vector<string> elements = i.second;
            for(auto & element : elements) {
                if(element[0] != '~') { //当前非epsilon
                    for(int k = 0; k < element.size()-1; k++) {
                        string cur = element.substr(k, 1), next = element.substr(k+1, 1);
                        if(terminal.find(cur) != terminal.end()) { //若当前字符为终结符
                            follow[cur].insert(cur);    //终结符本身的follow集即为自身
                        }
                        else{
                            if(terminal.find(next) != terminal.end()) {  //若下一个为终结符
                                follow[cur].insert(next);
                            }
                            else {  //下一个为非终结符，加入next的first
                                //if(first[next].find("~") == first[next].end()) {    //若下一个是非终结符并且该非终结符不包含epsilon
                                    for(const auto& l : first[next]) {  //加入他的first集
                                        if(l != "~")
                                            follow[cur].insert(l);
                                    }
                                //}
                                if(first[next].find("~") != first[next].end()) {  //若下一个是非终结符并且包含epsilon
                                    if(!follow[i.first].empty())
                                    for(const auto& l : follow[i.first]) {
                                        follow[cur].insert(l);
                                        still_empty[next] = cur;
                                    }
                                    else {
                                        still_empty[next] = cur;
                                    }
                                }
                            }
                        }
                    }
                    string k = element.substr((int)element.size()-1 ,1);
                    if(terminal.find(k) == terminal.end()) {
                        if(!follow[i.first].empty()) {  //若不为空
                            for(const auto& l : follow[i.first]) {
                                follow[k].insert(l);
                                still_empty[k] = i.first;
                            }
                        }
                        else {
                            still_empty[k] = i.first;
                        }
                    }
                }
            }
        }
        for(const auto& i : still_empty) {
            follow[i.first].insert(follow[i.second].begin(), follow[i.second].end());
        }
        now_size = 0;
        for(const auto& i : non_terminal) {
            now_size += (int)follow[i].size();
        }
    }
    cout << "follow get" << endl;
}

void make_table() {
    for(const auto& i : G) {
        for(const auto& j : i.second) {
            for(const auto& k : terminal) {
                auto key = make_pair(i.first, k);
                if(first[j].find(k) != first[j].end()) {
                    table[key].push_back(j);
                }
            }
            if(first[j].find("~") != first[j].end()) {
                for(const auto&l : follow[i.first]) {
                    auto key = make_pair(i.first, l);
                    table[key].push_back(i.first);
                }
            }
        }
    }

//    cout << setw(10);
//    for(auto i : terminal) {
//        cout << i << "          ";
//    }
//    cout << endl;
//    for(auto i : non_terminal) {
//        cout << i << setw(10);
//        for(auto j : terminal) {
//            auto key = make_pair(i, j);
//            if(!table[key].empty())
//                cout << i << " -> " << table[key][0] << setw(10);
//            else
//                cout << "          ";
//        }
//        cout << endl;
//    }
    cout << "table get" << endl;
}

void analyze() {
    stack<string> status, input;
    status.push("$");
    status.push(start);
    input.push("$");
    for(int i = (int)s.size()-1; i >= 0; i--) {
        input.push(s.substr(i, 1));
    }

    while(status.top() != "$" && input.top() != "$") {
        string sta = status.top(), inp = input.top();
        auto key = make_pair(sta, inp);
        if(non_terminal.find(sta) != non_terminal.end()) {
            status.pop();
            if(!table[key].empty()) {
                //逆序压栈
                for(int i = (int)table[key][0].size()-1; i >= 0; i--) {
                    status.push(table[key][0].substr(i, 1));
                }
            }
        }
        else if(sta == inp) {
            input.pop();
            status.pop();
        }
    }
    cout << "analyze get" << endl;
}