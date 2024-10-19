#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <array>
#include <algorithm>

using namespace std;

#define EPSILON "epsilon"

std::set<string> nonTset;
std::map<std::string, std::set<std::string>> first;
std::map<std::string, std::set<std::string>> follow;
std::map<std::string, std::map<std::string, std::array<std::string, 2>>> M; // 预测分析表

// Function to split the grammar rule into left and right parts
std::vector<string> split(const std::string &grammar) {
    std::vector<string> results;
    size_t arrowPos = grammar.find("->");  

    if (arrowPos == std::string::npos) {  
        std::cerr << "Error: No '->' found in the grammar rule." << std::endl;  
        return {};  
    } 

    std::string leftSide = grammar.substr(0, arrowPos);
    results.push_back(leftSide);
    std::string rightSide = grammar.substr(arrowPos + 2);
    size_t barPos = rightSide.find("|");  

    while (barPos != std::string::npos) {  
        results.push_back(rightSide.substr(0, barPos));
        rightSide = rightSide.substr(barPos + 1);
        barPos = rightSide.find("|");  
    }

    results.push_back(rightSide);
    return results;
}

// Function to eliminate left recursion from grammar grammar
void eliminateLeftRecursion(std::vector<std::vector<string>> &grammar) {
    for (int i = 0; i < grammar.size(); ++i) {
        std::vector<string> alpha; // 左递归部分
        std::vector<string> beta;  // 非左递归部分
        std::string nonTerminal = grammar[i][0]; // 当前非终结符
        bool hasLeftRecursion = false;

        for (int j = 1; j < grammar[i].size(); ++j) {
            if (grammar[i][j].find(nonTerminal) == 0) {
                hasLeftRecursion = true;
                alpha.push_back(grammar[i][j].substr(nonTerminal.length())); // 提取左递归部分
            } else {
                beta.push_back(grammar[i][j]); // 非左递归部分
            }
        }

        if (hasLeftRecursion) {
            // 新的非终结符名称
            std::string newNonTerminal = nonTerminal + "'";

            // 生成非左递归规则：A -> B A'
            grammar[i].clear();
            grammar[i].push_back(nonTerminal);
            for (const auto& b : beta) {
                grammar[i].push_back(b + newNonTerminal);
            }

            // 生成左递归规则：A' -> A' alpha | epsilon
            std::vector<string> newRule;
            newRule.push_back(newNonTerminal);
            for (const auto& a : alpha) {
                newRule.push_back(a + newNonTerminal);
            }
            newRule.push_back(EPSILON);

            // 添加新规则
            grammar.push_back(newRule);
        }
    }
}

// 函数：检查字符串是否包含set中的任意子串
size_t containsAnySubstring(const std::string& target, const std::set<std::string>& substrings) {
    // 遍历set中的每个字符串
    size_t minPosition = target.length() + 1; 
    for (const auto& sub : substrings) {
        // 使用 find 函数查找子串
        size_t temp = target.find(sub);
        if(temp != string::npos && temp < minPosition) {
            minPosition = temp; // 更新最小位置
        }
    }
    if (minPosition == target.length() + 1) {
        return string::npos;  // 没有找到，返回空
    }
    return minPosition;
}

string containNonT(const std::string& target, const std::set<std::string>& substrings) {
    // 遍历set中的每个字符串 
    string result = "";
    for (const auto& sub : substrings) {
        // 使用 find 函数查找子串
        size_t temp = target.find(sub);
        if(temp != string::npos && temp == 0) {
            if(result.length() < sub.length()) {
                result = sub;
            }
        }
    }
    return result;
}

string containsSubFollow(const std::string& target, const std::set<std::string>& substrings) {
    // 遍历set中的每个字符串
    size_t minPosition = target.length() + 1; 
    string result = "";
    for (const auto& sub : substrings) {
        // 使用 find 函数查找子串
        size_t temp = target.find(sub);
        if(temp != string::npos && temp < minPosition) {
            minPosition = temp; // 更新最小位置
            result = sub;
        }
    }
    if (target.length() > minPosition + result.length()) {
        if (target[minPosition + result.length()] == '\''){
            result = result + "'";
        }
    }
    
    return result;
}

// Function to find the key of the map
std::string findKey(const std::map<std::string, std::string> &mymap, const std::string &key) {
    for (auto it = mymap.begin(); it != mymap.end(); ++it) {
        if (it->first == key) {
            return it->first;
        }
    }
    return "";
} 

void findSet(const std::set<std::array<string, 2>> &myset, const std::string & key){
    for (auto it : myset) {
        if(it[0] == key) {
            for (const auto& str : follow[it[0]]) {
                follow[it[1]].insert(str);
            }
            findSet(myset, it[1]);
        }
    }
}

// Function to find the FIRST set
void findFIRST(const std::vector<std::vector<string>> &grammar) {
    // 第一遍找 FIRST
    for (const auto& words : grammar) {
        for (int i = 1; i < words.size(); ++i) {
            size_t temp = containsAnySubstring(words[i], nonTset);
            if (temp == string::npos) {
                first[words[0]].insert(words[i]);
            } else if (temp != 0 && words[i].find(words[0] + "'") == string::npos) {
                first[words[0]].insert(words[i].substr(0, temp));
            }
        }  
    }

    std::map<string, string> equalNonT; 

    // 第二遍找 FIRST
    for (const auto& words : grammar) {
        for (int i = 1; i < words.size(); ++i) {
            string temp = containNonT(words[i], nonTset);
            if (temp != "") {
                equalNonT[temp] = words[0];
            }
        }  
    }

    for (auto it = equalNonT.begin(); it != equalNonT.end(); ++it) {
        for (const auto& str : first[it->first]) {
            if (str != EPSILON) {
                first[it->second].insert(str);
            }
        }
        string temp = it->second;
        while (findKey(equalNonT, temp) != "") {
            for (const auto& str : first[temp]) {
                if (str != EPSILON) {
                    first[it->second].insert(str);
                }
            }
            temp = equalNonT[temp];
        }
    }

    // TODO: 完成第三遍找 FIRST （本题用不到，暂时搁置）

    std::cout << "FIRST set:" << endl;
    for (auto it = first.begin(); it != first.end(); ++it) {
        std::cout << it->first << " -> ";
        for(auto str : it->second) {
            std::cout << str << " ";
        }
        std::cout << endl;
    }
}

