//
// Created by 土旦木的小世界 on 2021/10/23.
//

#include "utils.h"
vector<string> split_blank(const string& str) {
    vector<string> res;
    stringstream input(str);
    string buf;
    while(input >> buf) {
        res.push_back(buf);
    }
    return res;
}