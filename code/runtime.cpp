#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <ctime>
#include <winuser.h>
#include <winbase.h>
#include <windows.h>
#include "header.hpp"

std::vector<Scope> scopes;
std::vector<std::string> scope_indices;

int runtime(std::vector<std::vector<Token>> statements, Scope &scope, bool *error_occurred, int limit, int precision, std::vector<Token> &return_variable) {
    std::vector<std::string> constants = { "@EOL", "@sec", "@min", "@hour", "@mday", "@yday", "@mon", "@year", "@clipboard", "@home", "@desktopW", "@desktopH", "@environment", "@IP", "@inf", "@write", "@append" };
    for (auto outer = statements.begin(); outer < statements.end(); std::advance(outer, 1)) {
        std::vector<Token> stmt = *outer;
        std::vector<Type> native_functions = { TOKEN_INPUT, ASSERT, WRITETO, LENGTH, HASH, RPRINT, FPRINT, RFPRINT, THROW, EVAL, RAND, AT, DISPOSE, SET_SCOPE, SAVE_SCOPE, STR, TOKEN_INT, IS_STRING, IS_NUMBER, IS_BOOL, SOLVE };
        int index = 0;
        for (auto token = stmt.end()-1; token >= stmt.begin(); token--) {
            if (in((*token).str(), scope.function_names)) {
                Token token_copy((*token).str(), (*token).lines(), (*token).col(), (*token).typ(), (*token).actual_line());
                bool eroc = false;
                auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                if (eroc)
                    return 1;
                std::pair<bool, int> found = findInV(scope.function_names, (*token).str());
                if (found.first) {
                    if (call_params.size() != scope.function_params[found.second].size()) {
                        std::string msg = "Run-time Error: Received " + std::to_string(call_params.size()) + " params but expected " + std::to_string(scope.function_params[found.second].size());
                        error(*token, msg);
                        return 1;
                    }
                    Scope nscope;
                    std::vector<std::vector<std::vector<Token>>> f;
                    std::vector<std::vector<std::string>> fp;
                    std::string rv;
                    if (in((*token).str(), scope.aware_functions)) {
                        nscope.names = scope.names;
                        nscope.values = scope.values;
                        nscope.immutables = scope.immutables;
                        nscope.function_names = scope.function_names;
                        nscope.aware_functions = scope.aware_functions;
                        nscope.function_bodies = scope.function_bodies;
                        nscope.function_params = scope.function_params;
                    }
                    for (int i = 0; i < call_params.size(); i++) {
                        nscope.names.push_back(scope.function_params[found.second][i]);
                        if (in(scope.function_params[found.second][i], scope.immutables))
                            nscope.immutables.push_back(scope.function_params[found.second][i]);
                        bool err = false;
                        nscope.values.push_back(solve(call_params[i], scope, &err, precision));
                        if (err) {
                            error(*token, "Run-time Error: Evaluation Error.");
                            return 1;
                        }
                    }
                    std::vector<Token> return_val;
                    bool er = false;
                    int result = runtime(scope.function_bodies[found.second], nscope, &er, limit, precision, return_val);
                    if (result == 1) {
                        return 1;
                    }
                    int ct = token - stmt.begin();
                    int nested = 0;
                    while (true) {
                        if (nested == 1 && stmt[ct].typ() == RIGHT_PAREN) {
                            stmt.erase(stmt.begin()+ct);
                            break;
                        }
                        if (stmt[ct].typ() == RIGHT_PAREN) {
                            nested--;
                        } else if (stmt[ct].typ() == LEFT_PAREN) {
                            nested++;
                        }
                        stmt.erase(stmt.begin()+ct);
                    }
                    if (return_val.empty())
                        stmt.insert(stmt.begin()+ct, Token("_void_func_holder", token_copy.lines(), token_copy.col(), token_copy.typ(), token_copy.actual_line()));
                    else {
                        for (auto ret = return_val.rbegin(); ret < return_val.rend(); ret++) {
                            stmt.insert(stmt.begin()+ct, *ret);
                        }
                    }
                }
            } else if (in((*token).typ(), native_functions)) {
                if ((*token).typ() == TOKEN_INPUT) {
                    bool eroc = false;
                    auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                    if (eroc)
                        return 1;
                    if (call_params.size() != 1) {
                        std::string msg = "Run-time Error: Received " + std::to_string(call_params.size()) + " params but expected 1.";
                        error(*token, msg);
                        return 1;
                    }
                    std::string solved;
                    for (auto cur = call_params[0].begin(); cur < call_params[0].end(); cur++) {
                        if ((*cur).syhtyp() == TERMINAL) {
                            std::string currentString = getVarVal(*cur, scope, error_occurred);
                            if (currentString.at(0) == '"') {
                                currentString = currentString.substr(1, currentString.length()-2);
                            }
                            solved += currentString;
                        }
                    }
                    std::cout << solved;
                    std::string raw_input;
                    std::getline(std::cin, raw_input);
                    int ct = token - stmt.begin();
                    int nested = 0;
                    while (true) {
                        if (nested == 1 && stmt[ct].typ() == RIGHT_PAREN) {
                            stmt.erase(stmt.begin()+ct);
                            break;
                        }
                        if (stmt[ct].typ() == RIGHT_PAREN) {
                            nested--;
                        } else if (stmt[ct].typ() == LEFT_PAREN) {
                            nested++;
                        }
                        stmt.erase(stmt.begin()+ct);
                    }
                    stmt.insert(stmt.begin()+ct, Token('"' + raw_input + '"', (*token).lines(), (*token).col(), STRING, (*token).actual_line()));
                } else if ((*token).typ() == WRITETO) {
                    bool eroc = false;
                    auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                    if (eroc)
                        return 1;
                    if (call_params.size() < 2) {
                        std::string msg = "Run-time Error: Received " + std::to_string(call_params.size()) + " params but expected 3.";
                        error(*token, msg);
                        return 1;
                    }
                    std::string filename;
                    for (auto cur = call_params[0].begin(); cur < call_params[0].end(); cur++) {
                        if ((*cur).syhtyp() == TERMINAL) {
                            std::string currentString = getVarVal(*cur, scope, error_occurred);
                            if (currentString.at(0) == '"') {
                                currentString = currentString.substr(1, currentString.length()-2);
                            }
                            filename += currentString;
                        }
                    }
                    std::string text;
                    for (auto cur = call_params[1].begin(); cur < call_params[1].end(); cur++) {
                        if ((*cur).syhtyp() == TERMINAL) {
                            std::string currentString = getVarVal(*cur, scope, error_occurred);
                            if (currentString.at(0) == '"') {
                                currentString = currentString.substr(1, currentString.length()-2);
                            }
                            text += currentString;
                        }
                    }
                    Type foundornot = TTRUE;
                    std::string fon = "true";
                    if (call_params.size() > 2) {
                        if (call_params[2].size() > 1) {
                            error(call_params[2][1], "Run-time Error: Too many.");
                            return 1;
                        }
                        bool e = false;
                        if (getVarVal(call_params[2][0], scope, &e) == "@append") {
                            std::ofstream output(filename, std::ios_base::app);
                            if (!output) {
                                foundornot = TFALSE;
                                fon = "false";
                            } else {
                                output << text;
                                output.close();
                            }
                        } else if (e) {
                            error(call_params[2][0], "Run-time Error: Evaluation Error.");
                            return 1;
                        } else {
                            std::ofstream output(filename);
                            if (!output) {
                                foundornot = TFALSE;
                                fon = "false";
                            } else {
                                output << text;
                                output.close();
                            }
                        }
                    } else {
                        std::ofstream output(filename);
                        if (!output) {
                            foundornot = TFALSE;
                            fon = "false";
                        } else {
                            output << text;
                            output.close();
                        }
                    }
                    int ct = token - stmt.begin();
                    int nested = 0;
                    while (true) {
                        if (nested == 1 && stmt[ct].typ() == RIGHT_PAREN) {
                            stmt.erase(stmt.begin()+ct);
                            break;
                        }
                        if (stmt[ct].typ() == RIGHT_PAREN) {
                            nested--;
                        } else if (stmt[ct].typ() == LEFT_PAREN) {
                            nested++;
                        }
                        stmt.erase(stmt.begin()+ct);
                    }
                    stmt.insert(stmt.begin()+ct, Token(fon, (*token).lines(), (*token).col(), foundornot, (*token).actual_line()));
                } else if ((*token).typ() == ASSERT) {
                    if ((*std::next(token)).typ() != LEFT_PAREN) {
                        error(*std::next(token), "Run-time Error: Incorrect formatting of assert call.");
                        return 1;
                    }
                    bool eroc = false;
                    auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                    if (eroc)
                        return 1;
                    for (auto param_segment = call_params.begin(); param_segment < call_params.end(); param_segment++) {
                        if (!boolsolve(*param_segment, scope, error_occurred)) {
                            error((*param_segment).front(), "Run-time Error: Assertion Failed.");
                            return 1;
                        }
                    }
                    int ct = token - stmt.begin();
                    int nested = 0;
                    while (true) {
                        if (nested == 1 && stmt[ct].typ() == RIGHT_PAREN) {
                            stmt.erase(stmt.begin()+ct);
                            break;
                        }
                        if (stmt[ct].typ() == RIGHT_PAREN) {
                            nested--;
                        } else if (stmt[ct].typ() == LEFT_PAREN) {
                            nested++;
                        }
                        stmt.erase(stmt.begin()+ct);
                    }
                } else if ((*token).typ() == LENGTH) {
                    bool e = false;
                    if ((*std::next(token)).typ() != LEFT_PAREN) {
                        error(*std::next(token), "Run-time Error: Expected a left bracket token.");
                        return 1;
                    }
                    if (e) {
                        error(*std::prev(std::prev(token)), "Run-time Error: ");
                        return 1;
                    }
                    bool eroc = false;
                    auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                    if (eroc)
                        return 1;
                    if (call_params.size() != 1) {
                        error(*token, "Run-time Error: Expected 1 parameter. Received " + std::to_string(call_params.size()) + ".");
                        return 1;
                    }
                    int ct = token - stmt.begin();
                    int nested = 0;
                    while (true) {
                        if (nested == 1 && stmt[ct].typ() == RIGHT_PAREN) {
                            stmt.erase(stmt.begin()+ct);
                            break;
                        }
                        if (stmt[ct].typ() == RIGHT_PAREN) {
                            nested--;
                        } else if (stmt[ct].typ() == LEFT_PAREN) {
                            nested++;
                        }
                        stmt.erase(stmt.begin()+ct);
                    }
                    stmt.insert(stmt.begin()+ct, Token(std::to_string(solve(call_params[0], scope, &e, precision).length() - 2), (*token).lines(), (*token).col(), NUMBER, (*token).actual_line()));
                } else if ((*token).typ() == HASH) {
                    if ((*std::next(token)).typ() != LEFT_PAREN) {
                        error(*std::next(token), "Run-time Error: Expected a left bracket token.");
                        return 1;
                    }
                    bool eroc = false;
                    auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                    if (eroc)
                        return 1;
                    if (call_params.size() != 1) {
                        error(*token, "Run-time Error: Expected 1 parameter. Received " + std::to_string(call_params.size()) + ".");
                        return 1;
                    }
                    std::string solved;
                    for (auto cur = call_params[0].begin(); cur < call_params[0].end(); cur++) {
                        if ((*cur).syhtyp() == TERMINAL) {
                            std::string currentString = getVarVal(*cur, scope, error_occurred);
                            if (currentString.at(0) == '"') {
                                currentString = currentString.substr(1, currentString.length()-2);
                            }
                            solved += currentString;
                        }
                    }
                    int ct = token - stmt.begin();
                    int nested = 0;
                    while (true) {
                        if (nested == 1 && stmt[ct].typ() == RIGHT_PAREN) {
                            stmt.erase(stmt.begin()+ct);
                            break;
                        }
                        if (stmt[ct].typ() == RIGHT_PAREN) {
                            nested--;
                        } else if (stmt[ct].typ() == LEFT_PAREN) {
                            nested++;
                        }
                        stmt.erase(stmt.begin()+ct);
                    }
                    stmt.insert(stmt.begin()+ct, Token('"' + hash(solved) + '"', (*token).lines(), (*token).col(), STRING, (*token).actual_line()));
                } else if ((*token).typ() == RPRINT) {
                    bool eroc = false;
                    auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                    if (eroc)
                        return 1;
                    if (call_params.size() != 1) {
                        std::string msg = "Run-time Error: Received " + std::to_string(call_params.size()) + " params but expected 1.";
                        error(*token, msg);
                        return 1;
                    }
                    std::string solved;
                    for (auto cur = call_params[0].begin(); cur < call_params[0].end(); cur++) {
                        if ((*cur).syhtyp() == TERMINAL) {
                            std::string currentString = getVarVal(*cur, scope, error_occurred);
                            solved += currentString;
                        }
                    }
                    std::cout << solved;
                    int ct = token - stmt.begin();
                    int nested = 0;
                    while (true) {
                        if (nested == 1 && stmt[ct].typ() == RIGHT_PAREN) {
                            stmt.erase(stmt.begin()+ct);
                            break;
                        }
                        if (stmt[ct].typ() == RIGHT_PAREN) {
                            nested--;
                        } else if (stmt[ct].typ() == LEFT_PAREN) {
                            nested++;
                        }
                        stmt.erase(stmt.begin()+ct);
                    }
                    stmt.insert(stmt.begin()+ct, Token("_void_func_holder", (*token).lines(), (*token).col(), _VOID_FUNC_HOLDER, (*token).actual_line()));
                } else if ((*token).typ() == FPRINT) {
                    bool eroc = false;
                    auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                    if (eroc)
                        return 1;
                    if (call_params.size() != 1) {
                        std::string msg = "Run-time Error: Received " + std::to_string(call_params.size()) + " params but expected 1.";
                        error(*token, msg);
                        return 1;
                    }
                    std::string solved;
                    for (auto cur = call_params[0].begin(); cur < call_params[0].end(); cur++) {
                        if ((*cur).syhtyp() == TERMINAL) {
                            std::string currentString = getVarVal(*cur, scope, error_occurred);
                            if (currentString.at(0) == '"') {
                                currentString = currentString.substr(1, currentString.length()-2);
                            }
                            solved += currentString;
                        }
                    }
                    std::cout << solved << std::endl;
                    int ct = token - stmt.begin();
                    int nested = 0;
                    while (true) {
                        if (nested == 1 && stmt[ct].typ() == RIGHT_PAREN) {
                            stmt.erase(stmt.begin()+ct);
                            break;
                        }
                        if (stmt[ct].typ() == RIGHT_PAREN) {
                            nested--;
                        } else if (stmt[ct].typ() == LEFT_PAREN) {
                            nested++;
                        }
                        stmt.erase(stmt.begin()+ct);
                    }
                    stmt.insert(stmt.begin()+ct, Token("_void_func_holder", (*token).lines(), (*token).col(), _VOID_FUNC_HOLDER, (*token).actual_line()));
                } else if ((*token).typ() == RFPRINT) {
                    bool eroc = false;
                    auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                    if (eroc)
                        return 1;
                    if (call_params.size() != 1) {
                        std::string msg = "Run-time Error: Received " + std::to_string(call_params.size()) + " params but expected 1.";
                        error(*token, msg);
                        return 1;
                    }
                    std::string solved;
                    for (auto cur = call_params[0].begin(); cur < call_params[0].end(); cur++) {
                        if ((*cur).syhtyp() == TERMINAL) {
                            std::string currentString = getVarVal(*cur, scope, error_occurred);
                            solved += currentString;
                        }
                    }
                    std::cout << solved << std::endl;
                    int ct = token - stmt.begin();
                    int nested = 0;
                    while (true) {
                        if (nested == 1 && stmt[ct].typ() == RIGHT_PAREN) {
                            stmt.erase(stmt.begin()+ct);
                            break;
                        }
                        if (stmt[ct].typ() == RIGHT_PAREN) {
                            nested--;
                        } else if (stmt[ct].typ() == LEFT_PAREN) {
                            nested++;
                        }
                        stmt.erase(stmt.begin()+ct);
                    }
                    stmt.insert(stmt.begin()+ct, Token("_void_func_holder", (*token).lines(), (*token).col(), _VOID_FUNC_HOLDER, (*token).actual_line()));
                } else if ((*token).typ() == EVAL) {
                    if ((*std::next(token)).typ() != LEFT_PAREN) {
                        error(*std::next(token), "Run-time Error: Incorrect formatting of function call.");
                        return 1;
                    }
                    bool eroc = false;
                    auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                    if (eroc)
                        return 1;
                    if (call_params.size() != 1) {
                        error(*token, "Run-time Error: Expected 1 parameter. Received " + std::to_string(call_params.size()) + ".");
                        return 1;
                    }
                    Type evaluated = TFALSE;
                    std::string bol = "false";
                    if (boolsolve(call_params[0], scope, error_occurred)) {
                        evaluated = TTRUE;
                        bol = "true";
                    }
                    int ct = token - stmt.begin();
                    int nested = 0;
                    while (true) {
                        if (nested == 1 && stmt[ct].typ() == RIGHT_PAREN) {
                            stmt.erase(stmt.begin()+ct);
                            break;
                        }
                        if (stmt[ct].typ() == RIGHT_PAREN) {
                            nested--;
                        } else if (stmt[ct].typ() == LEFT_PAREN) {
                            nested++;
                        }
                        stmt.erase(stmt.begin()+ct);
                    }
                    stmt.insert(stmt.begin()+ct, Token(bol, (*token).lines(), (*token).col(), evaluated, (*token).actual_line()));
                } else if ((*token).typ() == SOLVE) {
                    if ((*std::next(token)).typ() != LEFT_PAREN) {
                        error(*std::next(token), "Run-time Error: Incorrect formatting of function call.");
                        return 1;
                    }
                    bool eroc = false;
                    auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                    if (eroc)
                        return 1;
                    if (call_params.size() != 1) {
                        error(*token, "Run-time Error: Expected 1 parameter. Received " + std::to_string(call_params.size()) + ".");
                        return 1;
                    }
                    Type evaluated = NUMBER;
                    std::string solved = solve(call_params[0], scope, error_occurred, precision);
                    if (solved.at(0) == '"') {
                        evaluated = STRING;
                    }
                    int ct = token - stmt.begin();
                    int nested = 0;
                    while (true) {
                        if (nested == 1 && stmt[ct].typ() == RIGHT_PAREN) {
                            stmt.erase(stmt.begin()+ct);
                            break;
                        }
                        if (stmt[ct].typ() == RIGHT_PAREN) {
                            nested--;
                        } else if (stmt[ct].typ() == LEFT_PAREN) {
                            nested++;
                        }
                        stmt.erase(stmt.begin()+ct);
                    }
                    stmt.insert(stmt.begin()+ct, Token(solved, (*token).lines(), (*token).col(), evaluated, (*token).actual_line()));
                } else if ((*token).typ() == THROW) {
                    bool eroc = false;
                    auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                    if (eroc)
                        return 1;
                    if (call_params.size() != 1) {
                        std::string msg = "Run-time Error: Received " + std::to_string(call_params.size()) + " params but expected 1.";
                        error(*token, msg);
                        return 1;
                    }
                    std::string solved;
                    for (auto cur = call_params[0].begin(); cur < call_params[0].end(); cur++) {
                        if ((*cur).syhtyp() == TERMINAL) {
                            std::string currentString = getVarVal(*cur, scope, error_occurred);
                            if (currentString.at(0) == '"') {
                                currentString = currentString.substr(1, currentString.length()-2);
                            }
                            solved += currentString;
                        }
                    }
                    error(*token, solved);
                    return 1;
                } else if ((*token).typ() == RAND) {
                    if ((*std::next(token)).typ() != LEFT_PAREN) {
                        error(*std::next(token), "Run-time Error: Incorrect formatting of function call.");
                        return 1;
                    }
                    bool eroc = false;
                    auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                    if (eroc)
                        return 1;
                    if (call_params.size() != 1) {
                        error(*token, "Run-time Error: Expected 1 parameter. Received " + std::to_string(call_params.size()) + ".");
                        return 1;
                    }
                    std::string solved = solve(call_params[0], scope, error_occurred, precision);
                    std::string ret;
                    try {
                        srand(time(NULL));
                        ret = std::to_string(rand() % std::stoi(solved));
                    } catch (...) {
                        error(*token, "Run-time Error: rand() expects an intiger value as a parameter.");
                        return 1;
                    }
                    int ct = token - stmt.begin();
                    int nested = 0;
                    while (true) {
                        if (nested == 1 && stmt[ct].typ() == RIGHT_PAREN) {
                            stmt.erase(stmt.begin()+ct);
                            break;
                        }
                        if (stmt[ct].typ() == RIGHT_PAREN) {
                            nested--;
                        } else if (stmt[ct].typ() == LEFT_PAREN) {
                            nested++;
                        }
                        stmt.erase(stmt.begin()+ct);
                    }
                    stmt.insert(stmt.begin()+ct, Token(ret, (*token).lines(), (*token).col(), NUMBER, (*token).actual_line()));
                } else if ((*token).typ() == AT) {
                    if ((*std::next(token)).typ() != LEFT_PAREN) {
                        error(*std::next(token), "Run-time Error: Incorrect formatting of function call.");
                        return 1;
                    }
                    bool e = false;
                    bool eroc = false;
                    auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                    if (eroc)
                        return 1;
                    if (call_params.size() != 2) {
                        error(*token, "Run-time Error: Expected 2 parameters. Received " + std::to_string(call_params.size()) + ".");
                        return 1;
                    }
                    std::string value = solve(call_params[0], scope, &e, precision).substr(1, solve(call_params[1], scope, &e, precision).length()-2);
                    std::string solved = solve(call_params[1], scope, &e, precision);
                    if (e) {
                        error(call_params[0][0], "Run-time Error: Solving error.");
                        return 1;
                    }
                    int pos;
                    try {pos = std::stoi(solved);}
                    catch(...) {
                        error(*token, "Run-time Error: Expected an intiger.");
                        return 1;
                    }
                    if (pos >= value.length()) {
                        error(*token, "Run-time Error: " + std::to_string(pos) + " out of range.");
                        return 1;
                    }
                    int ct = token - stmt.begin();
                    int nested = 0;
                    while (true) {
                        if (nested == 1 && stmt[ct].typ() == RIGHT_PAREN) {
                            stmt.erase(stmt.begin()+ct);
                            break;
                        }
                        if (stmt[ct].typ() == RIGHT_PAREN) {
                            nested--;
                        } else if (stmt[ct].typ() == LEFT_PAREN) {
                            nested++;
                        }
                        stmt.erase(stmt.begin()+ct);
                    }
                    stmt.insert(stmt.begin()+ct, Token(R"(")" + getString(value.at(pos)) + R"(")", (*token).lines(), (*token).col(), STRING, (*token).actual_line()));
                } else if ((*token).typ() == DISPOSE) {
                    bool e = false;
                    if ((*std::next(token)).typ() != LEFT_PAREN) {
                        error(*std::next(token), "Run-time Error: Incorrect formatting of call.");
                        return 1;
                    }
                    if (e) {
                        error(*token, "Run-time Error: Eval error.");
                    }
                    bool eroc = false;
                    auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                    if (eroc) {
                        return 1;
                    }
                    if (call_params.size() != 1) {
                        error(*token, "Run-time Error: Expected 1 parameter. Received " + std::to_string(call_params.size()) + ".");
                        return 1;
                    } else if (call_params[0].size() != 1 || call_params[0][0].typ() != IDENTIFIER) {
                        error(*token, "Run-time Error: Expected a single identifier as a parameter. Received " + std::to_string(call_params.size()) + ".");
                        return 1;
                    }
                    for (int i = 0; i < scope.names.size(); i++) {
                        if (scope.names[i] == call_params[0][0].str()) {
                            scope.names.erase(scope.names.begin()+i);
                            scope.values.erase(scope.values.begin()+i);
                        }
                    }
                    for (int i = 0; i < scope.immutables.size(); i++) {
                        if (scope.immutables[i] == call_params[0][0].str()) {
                            scope.immutables.erase(scope.immutables.begin()+i);
                        }
                    }
                    for (int i = 0; i < scope.function_names.size(); i++) {
                        if (scope.function_names[i] == call_params[0][0].str()) {
                            scope.function_names.erase(scope.function_names.begin()+i);
                            scope.function_params.erase(scope.function_params.begin()+i);
                            scope.function_bodies.erase(scope.function_bodies.begin()+i);
                        }
                    }
                    for (int i = 0; i < scope.aware_functions.size(); i++) {
                        if (scope.aware_functions[i] == call_params[0][0].str()) {
                            scope.aware_functions.erase(scope.aware_functions.begin()+i);
                        }
                    }
                    int ct = token - stmt.begin();
                    int nested = 0;
                    while (true) {
                        if (nested == 1 && stmt[ct].typ() == RIGHT_PAREN) {
                            stmt.erase(stmt.begin()+ct);
                            break;
                        }
                        if (stmt[ct].typ() == RIGHT_PAREN) {
                            nested--;
                        } else if (stmt[ct].typ() == LEFT_PAREN) {
                            nested++;
                        }
                        stmt.erase(stmt.begin()+ct);
                    }
                    Token insert_tok("_void_func_holder", (*token).lines(), (*token).col(), _VOID_FUNC_HOLDER, (*token).actual_line());
                    stmt.insert(stmt.begin()+ct, insert_tok);
                } else if ((*token).typ() == SAVE_SCOPE) {
                    if ((*std::next(token)).typ() != LEFT_PAREN) {
                        error(*std::next(token), "Run-time Error: Incorrect formatting of assert call.");
                        return 1;
                    }
                    bool eroc = false;
                    auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                    if (eroc)
                        return 1;
                    if (call_params.size() != 1) {
                        error(*token, "Run-time Error: Expected 1 parameter, received " + std::to_string(call_params.size()) + ".");
                        return 1;
                    } else if (call_params[0].size() != 1) {
                        error(*token, "Run-time Error: Expected 1 parameter, received " + std::to_string(call_params.size()) + ".");
                        return 1;
                    }
                    bool gvve = false;
                    scopes.push_back(scope);
                    scope_indices.push_back(getVarVal(*std::next(std::next(token)), scope, &gvve));
                    if (gvve) {
                        error(*std::next(std::next(token)), "Run-time Error: Eval error.");
                        return 1;
                    }
                    int ct = token - stmt.begin();
                    int nested = 0;
                    while (true) {
                        if (nested == 1 && stmt[ct].typ() == RIGHT_PAREN) {
                            stmt.erase(stmt.begin()+ct);
                            break;
                        }
                        if (stmt[ct].typ() == RIGHT_PAREN) {
                            nested--;
                        } else if (stmt[ct].typ() == LEFT_PAREN) {
                            nested++;
                        }
                        stmt.erase(stmt.begin()+ct);
                    }
                } else if ((*token).typ() == SET_SCOPE) {
                    if ((*std::next(token)).typ() != LEFT_PAREN) {
                        error(*std::next(token), "Run-time Error: Incorrect formatting of assert call.");
                        return 1;
                    }
                    bool eroc = false;
                    auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                    if (eroc)
                        return 1;
                    if (call_params.size() != 1) {
                        error(*token, "Run-time Error: Expected 1 parameter, received " + std::to_string(call_params.size()) + ".");
                        return 1;
                    }
                    bool gvve = false;
                    auto found = findInV(scope_indices, getVarVal(*std::next(std::next(token)), scope, &gvve));
                    if (gvve) {
                        error(*std::next(std::next(token)), "Run-time Error: Eval error.");
                        return 1;
                    }
                    if (found.first) {
                        scope = scopes[found.second];
                    }
                    int ct = token - stmt.begin();
                    int nested = 0;
                    while (true) {
                        if (nested == 1 && stmt[ct].typ() == RIGHT_PAREN) {
                            stmt.erase(stmt.begin()+ct);
                            break;
                        }
                        if (stmt[ct].typ() == RIGHT_PAREN) {
                            nested--;
                        } else if (stmt[ct].typ() == LEFT_PAREN) {
                            nested++;
                        }
                        stmt.erase(stmt.begin()+ct);
                    }
                } else if ((*token).typ() == STR) {
                    bool e = false;
                    if ((*std::next(token)).typ() != LEFT_PAREN) {
                        error(*std::next(token), "Run-time Error: Expected a left bracket token.");
                        return 1;
                    }
                    if (e) {
                        error(*std::prev(std::prev(token)), "Run-time Error: ");
                        return 1;
                    }
                    bool eroc = false;
                    auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                    if (eroc)
                        return 1;
                    if (call_params.size() != 1) {
                        error(*token, "Run-time Error: Expected 1 parameter. Received " + std::to_string(call_params.size()) + ".");
                        return 1;
                    }
                    std::string solved = solve(call_params[0], scope, &e, precision);
                    if (solved.at(0) == '"') {
                        error(*token, "Run-time Error: Expected non-string as an input.");
                        return 1;
                    }
                    if (e) {
                        error(*token, "Run-time Error: Solving error.");
                        return 1;
                    }
                    int ct = token - stmt.begin();
                    int nested = 0;
                    while (true) {
                        if (nested == 1 && stmt[ct].typ() == RIGHT_PAREN) {
                            stmt.erase(stmt.begin()+ct);
                            break;
                        }
                        if (stmt[ct].typ() == RIGHT_PAREN) {
                            nested--;
                        } else if (stmt[ct].typ() == LEFT_PAREN) {
                            nested++;
                        }
                        stmt.erase(stmt.begin()+ct);
                    }
                    stmt.insert(stmt.begin()+ct, Token(R"(")" + solved + R"(")", (*token).lines(), (*token).col(), STRING, (*token).actual_line() ));
                } else if ((*token).typ() == TOKEN_INT) {
                    bool e = false;
                    if ((*std::next(token)).typ() != LEFT_PAREN) {
                        error(*std::next(token), "Run-time Error: Expected a left bracket token.");
                        return 1;
                    }
                    if (e) {
                        error(*std::prev(std::prev(token)), "Run-time Error: ");
                        return 1;
                    }
                    bool eroc = false;
                    auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                    if (eroc)
                        return 1;
                    if (call_params.size() != 1) {
                        error(*token, "Run-time Error: Expected 1 parameter. Received " + std::to_string(call_params.size()) + ".");
                        return 1;
                    }
                    std::string solved = solve(call_params[0], scope, &e, precision);
                    if (solved.at(0) != '"' && solved != "true" && solved == "false") {
                        error(*token, "Run-time Error: Expected non-number as an input.");
                        return 1;
                    } else if (solved == "true") {
                        solved = "1";
                    } else if (solved == "false") {
                        solved = "0";
                    } else {
                        solved = solved.substr(1, solved.length()-2);
                    }
                    if (e) {
                        error(*token, "Run-time Error: Solving error.");
                        return 1;
                    }
                    int ct = token - stmt.begin();
                    int nested = 0;
                    while (true) {
                        if (nested == 1 && stmt[ct].typ() == RIGHT_PAREN) {
                            stmt.erase(stmt.begin()+ct);
                            break;
                        }
                        if (stmt[ct].typ() == RIGHT_PAREN) {
                            nested--;
                        } else if (stmt[ct].typ() == LEFT_PAREN) {
                            nested++;
                        }
                        stmt.erase(stmt.begin()+ct);
                    }
                    stmt.insert(stmt.begin()+ct, Token(solved, (*token).lines(), (*token).col(), NUMBER, (*token).actual_line() ));
                } else if ((*token).typ() == IS_STRING) {
                    bool eroc = false;
                    auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                    if (eroc)
                        return 1;
                    if (call_params.size() != 1) {
                        std::string msg = "Run-time Error: Received " + std::to_string(call_params.size()) + " params but expected 1.";
                        error(*token, msg);
                        return 1;
                    } else if (call_params[0].size() != 1) {
                        std::string msg = "Run-time Error: Received " + std::to_string(call_params.size()) + " params but expected 1.";
                        error(*token, msg);
                        return 1;
                    }
                    std::string fon = "false";
                    Type f_on = TFALSE;
                    if (getVarVal(call_params[0][0], scope, &eroc).at(0) == '"') {
                        fon = "true";
                        f_on = TTRUE;
                    }
                    int ct = token - stmt.begin();
                    int nested = 0;
                    while (true) {
                        if (nested == 1 && stmt[ct].typ() == RIGHT_PAREN) {
                            stmt.erase(stmt.begin()+ct);
                            break;
                        }
                        if (stmt[ct].typ() == RIGHT_PAREN) {
                            nested--;
                        } else if (stmt[ct].typ() == LEFT_PAREN) {
                            nested++;
                        }
                        stmt.erase(stmt.begin()+ct);
                    }
                    stmt.insert(stmt.begin()+ct, Token(fon, (*token).lines(), (*token).col(), f_on, (*token).actual_line()));
                } else if ((*token).typ() == IS_NUMBER) {
                    bool eroc = false;
                    auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                    if (eroc)
                        return 1;
                    if (call_params.size() != 1) {
                        std::string msg = "Run-time Error: Received " + std::to_string(call_params.size()) + " params but expected 1.";
                        error(*token, msg);
                        return 1;
                    } else if (call_params[0].size() != 1) {
                        std::string msg = "Run-time Error: Received " + std::to_string(call_params.size()) + " params but expected 1.";
                        error(*token, msg);
                        return 1;
                    }
                    std::string fon = "true";
                    Type f_on = TTRUE;
                    if (getVarVal(call_params[0][0], scope, &eroc).at(0) == '"' || getVarVal(call_params[0][0], scope, &eroc) == "true" || getVarVal(call_params[0][0], scope, &eroc) == "false") {//here
                        fon = "false";
                        f_on = TFALSE;
                    }
                    int ct = token - stmt.begin();
                    int nested = 0;
                    while (true) {
                        if (nested == 1 && stmt[ct].typ() == RIGHT_PAREN) {
                            stmt.erase(stmt.begin()+ct);
                            break;
                        }
                        if (stmt[ct].typ() == RIGHT_PAREN) {
                            nested--;
                        } else if (stmt[ct].typ() == LEFT_PAREN) {
                            nested++;
                        }
                        stmt.erase(stmt.begin()+ct);
                    }
                    stmt.insert(stmt.begin()+ct, Token(fon, (*token).lines(), (*token).col(), f_on, (*token).actual_line()));
                } else if ((*token).typ() == IS_BOOL) {
                    bool eroc = false;
                    auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                    if (eroc)
                        return 1;
                    if (call_params.size() != 1) {
                        std::string msg = "Run-time Error: Received " + std::to_string(call_params.size()) + " params but expected 1.";
                        error(*token, msg);
                        return 1;
                    } else if (call_params[0].size() != 1) {
                        std::string msg = "Run-time Error: Received " + std::to_string(call_params.size()) + " params but expected 1.";
                        error(*token, msg);
                        return 1;
                    }
                    std::string fon = "false";
                    Type f_on = TFALSE;
                    if (getVarVal(call_params[0][0], scope, &eroc) == "true" || getVarVal(call_params[0][0], scope, &eroc) == "false") {//here
                        fon = "true";
                        f_on = TTRUE;
                    }
                    int ct = token - stmt.begin();
                    int nested = 0;
                    while (true) {
                        if (nested == 1 && stmt[ct].typ() == RIGHT_PAREN) {
                            stmt.erase(stmt.begin()+ct);
                            break;
                        }
                        if (stmt[ct].typ() == RIGHT_PAREN) {
                            nested--;
                        } else if (stmt[ct].typ() == LEFT_PAREN) {
                            nested++;
                        }
                        stmt.erase(stmt.begin()+ct);
                    }
                    stmt.insert(stmt.begin()+ct, Token(fon, (*token).lines(), (*token).col(), f_on, (*token).actual_line()));
                }
                token++;
            }
            index++;
        }
        int size = 0;
        for (auto inner = stmt.begin(); inner < stmt.end(); inner++) {
            size++;
            Token current = *inner;
            if (current.typ() == EQUAL) {//setting variable values
                Token previous = *std::prev(inner);
                if (previous.typ() != IDENTIFIER) {
                    error(previous, "Runtime Error: Inadequite identifier.");
                    return 1;
                } else {
                    for (auto tok = inner; tok < stmt.end(); tok++) {
                        if ((*tok).typ() == IDENTIFIER && findInV(scope.names, (*tok).str()).first == false && !in((*tok).str(), constants)) {
                            error(*tok, "Run-time Error: Undefined variable.");
                            return 1;
                        }
                    }
                    //its good!
                    if (!in(previous.str(), scope.immutables)) {
                        std::vector<Token>::const_iterator beg = std::next(inner);
                        std::vector<Token>::const_iterator end = stmt.begin() + (stmt.size()-1);
                        std::vector<Token> rest(beg, end);
                        bool err = false;
                        scope.values.push_back(solve(rest, scope, &err, precision));
                        if (err) {
                            error(previous, "Run-time Error: Evaluation Error");
                            return 1;
                        }
                        scope.names.push_back(previous.str());
                        if (size > 2) {
                            Token first = *std::prev(std::prev(inner));
                            if (first.typ() == IMMUTABLE) {
                                scope.immutables.push_back(previous.str());
                            }
                        }
                    } else {
                        error(previous, "Run-time Error: Immutable variable cannot be mutated.");
                        return 1;
                    }
                }
                break;
            } else if (current.typ() == MINUS_MINUS) {
                if ((*std::next(inner)).typ() != SEMICOLON) {
                    error((*std::next(inner)), "Run-time Error: Expected a semicolon.");
                    return 1;
                } else if ((*std::prev(inner)).typ() != IDENTIFIER) {
                    error((*std::next(inner)), "Run-time Error: Expected an identifier.");
                    return 1;
                }
                double pos;
                bool err = false;
                try {pos = std::stod(getVarVal((*std::prev(inner)), scope, &err)) - 1;}
                catch (...) {
                    error((*std::prev(inner)), "Run-time Error: Expected a number identifier.");
                    return 1;
                }
                scope.names.push_back((*std::prev(inner)).str());
                std::string preshortened = to_string_with_precision(pos, precision);
                shorten(preshortened);
                scope.values.push_back(preshortened);
                break;
            } else if (current.typ() == PLUS_PLUS) {
                if ((*std::next(inner)).typ() != SEMICOLON) {
                    error((*std::next(inner)), "Run-time Error: Expected a semicolon.");
                    return 1;
                } else if ((*std::prev(inner)).typ() != IDENTIFIER) {
                    error((*std::next(inner)), "Run-time Error: Expected an identifier.");
                    return 1;
                }
                double pos;
                bool err = false;
                try {pos = std::stod(getVarVal((*std::prev(inner)), scope, &err)) + 1;}
                catch (...) {
                    error((*std::prev(inner)), "Run-time Error: Expected a number identifier.");
                    return 1;
                }
                scope.names.push_back((*std::prev(inner)).str());
                std::string preshortened = to_string_with_precision(pos, precision);
                shorten(preshortened);
                scope.values.push_back(preshortened);
                break;
            } else if (current.typ() == PRINT) {
                Token next = *std::next(inner);
                if (next.typ() != LEFT_PAREN) {
                    error(next, "Run-time Error: Expected a left parentheses token. None were provided.");
                    return 1;
                } else {
                    int n = std::next(inner) - stmt.begin();
                    auto nd = std::next(inner);
                    int nested = 0;
                    while (true) {
                        if ((*nd).typ() == IDENTIFIER && findInV(scope.names, (*nd).str()).first == false && !in((*nd).str(), constants)) {
                            error(*nd, "Run-time Error: Undefined variable.");
                            return 1;
                        }
                        nd++;
                        n++;
                        if (stmt[n].typ() == RIGHT_PAREN && nested == 0) {
                            break;
                        } else {
                            if (stmt[n].typ() == LEFT_PAREN) {
                                nested++;
                            } else if (stmt[n].typ() == RIGHT_PAREN) {
                                nested--;
                            } else if (nd == stmt.end()) {
                                error(stmt.back(), "Run-time Error: Unclosed parentheses");
                                return 1;
                            }
                        }
                    }
                    n++;
                    if (stmt[n].typ() != SEMICOLON) {
                        error(current, "Warning: It's prudent to postfix the statement with a semicolon.");
                    };
                    std::vector<Token> segmented(std::next(inner), nd);
                    std::string solved;
                    for (auto ato = segmented.begin(); ato < segmented.end(); ato++) {
                        if ((*ato).syhtyp() == TERMINAL) {
                            std::string currentString = getVarVal(*ato, scope, error_occurred);
                            if (!currentString.empty()) {
                                if (currentString.at(0) == '"') {
                                    currentString = currentString.substr(1, currentString.length()-2);
                                }
                                solved += currentString;
                            }
                        }
                    }
                    std::cout << solved;
                }
                break;
            } else if (current.typ() == RUN) {
                Token next = *std::next(inner);
                if (next.typ() != LEFT_PAREN) {
                    error(next, "Run-time Error: Expected a left parentheses token. None were provided.");
                    return 1;
                } else {
                    int n = std::next(inner) - stmt.begin();
                    auto nd = std::next(inner);
                    int nested = 0;
                    while (true) {
                        if ((*nd).typ() == IDENTIFIER && findInV(scope.names, (*nd).str()).first == false && !in((*nd).str(), constants)) {
                            error(*nd, "Run-time Error: Undefined variable.");
                            return 1;
                        }
                        nd++;
                        n++;
                        if (stmt[n].typ() == RIGHT_PAREN && nested == 0) {
                            break;
                        } else {
                            if (stmt[n].typ() == LEFT_PAREN) {
                                nested++;
                            } else if (stmt[n].typ() == RIGHT_PAREN) {
                                nested--;
                            }
                        }
                    }
                    n++;
                    if (stmt[n].typ() != SEMICOLON) {
                        error(current, "Warning: It's prudent to postfix the statement with a semicolon.");
                    };
                    std::vector<Token> segmented(std::next(inner), nd);
                    bool err = false;
                    std::string solved = solve(segmented, scope, &err, precision);
                    if (err) {
                        error(current, "Run-time Error: Evauation Error");
                        return 1;
                    }
                    if (solved.at(0) == '"') {
                        solved = solved.substr(1, solved.length()-2);
                    }
                    system(solved.c_str());
                }
                break;
            } else if (current.typ() == DO) {
                Token next = *std::next(inner);
                if (next.typ() != LEFT_BRACE) {
                    error(next, "Run-time Error: Expected a left bracket token. None were provided.");
                    return 1;
                }
                std::vector<std::vector<Token>> whilecontents;
                int nested = 0;
                Token fis;
                while (true) {
                    outer++;
                    std::vector<Token> line = *outer;
                    if (line.front().typ() == _EOF) {
                        error(current, "Run-time Error: Unending do...while loop.");
                        return 1;
                    }
                    fis = line[0];
                    whilecontents.push_back(line);
                    if (fis.typ() == RIGHT_BRACE && nested == 0) {
                        break;
                    } else {
                        if (fis.typ() == RIGHT_BRACE) {
                            nested--;
                        } else if (line.back().typ() == LEFT_BRACE) {
                            nested++;
                        }
                    }
                }
                std::vector<Token> final = *outer;
                std::vector<Token> params;
                int ps = 0;
                for (auto token = final.begin(); token != final.end(); token++) {
                    if (ps > 0 && !((*token).typ() == RIGHT_PAREN) && ps == 1) {
                        params.push_back(*token);
                    }
                    if ((*token).typ() == LEFT_PAREN) {
                        ps++;
                    } else if ((*token).typ() == RIGHT_PAREN) {
                        ps--;
                    }
                }
                whilecontents.pop_back();
                int stop = 0;
                do {
                    stop++;
                    if (stop == limit) {
                        error(current, "Terminate after control finds repeating while loop, limit:" + std::to_string(limit));
                        return 1;
                    }
                    int result = runtime(whilecontents, scope, error_occurred, limit, precision, return_variable);
                    if (result == 1) {
                        return 1;
                    } else if (result == 47) {
                        break;
                    } else if (result == 44) {
                        break;
                    }
                } while (boolsolve(params, scope, error_occurred));
                break;
            } else if (current.typ() == SLEEP) {
                Token next = *std::next(inner);
                if (next.typ() != LEFT_PAREN) {
                    error(next, "Run-time Error: Expected a left parentheses token. None were provided.");
                    return 1;
                } else {
                    int n = std::next(inner) - stmt.begin();
                    auto nd = std::next(inner);
                    int nested = 0;
                    while (true) {
                        nd++;
                        n++;
                        if (stmt[n].typ() == RIGHT_PAREN && nested == 0) {
                            break;
                        } else {
                            if (stmt[n].typ() == LEFT_PAREN) {
                                nested++;
                            } else if (stmt[n].typ() == RIGHT_PAREN) {
                                nested--;
                            }
                        }
                    }
                    n++;
                    if (stmt[n].typ() != SEMICOLON) {
                        error(current, "Warning: It's prudent to postfix the statement with a semicolon.");
                    };
                    std::vector<Token> segmented(std::next(inner), nd);
                    bool err = false;
                    std::string solved = solve(segmented, scope, &err, precision);
                    if (err) {
                        error(current, "Run-time Error: Evauation Error");
                        return 1;
                    }
                    if (solved.at(0) == '"') {
                        error(current, "Run-time Error: Improper argument.");
                        return 1;
                    }
                    Sleep(std::stod(solved));
                }
                break;
            } else if (current.typ() == BREAK) {
                return 47;
            } else if (current.typ() == IF) {
                Token next = *std::next(inner);
                if (next.typ() != LEFT_PAREN) {
                    error(next, "Run-time Error: Expected a left parentheses token. None were provided.");
                    return 1;
                }
                if (stmt.back().typ() != LEFT_BRACE) {
                    error(next, "Run-time Error: Expected a left bracket token. None were provided.");
                    return 1;
                }

                std::vector<Token> params;
                int ps = 0;
                for (auto token = stmt.begin(); token != stmt.end(); token++) {
                    if (ps > 0 && !((*token).typ() == RIGHT_PAREN) && ps == 1) {
                        params.push_back(*token);
                    }
                    if ((*token).typ() == LEFT_PAREN) {
                        ps++;
                    } else if ((*token).typ() == RIGHT_PAREN) {
                        ps--;
                    }
                }
                std::vector<std::vector<Token>> ifcontents;
                int nested = 0;
                Token fis;
                while (true) {
                    outer++;
                    std::vector<Token> cline = *outer;
                    if (cline.front().typ() == _EOF) {
                        error(current, "Run-time Error: Unending if statement.");
                        return 1;
                    }
                    fis = cline.front();
                    ifcontents.push_back(cline);
                    for (auto cur = cline.begin(); cur < cline.end(); cur++) {
                    }
                    if (fis.typ() == RIGHT_BRACE && nested == 0) {
                        break;
                    } else {
                        if (fis.typ() == RIGHT_BRACE) {
                            nested--;
                        } else if (cline.back().typ() == LEFT_BRACE) {
                            nested++;
                        }
                    }
                }
                if (ifcontents.back().size() == 1) {
                    error(ifcontents.back().back(), "Run-time Error: Expected a ; at the end of the if statement.");
                    return 1;
                } else {
                    if (ifcontents.back()[1].typ() != SEMICOLON && ifcontents.back()[1].typ() != ELSE) {
                        error(ifcontents.back()[1], "Run-time Error: Expected a ; at the end of the if statement.");
                        return 1;
                    }

                    std::vector<std::vector<Token>> elsecontents;
                    if (ifcontents.back()[1].typ() == ELSE) {
                        nested = 0;
                        while (true) {
                            outer++;
                            std::vector<Token> cline = *outer;
                            if (cline.front().typ() == _EOF) {
                                error(current, "Run-time Error: Unending else.");
                                return 1;
                            }
                            fis = cline.front();
                            elsecontents.push_back(cline);
                            if (fis.typ() == RIGHT_BRACE && nested == 0) {
                                break;
                            } else {
                                if (fis.typ() == RIGHT_BRACE) {
                                    nested--;
                                } else if (cline.back().typ() == LEFT_BRACE) {
                                    nested++;
                                }
                            }
                        }
                        if (elsecontents.back().size() == 1) {
                            error(elsecontents.back().back(), "Run-time Error: Expected a ; at the end of the if statement.");
                            return 1;
                        } else {
                            if (elsecontents.back()[1].typ() != SEMICOLON) {
                                error(elsecontents.back()[1], "Run-time Error: Expected a ; at the end of the if statement.");
                                return 1;
                            }
                            elsecontents.pop_back();
                        }
                    }
                    ifcontents.pop_back();

                    if (boolsolve(params, scope, error_occurred)) {
                        int result = runtime(ifcontents, scope, error_occurred, limit, precision, return_variable);
                        if (result == 1) {
                            return 1;
                        } else if (result == 47) {
                            return 47;
                        }
                    } else {
                        if (!elsecontents.empty()) {
                            int result = runtime(elsecontents, scope, error_occurred, limit, precision, return_variable);
                            if (result == 1) {
                                return 1;
                            } else if (result == 47) {
                                return 47;
                            }
                        }
                    }
                }
                break;
            } else if (current.typ() == WHILE) {
                Token next = *std::next(inner);
                if (next.typ() != LEFT_PAREN) {
                    error(next, "Run-time Error: Expected a left parentheses token. None were provided.");
                    return 1;
                }
                if (stmt.back().typ() != LEFT_BRACE) {
                    error(next, "Run-time Error: Expected a left bracket token. None were provided.");
                    return 1;
                }
                std::vector<Token> final = *outer;
                bool err = false;
                std::vector<std::vector<Token>> params = findParams(stmt, inner, SEMICOLON, scope.names, err);
                if (err) {
                    error(current, "Run-time Error: Param error.");
                    return 1;
                } else if (params.size() != 1) {
                    error(current, "Run-time Error: Semicolon why.");
                    return 1;
                }

                std::vector<std::vector<Token>> whilecontents;
                int nested = 0;
                Token fis;
                while (true) {
                    outer++;
                    std::vector<Token> cline = *outer;
                    if (cline.front().typ() == _EOF) {
                        error(current, "Run-time Error: Unending while loop.");
                        return 1;
                    }
                    fis = cline.front();
                    whilecontents.push_back(cline);
                    if (fis.typ() == RIGHT_BRACE && nested == 0) {
                        break;
                    } else {
                        if (fis.typ() == RIGHT_BRACE) {
                            nested--;
                        } else if (cline.back().typ() == LEFT_BRACE) {
                            nested++;
                        }
                    }
                }
                if (whilecontents.back()[1].typ() != SEMICOLON) {
                    error(whilecontents.back()[0], "Run-time Error: Expected a ; at the end of the while statement.");
                    return 1;
                }
                whilecontents.pop_back();
                int stop = 0;
                while (boolsolve(params[0], scope, error_occurred)) {
                    stop++;
                    if (stop == limit) {
                        error(current, "Terminate after control finds repeating while loop, limit: " + std::to_string(limit));
                        return 1;
                    }
                    int result = runtime(whilecontents, scope, error_occurred, limit, precision, return_variable);
                    if (result == 1) {
                        return 1;
                    } else if (result == 47) {
                        break;
                    } else if (result == 44) {
                        break;
                    }
                }
                break;
            } else if (current.typ() == FUN) {
                if ((*std::next(inner)).typ() != IDENTIFIER) {
                    error(*std::next(inner), "Run-time Error: Inadequite function name.");
                    return 1;
                } else {
                    scope.function_names.push_back((*std::next(inner)).str());
                }
                if ((*std::prev(inner)).typ() == AWARE) {
                    scope.aware_functions.push_back((*std::next(inner)).str());
                }
                std::vector<Token> final = *outer;
                std::vector<std::string> current_params;
                int ps = 0;
                for (auto token = final.begin(); token != final.end(); token++) {
                    if (ps > 0 && !((*token).typ() == RIGHT_PAREN) && ps == 1) {
                        if ((*token).typ() == IDENTIFIER || (*token).typ() == COMMA) {
                            if ((*token).typ() == IDENTIFIER)
                            current_params.push_back((*token).str());
                        } else {
                            error(current, "a.");
                            return 1;
                        }
                    }
                    if ((*token).typ() == LEFT_PAREN) {
                        ps++;
                    } else if ((*token).typ() == RIGHT_PAREN) {
                        ps--;
                    }
                }
                scope.function_params.push_back(current_params);
                std::vector<std::vector<Token>> func_body;
                int nested = 0;
                Token fis;
                while (true) {
                    outer++;
                    std::vector<Token> cline = *outer;
                    if (cline.front().typ() == _EOF) {
                        error(current, "Run-time Error: Unending function.");
                        return 1;
                    }
                    fis = cline.front();
                    func_body.push_back(cline);
                    if (fis.typ() == RIGHT_BRACE && nested == 0) {
                        break;
                    } else {
                        if (fis.typ() == RIGHT_BRACE) {
                            nested--;
                        }
                        if (cline.back().typ() == LEFT_BRACE) {
                            nested++;
                        }
                    }
                }
                if (func_body.back()[1].typ() != SEMICOLON) {
                    error(func_body.back()[1], "Run-time Error: Expected a ; at the end of the function declaration.");
                    return 1;
                }
                func_body.pop_back();
                scope.function_bodies.push_back(func_body);
                break;
            } else if (current.typ() == ELSE) {
                error(current, "Run-time Error: Stray else.");
                return 1;
            } else if (current.typ() == RIGHT_BRACE || current.typ() == LEFT_BRACE || current.typ() == RIGHT_PAREN || current.typ() == LEFT_PAREN) {
                error(current, "Run-time Error: Stray token.");
                return 1;
            } else if (current.typ() == RETURN) {
                int n = std::next(inner) - stmt.begin();
                auto nd = std::next(inner);
                int nested = 0;
                while (true) {
                    nd++;
                    n++;
                    if (stmt[n].typ() == RIGHT_PAREN && nested == 0) {
                        break;
                    } else {
                        if (stmt[n].typ() == LEFT_PAREN) {
                            nested++;
                        } else if (stmt[n].typ() == RIGHT_PAREN) {
                            nested--;
                        } else if (nd == stmt.end()) {
                            error(stmt.back(), "Run-time Error: Unclosed parentheses");
                            return 44;
                        }
                    }
                }
                n++;
                if (stmt[n].typ() != SEMICOLON) {
                    error(current, "Warning: It's prudent to postfix the statement with a semicolon.");
                };
                std::vector<Token> segmented(std::next(inner), nd);
                bool boolean = false;
                for (auto its = segmented.begin(); its < segmented.end(); its++) {
                    if ((*its).typ() == LEFT_PAREN)
                        continue;
                    if ((*its).typ() == IDENTIFIER) {
                        Type t;
                        if (getVarVal(*its, scope, error_occurred).at(0) == '"') {
                            t = STRING;
                        } else if (getVarVal(*its, scope, error_occurred) == "true") {
                            t = TTRUE;
                        } else if (getVarVal(*its, scope, error_occurred) == "false") {
                            t = TFALSE;
                        } else {
                            t = NUMBER;
                        }
                        return_variable.push_back(Token(getVarVal(*its, scope, error_occurred), (*its).lines(), (*its).col(), t, (*its).actual_line()));
                    } else {
                        return_variable.push_back(*its);
                    }
                }
                return 47;
            }
        }
    }
    return 0;
}