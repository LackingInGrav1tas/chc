#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <ctime>
#include <winuser.h>
#include <winbase.h>
#include <windows.h>
#include "header.hpp"

int handle_functions(std::vector<Token> &stmt, Scope &scope, int limit, int precision, std::vector<Scope> &scopes, std::vector<std::string> &scope_indices, bool *error_occurred) {
    std::vector<Type> native_functions = { TOKEN_INPUT, ASSERT, WRITETO, LENGTH, HASH, THROW, EVAL, RAND, AT, SET_SCOPE, SAVE_SCOPE, STR, TOKEN_INT, IS_STRING, IS_NUMBER, IS_BOOL, SOLVE };
    for (auto token = stmt.end()-1; token >= stmt.begin() && (*stmt.begin()).typ() != WHILE && (*stmt.begin()).typ() != DISPOSE; token--) {
        if (in((*token).str(), scope.function_names)) {
            Token token_copy((*token).str(), (*token).lines(), (*token).col(), (*token).typ(), (*token).actual_line(), (*token).filename());
            bool eroc = false;
            auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
            if (eroc)
                return EXIT_FAILURE;
            std::pair<bool, int> found = findInV(scope.function_names, (*token).str());
            if (found.first) {
                if (call_params.size() != scope.function_params[found.second].size()) {
                    std::string msg = "Run-time Error: Received " + std::to_string(call_params.size()) + " params but expected " + std::to_string(scope.function_params[found.second].size());
                    error(*token, msg);
                    return EXIT_FAILURE;
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
                        return EXIT_FAILURE;
                    }
                }
                std::vector<Token> return_val;
                bool er = false;
                int result = runtime(scope.function_bodies[found.second], nscope, &er, limit, precision, return_val);
                if (result == 1) {
                    return EXIT_FAILURE;
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
                    stmt.insert(stmt.begin()+ct, Token("_void_func_holder", token_copy.lines(), token_copy.col(), token_copy.typ(), token_copy.actual_line(), token_copy.filename()));
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
                    return EXIT_FAILURE;
                if (call_params.size() != 1) {
                    std::string msg = "Run-time Error: Received " + std::to_string(call_params.size()) + " params but expected 1.";
                    error(*token, msg);
                    return EXIT_FAILURE;
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
                stmt.insert(stmt.begin()+ct, Token('"' + raw_input + '"', (*token).lines(), (*token).col(), STRING, (*token).actual_line(), (*token).filename()));
            } else if ((*token).typ() == WRITETO) {
                bool eroc = false;
                auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                if (eroc)
                    return EXIT_FAILURE;
                if (call_params.size() < 2) {
                    std::string msg = "Run-time Error: Received " + std::to_string(call_params.size()) + " params but expected 3.";
                    error(*token, msg);
                    return EXIT_FAILURE;
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
                        return EXIT_FAILURE;
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
                        return EXIT_FAILURE;
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
                stmt.insert(stmt.begin()+ct, Token(fon, (*token).lines(), (*token).col(), foundornot, (*token).actual_line(), (*token).filename()));
            } else if ((*token).typ() == ASSERT) {
                if ((*std::next(token)).typ() != LEFT_PAREN) {
                    error(*std::next(token), "Run-time Error: Incorrect formatting of assert call.");
                    return EXIT_FAILURE;
                }
                bool eroc = false;
                auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                if (eroc)
                    return EXIT_FAILURE;
                for (auto param_segment = call_params.begin(); param_segment < call_params.end(); param_segment++) {
                    if (!boolsolve(*param_segment, scope, limit, precision, scopes, scope_indices, error_occurred)) {
                        error((*param_segment).front(), "Run-time Error: Assertion Failed.");
                        return EXIT_FAILURE;
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
                    return EXIT_FAILURE;
                }
                if (e) {
                    error(*std::prev(std::prev(token)), "Run-time Error: ");
                    return EXIT_FAILURE;
                }
                bool eroc = false;
                auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                if (eroc)
                    return EXIT_FAILURE;
                if (call_params.size() != 1) {
                    error(*token, "Run-time Error: Expected 1 parameter. Received " + std::to_string(call_params.size()) + ".");
                    return EXIT_FAILURE;
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
                stmt.insert(stmt.begin()+ct, Token(std::to_string(solve(call_params[0], scope, &e, precision).length() - 2), (*token).lines(), (*token).col(), NUMBER, (*token).actual_line(), (*token).filename()));
            } else if ((*token).typ() == HASH) {
                if ((*std::next(token)).typ() != LEFT_PAREN) {
                    error(*std::next(token), "Run-time Error: Expected a left bracket token.");
                    return EXIT_FAILURE;
                }
                bool eroc = false;
                auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                if (eroc)
                    return EXIT_FAILURE;
                if (call_params.size() != 1) {
                    error(*token, "Run-time Error: Expected 1 parameter. Received " + std::to_string(call_params.size()) + ".");
                    return EXIT_FAILURE;
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
                stmt.insert(stmt.begin()+ct, Token('"' + hash(solved) + '"', (*token).lines(), (*token).col(), STRING, (*token).actual_line(), (*token).filename()));
            } else if ((*token).typ() == EVAL) {
                if ((*std::next(token)).typ() != LEFT_PAREN) {
                    error(*std::next(token), "Run-time Error: Incorrect formatting of function call.");
                    return EXIT_FAILURE;
                }
                bool eroc = false;
                auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                if (eroc)
                    return EXIT_FAILURE;
                if (call_params.size() != 1) {
                    error(*token, "Run-time Error: Expected 1 parameter. Received " + std::to_string(call_params.size()) + ".");
                    return EXIT_FAILURE;
                }
                Type evaluated = TFALSE;
                std::string bol = "false";
                if (boolsolve(call_params[0], scope, limit, precision, scopes, scope_indices, error_occurred)) {
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
                stmt.insert(stmt.begin()+ct, Token(bol, (*token).lines(), (*token).col(), evaluated, (*token).actual_line(), (*token).filename()));
            } else if ((*token).typ() == SOLVE) {
                if ((*std::next(token)).typ() != LEFT_PAREN) {
                    error(*std::next(token), "Run-time Error: Incorrect formatting of function call.");
                    return EXIT_FAILURE;
                }
                bool eroc = false;
                auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                if (eroc)
                    return EXIT_FAILURE;
                if (call_params.size() != 1) {
                    error(*token, "Run-time Error: Expected 1 parameter. Received " + std::to_string(call_params.size()) + ".");
                    return EXIT_FAILURE;
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
                stmt.insert(stmt.begin()+ct, Token(solved, (*token).lines(), (*token).col(), evaluated, (*token).actual_line(), (*token).filename()));
            } else if ((*token).typ() == THROW) {
                bool eroc = false;
                auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                if (eroc)
                    return EXIT_FAILURE;
                if (call_params.size() != 1) {
                    std::string msg = "Run-time Error: Received " + std::to_string(call_params.size()) + " params but expected 1.";
                    error(*token, msg);
                    return EXIT_FAILURE;
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
                return EXIT_FAILURE;
            } else if ((*token).typ() == RAND) {
                if ((*std::next(token)).typ() != LEFT_PAREN) {
                    error(*std::next(token), "Run-time Error: Incorrect formatting of function call.");
                    return EXIT_FAILURE;
                }
                bool eroc = false;
                auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                if (eroc)
                    return EXIT_FAILURE;
                if (call_params.size() != 1) {
                    error(*token, "Run-time Error: Expected 1 parameter. Received " + std::to_string(call_params.size()) + ".");
                    return EXIT_FAILURE;
                }
                std::string solved = solve(call_params[0], scope, error_occurred, precision);
                std::string ret;
                try {
                    srand(time(NULL));
                    ret = std::to_string(rand() % std::stoi(solved));
                } catch (...) {
                    error(*token, "Run-time Error: rand() expects an intiger value as a parameter.");
                    return EXIT_FAILURE;
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
                stmt.insert(stmt.begin()+ct, Token(ret, (*token).lines(), (*token).col(), NUMBER, (*token).actual_line(), (*token).filename()));
            } else if ((*token).typ() == AT) {
                if ((*std::next(token)).typ() != LEFT_PAREN) {
                    error(*std::next(token), "Run-time Error: Incorrect formatting of function call.");
                    return EXIT_FAILURE;
                }
                bool e = false;
                bool eroc = false;
                auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                if (eroc)
                    return EXIT_FAILURE;
                if (call_params.size() != 2) {
                    error(*token, "Run-time Error: Expected 2 parameters. Received " + std::to_string(call_params.size()) + ".");
                    return EXIT_FAILURE;
                }
                std::string value = solve(call_params[0], scope, &e, precision).substr(1, solve(call_params[0], scope, &e, precision).length()-2);
                std::string solved = solve(call_params[1], scope, &e, precision);
                if (e) {
                    error(call_params[0][0], "Run-time Error: Solving error.");
                    return EXIT_FAILURE;
                }
                int pos;
                try {pos = std::stoi(solved);}
                catch(...) {
                    error(*token, "Run-time Error: Expected an intiger.");
                    return EXIT_FAILURE;
                }
                if (pos >= value.length()) {
                    error(*token, "Run-time Error: " + std::to_string(pos) + " out of range.");
                    return EXIT_FAILURE;
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
                stmt.insert(stmt.begin()+ct, Token(R"(")" + getString(value.at(pos)) + R"(")", (*token).lines(), (*token).col(), STRING, (*token).actual_line(), (*token).filename()));
            } else if ((*token).typ() == SAVE_SCOPE) {
                if ((*std::next(token)).typ() != LEFT_PAREN) {
                    error(*std::next(token), "Run-time Error: Incorrect formatting of assert call.");
                    return EXIT_FAILURE;
                }
                bool eroc = false;
                auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                if (eroc)
                    return EXIT_FAILURE;
                if (call_params.size() != 1) {
                    error(*token, "Run-time Error: Expected 1 parameter, received " + std::to_string(call_params.size()) + ".");
                    return EXIT_FAILURE;
                } else if (call_params[0].size() != 1) {
                    error(*token, "Run-time Error: Expected 1 parameter, received " + std::to_string(call_params.size()) + ".");
                    return EXIT_FAILURE;
                }
                bool gvve = false;
                scopes.push_back(scope);
                scope_indices.push_back(getVarVal(*std::next(std::next(token)), scope, &gvve));
                if (gvve) {
                    error(*std::next(std::next(token)), "Run-time Error: Eval error.");
                    return EXIT_FAILURE;
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
                    return EXIT_FAILURE;
                }
                bool eroc = false;
                auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                if (eroc)
                    return EXIT_FAILURE;
                if (call_params.size() != 1) {
                    error(*token, "Run-time Error: Expected 1 parameter, received " + std::to_string(call_params.size()) + ".");
                    return EXIT_FAILURE;
                }
                bool gvve = false;
                auto found = findInV(scope_indices, getVarVal(*std::next(std::next(token)), scope, &gvve));
                if (gvve) {
                    error(*std::next(std::next(token)), "Run-time Error: Eval error.");
                    return EXIT_FAILURE;
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
                    return EXIT_FAILURE;
                }
                if (e) {
                    error(*std::prev(std::prev(token)), "Run-time Error: ");
                    return EXIT_FAILURE;
                }
                bool eroc = false;
                auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                if (eroc)
                    return EXIT_FAILURE;
                if (call_params.size() != 1) {
                    error(*token, "Run-time Error: Expected 1 parameter. Received " + std::to_string(call_params.size()) + ".");
                    return EXIT_FAILURE;
                }
                std::string solved = solve(call_params[0], scope, &e, precision);
                if (solved.at(0) == '"') {
                    error(*token, "Run-time Error: Expected non-string as an input.");
                    return EXIT_FAILURE;
                }
                if (e) {
                    error(*token, "Run-time Error: Solving error.");
                    return EXIT_FAILURE;
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
                stmt.insert(stmt.begin()+ct, Token(R"(")" + solved + R"(")", (*token).lines(), (*token).col(), STRING, (*token).actual_line(), (*token).filename() ));
            } else if ((*token).typ() == TOKEN_INT) {
                bool e = false;
                if ((*std::next(token)).typ() != LEFT_PAREN) {
                    error(*std::next(token), "Run-time Error: Expected a left bracket token.");
                    return EXIT_FAILURE;
                }
                if (e) {
                    error(*std::prev(std::prev(token)), "Run-time Error: ");
                    return EXIT_FAILURE;
                }
                bool eroc = false;
                auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                if (eroc)
                    return EXIT_FAILURE;
                if (call_params.size() != 1) {
                    error(*token, "Run-time Error: Expected 1 parameter. Received " + std::to_string(call_params.size()) + ".");
                    return EXIT_FAILURE;
                }
                std::string solved = solve(call_params[0], scope, &e, precision);
                if (solved.at(0) != '"' && solved != "true" && solved == "false") {
                    error(*token, "Run-time Error: Expected non-number as an input.");
                    return EXIT_FAILURE;
                } else if (solved == "true") {
                    solved = "1";
                } else if (solved == "false") {
                    solved = "0";
                } else {
                    solved = solved.substr(1, solved.length()-2);
                }
                if (e) {
                    error(*token, "Run-time Error: Solving error.");
                    return EXIT_FAILURE;
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
                stmt.insert(stmt.begin()+ct, Token(solved, (*token).lines(), (*token).col(), NUMBER, (*token).actual_line(), (*token).filename() ));
            } else if ((*token).typ() == IS_STRING) {
                bool eroc = false;
                auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                if (eroc)
                    return EXIT_FAILURE;
                if (call_params.size() != 1) {
                    std::string msg = "Run-time Error: Received " + std::to_string(call_params.size()) + " params but expected 1.";
                    error(*token, msg);
                    return EXIT_FAILURE;
                } else if (call_params[0].size() != 1) {
                    std::string msg = "Run-time Error: Received " + std::to_string(call_params.size()) + " params but expected 1.";
                    error(*token, msg);
                    return EXIT_FAILURE;
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
                stmt.insert(stmt.begin()+ct, Token(fon, (*token).lines(), (*token).col(), f_on, (*token).actual_line(), (*token).filename()));
            } else if ((*token).typ() == IS_NUMBER) {
                bool eroc = false;
                auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                if (eroc)
                    return EXIT_FAILURE;
                if (call_params.size() != 1) {
                    std::string msg = "Run-time Error: Received " + std::to_string(call_params.size()) + " params but expected 1.";
                    error(*token, msg);
                    return EXIT_FAILURE;
                } else if (call_params[0].size() != 1) {
                    std::string msg = "Run-time Error: Received " + std::to_string(call_params.size()) + " params but expected 1.";
                    error(*token, msg);
                    return EXIT_FAILURE;
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
                stmt.insert(stmt.begin()+ct, Token(fon, (*token).lines(), (*token).col(), f_on, (*token).actual_line(), (*token).filename()));
            } else if ((*token).typ() == IS_BOOL) {
                bool eroc = false;
                auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
                if (eroc)
                    return EXIT_FAILURE;
                if (call_params.size() != 1) {
                    std::string msg = "Run-time Error: Received " + std::to_string(call_params.size()) + " params but expected 1.";
                    error(*token, msg);
                    return EXIT_FAILURE;
                } else if (call_params[0].size() != 1) {
                    std::string msg = "Run-time Error: Received " + std::to_string(call_params.size()) + " params but expected 1.";
                    error(*token, msg);
                    return EXIT_FAILURE;
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
                stmt.insert(stmt.begin()+ct, Token(fon, (*token).lines(), (*token).col(), f_on, (*token).actual_line(), (*token).filename()));
            }
            token++;
        }
    }
}