void findFOLLOW(const std::vector<std::vector<string>> &grammar) {
    // 文法开始符号，置 $ 于 FOLLOW(S) 中
    follow[grammar[0][0]].insert("$");

    // 若有产生式 A -> alphaBbeta, 则把 FIRST(beta) 中的所有非 epsilon 加入 FOLLOW(B)中
    for (const auto& words : grammar) {
        for (int i = 1; i < words.size(); ++i) {
            string temp = containsSubFollow(words[i], nonTset);
            while (temp != "") {
                string subString = words[i].substr(words[i].find(temp) + temp.length());
                string temp2 = containsSubFollow(subString, nonTset);
                if(temp2 == ""){
                    follow[temp].insert(subString);
                } else {
                    size_t pos = subString.find(temp2);
                    if (pos == 0) {
                        for(const auto& str : first[temp2]) {
                            if (str != EPSILON){
                                follow[temp].insert(str);

                            }
                        }
                    } else {
                        follow[temp].insert(subString.substr(0, pos));
                    }
                }  
                temp = temp2;
            }
        }
    }

    std::set<std::array<string, 2>> equalNonT; 

    // 若有产生式 A -> alphaB, 或有产生式 A -> alphaBbeta, 但是 epsilon \in FIRST(beta), 则把 FOLLOW(A) 中所有元素加入 FOLLOW(B) 中
    for (const auto& words : grammar) {
        for (int i = 1; i < words.size(); ++i) {
            string temp = containsSubFollow(words[i], nonTset);
            while (temp != "") {
                string subString = words[i].substr(words[i].find(temp) + temp.length());
                if (subString == "" && words[0] != temp){
                    equalNonT.insert({words[0], temp});
                    break;
                }
                string temp2 = containsSubFollow(subString, nonTset);
                if(temp2 != "") {
                    size_t pos = subString.find(temp2);
                    if (pos == 0) {
                        for(const auto& str : first[temp2]) {
                            if (str == EPSILON){
                                equalNonT.insert({words[0], temp});
                            }
                        }
                    }
                }  
                temp = temp2;
            }
        }
    }

    for (auto it : equalNonT) {
        for (const auto& str : follow[it[0]]) {
            follow[it[1]].insert(str);
        }
        string temp = it[1];
        findSet(equalNonT, temp);
    }

    // print follow set
    std::cout << "FOLLOW set:" << endl;
    for (auto it = follow.begin(); it != follow.end(); ++it) {
        std::cout << it->first << " -> ";
        for(auto str : it->second) {
            std::cout << str << " ";
        }
        std::cout << endl;
    }
}

void createPAtable(const std::vector<std::vector<string>> &grammar) {
    for (const auto& words : grammar) {
        for (int i = 1; i < words.size(); ++i) {
            string pos = containsSubFollow(words[i], nonTset);
            if (pos == "") {
                if (words[i] != EPSILON) {
                    M[words[0]][words[i]] = {words[0],words[i]};
                } else {
                    for (const auto& str : follow[words[0]]) {
                        M[words[0]][str] = {words[0], words[i]};
                    }
                }
            } else if (words[i].find(pos) == 0) {
                for (const auto& str : first[pos]) {
                    if (str != EPSILON) {
                        M[words[0]][str] = {words[0], words[i]};
                    } else {
                        for (const auto& str2 : follow[words[0]]) {
                            M[words[0]][str2] = {words[0], words[i]};
                        }
                    }
                }
            } else {
                M[words[0]][words[i].substr(0, words[i].find(pos))] = {words[0], words[i]};
            }
        }
    }
}

int main() {
    int n;
    std::cout << "Please enter the number of grammatical lines: " << std::endl;
    cin >> n;
    cin.ignore();  // 忽略缓冲区中的换行符
    std::vector<string> s(n);
    
    std::cout << "Enter grammar (one per line, e.g., A->Aa|b): " << std::endl;
    for (int i = 0; i < n; ++i) {
        getline(cin, s[i]);
    }
    
    std::vector<vector<string>> initG;
    for (const auto& str : s) {
        std::vector<string> words = split(str);
        initG.push_back(words);
    }

    // Eliminate left recursion
    eliminateLeftRecursion(initG);

    // Print the resulting grammar
    std::cout << "\nThe grammar after eliminating left recursion:" << std::endl;
    for (const auto& words : initG) {
        std::cout << words[0] << " -> ";
        for (int i = 1; i < words.size(); ++i) {
            std::cout << words[i];
            if (i < words.size() - 1) {
                std::cout << " | ";
            }
        }
        std::cout << std::endl;
    }

    for (const auto& words : initG) {
        nonTset.insert(words[0]);
    }

    cout << endl;
    cout << "Non-terminal set: " << endl;
    for (auto word : nonTset) {
        cout << word << " ";
    }
    cout << endl;

    cout << endl;

    findFIRST(initG);
    findFOLLOW(initG);



    return 0;
}
