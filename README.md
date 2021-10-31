# 编译原理与设计



**题目**：语法分析程序的设计与实现

**班级**：<u>2019211308</u>

**学号**：<u>2019211517</u>

**姓名**：<u>王恒杰</u>



## 实验内容

* ### 题目：

  与法分析程序的设计与实现

* ### 实验内容

  编写语法分析程序，实现对算术表达式的语法分析。

* ### 实验要求

  在对输入的算术表达式进行分析的过程中，一次输出所采用的产生式。

* ### 实验方法

  编写LL(1)语法分析程序

  1. 编程实现课本算法4.2，为给定文法自动构造预测分析表

  2. 编写实现课本算法4.1，构造LL(1)预测分析程序。

     

## 运行开发环境

* **系统环境**：macOS Big Sur: 11.4

* **编译环境**：Clion: 11.0.10+9-b1341.41 aarch64

* **版本管理工具**：github

  

## 设计、实现说明

* ### 该要设计

  #### 1. 实现为给定文法自动构造预测分析表

  * 从input.txt文件中读取未经处理的原始文法
  * 消除左递归
  * 消除左公因子
  * 打印文法，并将处理后的文法输出到input.out.txt文件
  * 构造LL1预测分析表
    * 构造FIRST集
    * 构造FOLLOW集
    * 生成分析表
    * 加入错误处理sync
    * 打印LL1分析表，并将分析表输出到LL1_table.txt文件

  #### 2. 构造LL1预测分析程序

  * 读入待分析表达式
  * 读入LL1分析表
    * 初始化分析器
    * 循环直到栈空
    * 输出每次所采用的产生式
  * 输出分析结果(接受或不接受)

  ![image-20211025090032129](https://tva1.sinaimg.cn/large/008i3skNly1gvra1fy6ssj60ve0u0q6e02.jpg)

  <!--之所以把左递归回溯、构造分析表、LL1分析程序三者分开，是出于对提高分析程序的简洁与运行性能的考量-->

* ### 详细设计

* ##### 1. 结构变量说明

  ```C++
  set<string> non_terminal, terminal; //使用set存储非终结符与终结符，省去对重复项的过滤
  map<string, set<string>> first, follow; //使用hash_map存储first、follow集，string到set<string>的映射同样可以省去对重复项的过滤
  map<string, vector<string>> G;  //存储文法
  map<string, string> still_empty;    //用于构造follow集时出现的未计算的产生式
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
  unordered_map<pair<string, string>, vector<string>, StringPairHash> table;  //预测分析表，使用pair<string, string>到vector<string>的映射
  ```

* ##### 2. 函数成员

  ```C++
  void fix_left_recursion();	//消除左递归
  void fix_left_common_factor();	//消除回溯
  void read_input_out_txt();	//读取处理好的文法输入文件
  void get_first();	//获取first集
  void get_follow();	//获取follow集
  void make_table();	//构造预测分析表
  void analyze();	//预测分析
  void find_terminal(const string &left, const string &right, int times, const string& init);	//求解first集所用递归函数
  vector<string> split_blank(const string& str);	//获取文件读入划分字符串
  ```

  

* ##### 3. 构造FIRST集

  ![image-20211025091013728](https://tva1.sinaimg.cn/large/008i3skNly1gvrabhc6suj60u010lq6402.jpg)

​	

```C++
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
    //cout << "first get" << endl;
}
```



* ##### 4. 构造FOLLOW集

  ![image-20211025091225350](https://tva1.sinaimg.cn/large/008i3skNly1gvradrrpjcj60u00ysgpl02.jpg)



```C++
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
    //cout << "follow get" << endl;
}
```

* ##### 5. 构造预测分析表

  ![image-20211025093336051](https://tva1.sinaimg.cn/large/008i3skNly1gvrazst6o6j60jc12odin02.jpg)	

```C++
void analyze() {
    stack<string> status, input;
    status.push("$");
    status.push(start);
    input.push("$");
    for(int i = (int)s.size()-1; i >= 0; i--) {
        input.push(s.substr(i, 1));
    }

  //格式化输出预测分析表
    int cnt = 1;
    while(status.top() != "$" && input.top() != "$") {
        string sta = status.top(), inp = input.top();
        auto key = make_pair(sta, inp);
        if(non_terminal.find(sta) != non_terminal.end()) {
            status.pop();
            if(!table[key].empty()) {
                //输出
                cout << "(" <<cnt << ") " << sta << " -> " << table[key][0] << endl;
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
        else {
            cout << "wrong sentence" << endl;
            return;
        }
    }
    cout << "analyze get" << endl;
}
```



* ##### 6. 预测分析表达式

  ![image-20211025103313050](https://tva1.sinaimg.cn/large/008i3skNly1gvrcpu5oz0j60t8112gps02.jpg)



```C++
void analyze() {
    stack<string> status, input;
    status.push("$");
    status.push(start);
    input.push("$");
    for(int i = (int)s.size()-1; i >= 0; i--) {
        input.push(s.substr(i, 1));
    }

    int cnt = 1;
    while(status.top() != "$" && input.top() != "$") {
        string sta = status.top(), inp = input.top();
        auto key = make_pair(sta, inp);
        if(non_terminal.find(sta) != non_terminal.end()) {
            status.pop();
            if(!table[key].empty()) {
                //输出
                cout << "(" <<cnt << ") " << sta << " -> " << table[key][0] << endl;
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
        else {
            cout << "wrong sentence" << endl;
            return;
        }
    }
    cout << "analyze get" << endl;
}
```



## 测试说明

* ##### 使用说明

  为便于测试，将文件简化为一个cpp文件与一个txt输入文件，分别为

  * LL1Paraser3_0.cpp

  * input.out.txt

    输入文件格式为：

    1. 起始符
    2. 非终结符
    3. 终结符
    4. 产生式

    示例：

![image-20211025104959184](https://tva1.sinaimg.cn/large/008i3skNly1gvrd7a3ydjj606g0a0q2v02.jpg) </left>

* ##### 输出

* 

  ```
                 (              )              ,              a              b              $
  A                                                           A -> a         A -> b               
  B              B -> (L)                                                                           
  E              E -> B                                       E -> A         E -> A               
  L              L -> EM                                      L -> EM        L -> EM               
  M                                            M -> ,L                                             
  
  ```

* ```
  (1) E -> B
  (2) B -> (L)
  (3) L -> EM
  (4) E -> A
  (5) A -> a
  (6) M -> ,L
  (7) L -> EM
  (8) E -> B
  (9) B -> (L)
  (10) L -> EM
  (11) E -> A
  (12) A -> a
  (13) M -> ,L
  (14) L -> EM
  (15) E -> B
  (16) B -> (L)
  (17) L -> EM
  (18) E -> A
  (19) A -> b
  (20) M -> ,L
  (21) L -> EM
  (22) E -> B
  (23) B -> (L)
  (24) L -> EM
  (25) E -> A
  
  ```

  
