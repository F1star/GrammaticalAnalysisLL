#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

#define EPSILON "epsilon"

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

// Function to eliminate left recursion from grammar rules
void eliminateLeftRecursion(std::vector<std::vector<string>> &rules) {
    for (int i = 0; i < rules.size(); ++i) {
        std::vector<string> alpha; // 左递归部分
        std::vector<string> beta;  // 非左递归部分
        std::string nonTerminal = rules[i][0]; // 当前非终结符
        bool hasLeftRecursion = false;

        for (int j = 1; j < rules[i].size(); ++j) {
            if (rules[i][j].find(nonTerminal) == 0) {
                hasLeftRecursion = true;
                alpha.push_back(rules[i][j].substr(nonTerminal.length())); // 提取左递归部分
            } else {
                beta.push_back(rules[i][j]); // 非左递归部分
            }
        }

        if (hasLeftRecursion) {
            // 新的非终结符名称
            std::string newNonTerminal = nonTerminal + "'";

            // 生成非左递归规则：A -> B A'
            rules[i].clear();
            rules[i].push_back(nonTerminal);
            for (const auto& b : beta) {
                rules[i].push_back(b + newNonTerminal);
            }

            // 生成左递归规则：A' -> A' alpha | epsilon
            std::vector<string> newRule;
            newRule.push_back(newNonTerminal);
            for (const auto& a : alpha) {
                newRule.push_back(a + newNonTerminal);
            }
            newRule.push_back(EPSILON);

            // 添加新规则
            rules.push_back(newRule);
        }
    }
}

int main() {
    int n;
    std::cout << "Please enter the number of grammatical lines: " << std::endl;
    cin >> n;
    cin.ignore();  // 忽略缓冲区中的换行符
    std::vector<string> s(n);
    
    std::cout << "Enter grammar rules (one per line, e.g., A->Aa|b): " << std::endl;
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

    return 0;
}
