#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <ctime>
#include <winuser.h>
#include <winbase.h>
#include <windows.h>
#include "header.hpp"

int runtime(std::vector<std::vector<Token>> statements, bool c) {
    std::vector<std::string> names;
    std::vector<std::string> values;
    std::vector<std::string> immutables;
    for (auto outer = statements.begin(); outer < statements.end(); outer++) {
        std::vector<Token> stmt = *outer;
        int size = 0;
        for (auto inner = stmt.begin(); inner < stmt.end(); inner++) {
            size++;
            Token current = *inner;
            if (current.typ() == EQUAL) {
                Token previous = *std::prev(inner);
                if (previous.typ() != IDENTIFIER) {
                    error(previous, "Runtime Error: Inadequite identifier.");
                    return 1;
                } else {
                    //its good!
                    if (!in(previous.str(), immutables)) {
                        std::vector<Token>::const_iterator beg = std::next(inner);
                        std::vector<Token>::const_iterator end = stmt.begin() + (stmt.size()-1);
                        std::vector<Token> rest(beg, end);
                        bool err = false;
                        values.push_back(solve(rest, names, values, &err));
                        if (err) {
                            error(previous, "Run-time Error: Evaluation Error");
                            return 1;
                        }
                        names.push_back(previous.str());
                        if (size > 2) {
                            Token first = *std::prev(std::prev(inner));
                            if (first.typ() == IMMUTABLE) {
                                immutables.push_back(previous.str());
                            }
                        }
                    } else {
                        error(previous, "Run-time Error: Immutable variable cannot be mutated.");
                        return 1;
                    }
                }
            } else if (current.typ() == PRINT) {
                Token next = *std::next(inner);
                if (next.typ() != LEFT_PAREN) {
                    error(next, "Run-time Error: Expected a left parentheses token. None were provided.");
                    return 1;
                } else {
                    int n = std::next(inner) - stmt.begin();
                    auto nd = std::next(inner);
                    for (n; stmt[n].typ() != RIGHT_PAREN; n++) {
                        nd++;
                    }
                    std::vector<Token> segmented(std::next(inner), nd);
                    bool err = false;
                    std::string solved = solve(segmented, names, values, &err);
                    if (err) {
                        error(current, "Run-time Error: Evauation Error");
                        return 1;
                    }
                    if (solved.at(0) == '"') {
                        solved = solved.substr(1, solved.length()-2);
                    }
                    std::cout << solved;
                }
            } else if (current.typ() == RUN) {
                Token next = *std::next(inner);
                if (next.typ() != LEFT_PAREN) {
                    error(next, "Run-time Error: Expected a left parentheses token. None were provided.");
                    return 1;
                } else {
                    int n = std::next(inner) - stmt.begin();
                    auto nd = std::next(inner);
                    for (n; stmt[n].typ() != RIGHT_PAREN; n++) {
                        nd++;
                    }
                    std::vector<Token> segmented(std::next(inner), nd);
                    bool err = false;
                    std::string solved = solve(segmented, names, values, &err);
                    if (err) {
                        error(current, "Run-time Error: Evauation Error");
                        return 1;
                    }
                    if (solved.at(0) == '"') {
                        solved = solved.substr(1, solved.length()-2);
                    }
                    system(solved.c_str());
                }
            }
        }
    }
    return 0;
}