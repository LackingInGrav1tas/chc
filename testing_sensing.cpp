#include <iostream>
#include <vector>
#include <string>

int main() {
    std::vector<std::string> line = { "callee", "(", "a", ",", "b", "-", "c", ",", "d", ")", ";" };
    std::vector<std::vector<std::string>> final;
    std::vector<std::string> current;
    int nested = 0;
    auto start = line.begin();
    for (auto a = std::next(start); a < line.end(); a++) {
        if (*a == "," || (*a == ")" && nested == 1)) {
            final.push_back(current);
            current.clear();
        } else if (*a != "(" && nested > 0) {
            current.push_back(*a);
        }
        if (*a == "(") {
            nested++;
        } else if (*a == ")") {
            nested--;
        }
        if (*a == ")" && nested == 1) {
            break;
        }
    }
    for (int i = 0; i < final.size(); i++) {
        std::cout << i << ": ";
        for (int o = 0; o < final[i].size(); o++) {
            std::cout << final[i][o] << " ";
        }
        std::cout << std::endl;
    }
}