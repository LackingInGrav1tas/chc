#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <ctime>
#include <windows.h>
#include "header.hpp"

std::vector<Scope> scopes;
std::vector<std::string> scope_indices;
std::vector<Scope> fun_scopes;
std::vector<std::string> fun_scope_names;

int handle_functions(std::vector<Token> &stmt, Scope &scope, int limit, int precision, bool *error_occurred) {
    std::vector<Type> native_functions = { TOKEN_INPUT, ASSERT, WRITETO, LENGTH, HASH, THROW, EVAL, RAND, AT, SET_SCOPE, SAVE_SCOPE, STR, TOKEN_INT, IS_STRING, IS_NUMBER, IS_BOOL, SOLVE };
    for (auto token = stmt.end()-1; token >= stmt.begin() && (*stmt.begin()).typ() != WHILE && (*std::next(stmt.begin())).typ() != WHILE && (*stmt.begin()).typ() != DISPOSE; token--) {
        if ((*token).typ() == LEFT_PAREN && !in((*std::prev(token)).typ(), native_functions) && !in((*std::prev(token)).str(), scope.function_names) && stmt[0].typ() != FUN && stmt[0].typ() != AWARE && (*std::prev(token)).typ() != RFPRINT && (*std::prev(token)).typ() != PRINT && (*std::prev(token)).typ() != FPRINT) {
            bool eroc = false;
            auto call_params = findParams(stmt, token, COMMA, scope.names, eroc);
            if (eroc)
                return EXIT_FAILURE;
            if (call_params.size() != 1) {
                error(*token, "Run-time Error: comma.");
                return EXIT_FAILURE;
            }
            std::string return_type = "non-boolean";
            std::vector<Type> bool_operators = { EQUAL_EQUAL, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL, EXC_EQUAL };
            for (auto ret = call_params[0].rbegin(); ret < call_params[0].rend(); ret++) {
                if (in((*ret).typ(), bool_operators))
                    return_type = "boolean";
            }
            int ct = token - stmt.begin();
            if (return_type == "non-boolean")
                stmt.insert(stmt.begin()+ct, Token("solve", (*token).lines(), (*token).col(), SOLVE, (*token).actual_line(), (*token).filename()));
            else
                stmt.insert(stmt.begin()+ct, Token("eval", (*token).lines(), (*token).col(), EVAL, (*token).actual_line(), (*token).filename()));
            token = stmt.end()-1;
            continue;
        }

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
                Scope nscope = fun_scopes[findInV(fun_scope_names, (*token).str()).second];
                std::string rv;
                if (in((*token).str(), scope.aware_functions))
                    nscope = scope;
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
                if (result == EXIT_FAILURE) {
                    return EXIT_FAILURE;
                }
                if (in((*token).str(), scope.aware_functions))
                    scope = nscope;
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
                    if (return_val.size() == 1)
                        stmt.insert(stmt.begin()+ct, return_val.back());
                    else {
                        std::string return_type = "non-boolean";
                        std::vector<Type> bool_operators = { EQUAL_EQUAL, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL, EXC_EQUAL };
                        for (auto ret = return_val.rbegin(); ret < return_val.rend(); ret++) {
                            if (in((*ret).typ(), bool_operators))
                                return_type = "boolean";
                        }
                        stmt.insert(stmt.begin()+ct, Token(")", (*token).lines(), (*token).col(), RIGHT_PAREN, (*token).actual_line(), (*token).filename()));
                        for (auto ret = return_val.rbegin(); ret < return_val.rend(); ret++) {
                            stmt.insert(stmt.begin()+ct, *ret);
                        }
                        stmt.insert(stmt.begin()+ct, Token("(", (*token).lines(), (*token).col(), LEFT_PAREN, (*token).actual_line(), (*token).filename()));
                        if (return_type == "non-boolean")
                            stmt.insert(stmt.begin()+ct, Token("solve", (*token).lines(), (*token).col(), SOLVE, (*token).actual_line(), (*token).filename()));
                        else
                            stmt.insert(stmt.begin()+ct, Token("eval", (*token).lines(), (*token).col(), EVAL, (*token).actual_line(), (*token).filename()));
                        token = stmt.end();
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
                if (getVarVal(call_params[0][0], scope, &eroc).at(0) == '"' || getVarVal(call_params[0][0], scope, &eroc) == "true" || getVarVal(call_params[0][0], scope, &eroc) == "false") {
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
                if (getVarVal(call_params[0][0], scope, &eroc) == "true" || getVarVal(call_params[0][0], scope, &eroc) == "false") {
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

int runtime(std::vector<std::vector<Token>> statements, Scope &scope, bool *error_occurred, int limit, int precision, std::vector<Token> &return_variable) {
    std::vector<std::string> constants = { "@EOL", "@sec", "@min", "@hour", "@mday", "@yday", "@mon", "@year", "@clipboard", "@home", "@desktopW", "@desktopH", "@environment", "@IP", "@inf", "@write", "@append" };
    for (auto outer = statements.begin(); outer < statements.end(); std::advance(outer, 1)) {
        std::vector<Token> stmt = *outer;
        
        int result = handle_functions(stmt, scope, limit, precision, error_occurred);
        if (result == 1)
            return 1;

        int size = 0;
        for (auto inner = stmt.begin(); inner < stmt.end(); inner++) {
            size++;
            Token current = *inner;
            if (current.typ() == EQUAL) {//setting variable values
                Token previous = *std::prev(inner);
                if (previous.typ() != IDENTIFIER) {
                    error(previous, "Runtime Error: Inadequite identifier.");
                    return EXIT_FAILURE;
                } else {
                    for (auto tok = inner; tok < stmt.end(); tok++) {
                        if ((*tok).typ() == IDENTIFIER && findInV(scope.names, (*tok).str()).first == false && !in((*tok).str(), constants)) {
                            error(*tok, "Run-time Error: Undefined variable.");
                            return EXIT_FAILURE;
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
                            return EXIT_FAILURE;
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
                        return EXIT_FAILURE;
                    }
                }
                break;
            } else if (current.typ() == MINUS_MINUS) {
                if ((*std::next(inner)).typ() != SEMICOLON) {
                    error((*std::next(inner)), "Run-time Error: Expected a semicolon.");
                    return EXIT_FAILURE;
                } else if ((*std::prev(inner)).typ() != IDENTIFIER) {
                    error((*std::next(inner)), "Run-time Error: Expected an identifier.");
                    return EXIT_FAILURE;
                }
                double pos;
                bool err = false;
                try {pos = std::stod(getVarVal((*std::prev(inner)), scope, &err)) - 1;}
                catch (...) {
                    error((*std::prev(inner)), "Run-time Error: Expected a number identifier.");
                    return EXIT_FAILURE;
                }
                scope.names.push_back((*std::prev(inner)).str());
                std::string preshortened = to_string_with_precision(pos, precision);
                shorten(preshortened);
                scope.values.push_back(preshortened);
                break;
            } else if (current.typ() == PLUS_PLUS) {
                if ((*std::next(inner)).typ() != SEMICOLON) {
                    error((*std::next(inner)), "Run-time Error: Expected a semicolon.");
                    return EXIT_FAILURE;
                } else if ((*std::prev(inner)).typ() != IDENTIFIER) {
                    error((*std::next(inner)), "Run-time Error: Expected an identifier.");
                    return EXIT_FAILURE;
                }
                double pos;
                bool err = false;
                try {pos = std::stod(getVarVal((*std::prev(inner)), scope, &err)) + 1;}
                catch (...) {
                    error((*std::prev(inner)), "Run-time Error: Expected a number identifier.");
                    return EXIT_FAILURE;
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
                    return EXIT_FAILURE;
                } else {
                    int n = std::next(inner) - stmt.begin();
                    auto nd = std::next(inner);
                    int nested = 0;
                    while (true) {
                        if ((*nd).typ() == IDENTIFIER && findInV(scope.names, (*nd).str()).first == false && !in((*nd).str(), constants)) {
                            error(*nd, "Run-time Error: Undefined variable.");
                            return EXIT_FAILURE;
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
                                return EXIT_FAILURE;
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
            } else if (current.typ() == FPRINT) {
                bool eroc = false;
                auto call_params = findParams(stmt, inner, COMMA, scope.names, eroc);
                if (eroc)
                    return EXIT_FAILURE;
                if (call_params.size() != 1) {
                    std::string msg = "Run-time Error: Received " + std::to_string(call_params.size()) + " params but expected 1.";
                    error(current, msg);
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
                    } else if ((*cur).typ() != PLUS) {
                        error(*cur, "Run-time Error: Stray operator.");
                        return EXIT_FAILURE;
                    }
                }
                std::cout << solved << std::endl;
                break;
            } else if (current.typ() == RPRINT) {
                bool eroc = false;
                auto call_params = findParams(stmt, inner, COMMA, scope.names, eroc);
                if (eroc)
                    return EXIT_FAILURE;
                if (call_params.size() != 1) {
                    std::string msg = "Run-time Error: Received " + std::to_string(call_params.size()) + " params but expected 1.";
                    error(current, msg);
                    return EXIT_FAILURE;
                }
                std::string solved;
                for (auto cur = call_params[0].begin(); cur < call_params[0].end(); cur++) {
                    if ((*cur).syhtyp() == TERMINAL) {
                        std::string currentString = getVarVal(*cur, scope, error_occurred);
                        solved += currentString;
                    } else if ((*cur).typ() != PLUS) {
                        error(*cur, "Run-time Error: Stray operator.");
                        return EXIT_FAILURE;
                    }
                }
                std::cout << solved;
                break;
            } else if (current.typ() == RFPRINT) {
                bool eroc = false;
                auto call_params = findParams(stmt, inner, COMMA, scope.names, eroc);
                if (eroc)
                    return EXIT_FAILURE;
                if (call_params.size() != 1) {
                    std::string msg = "Run-time Error: Received " + std::to_string(call_params.size()) + " params but expected 1.";
                    error(current, msg);
                    return EXIT_FAILURE;
                }
                std::string solved;
                for (auto cur = call_params[0].begin(); cur < call_params[0].end(); cur++) {
                    if ((*cur).syhtyp() == TERMINAL) {
                        std::string currentString = getVarVal(*cur, scope, error_occurred);
                        solved += currentString;
                    } else if ((*cur).typ() != PLUS) {
                        error(*cur, "Run-time Error: Stray operator.");
                        return EXIT_FAILURE;
                    }
                }
                std::cout << solved << std::endl;
                break;
            } else if (current.typ() == RUN) {
                Token next = *std::next(inner);
                if (next.typ() != LEFT_PAREN) {
                    error(next, "Run-time Error: Expected a left parentheses token. None were provided.");
                    return EXIT_FAILURE;
                } else {
                    int n = std::next(inner) - stmt.begin();
                    auto nd = std::next(inner);
                    int nested = 0;
                    while (true) {
                        if ((*nd).typ() == IDENTIFIER && findInV(scope.names, (*nd).str()).first == false && !in((*nd).str(), constants)) {
                            error(*nd, "Run-time Error: Undefined variable.");
                            return EXIT_FAILURE;
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
                        return EXIT_FAILURE;
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
                    return EXIT_FAILURE;
                }
                std::vector<std::vector<Token>> whilecontents;
                int nested = 0;
                Token fis;
                while (true) {
                    outer++;
                    std::vector<Token> line = *outer;
                    if (line.front().typ() == _EOF) {
                        error(current, "Run-time Error: Unending do...while loop.");
                        return EXIT_FAILURE;
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
                        return EXIT_FAILURE;
                    }
                    int result = runtime(whilecontents, scope, error_occurred, limit, precision, return_variable);
                    if (result == 1) {
                        return EXIT_FAILURE;
                    } else if (result == 47) {
                        break;
                    } else if (result == 44) {
                        break;
                    }
                } while (boolsolve(params, scope, limit, precision, scopes, scope_indices, error_occurred));
                break;
            } else if (current.typ() == SLEEP) {
                Token next = *std::next(inner);
                if (next.typ() != LEFT_PAREN) {
                    error(next, "Run-time Error: Expected a left parentheses token. None were provided.");
                    return EXIT_FAILURE;
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
                        return EXIT_FAILURE;
                    }
                    if (solved.at(0) == '"') {
                        error(current, "Run-time Error: Improper argument.");
                        return EXIT_FAILURE;
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
                    return EXIT_FAILURE;
                }
                if (stmt.back().typ() != LEFT_BRACE) {
                    error(next, "Run-time Error: Expected a left bracket token. None were provided.");
                    return EXIT_FAILURE;
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
                        return EXIT_FAILURE;
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
                    return EXIT_FAILURE;
                } else {
                    if (ifcontents.back()[1].typ() != SEMICOLON && ifcontents.back()[1].typ() != ELSE) {
                        error(ifcontents.back()[1], "Run-time Error: Expected a ; at the end of the if statement.");
                        return EXIT_FAILURE;
                    }

                    std::vector<std::vector<Token>> elsecontents;
                    if (ifcontents.back()[1].typ() == ELSE) {
                        nested = 0;
                        while (true) {
                            outer++;
                            std::vector<Token> cline = *outer;
                            if (cline.front().typ() == _EOF) {
                                error(current, "Run-time Error: Unending else.");
                                return EXIT_FAILURE;
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
                            return EXIT_FAILURE;
                        } else {
                            if (elsecontents.back()[1].typ() != SEMICOLON) {
                                error(elsecontents.back()[1], "Run-time Error: Expected a ; at the end of the if statement.");
                                return EXIT_FAILURE;
                            }
                            elsecontents.pop_back();
                        }
                    }
                    ifcontents.pop_back();

                    if (boolsolve(params, scope, limit, precision, scopes, scope_indices, error_occurred)) {
                        int result = runtime(ifcontents, scope, error_occurred, limit, precision, return_variable);
                        if (result == 1) {
                            return EXIT_FAILURE;
                        } else if (result == 47) {
                            return 47;
                        }
                    } else {
                        if (!elsecontents.empty()) {
                            int result = runtime(elsecontents, scope, error_occurred, limit, precision, return_variable);
                            if (result == 1) {
                                return EXIT_FAILURE;
                            } else if (result == 47) {
                                return 47;
                            }
                        }
                    }
                }
                break;
            } else if (current.typ() == TRY) {
                Token next = *std::next(inner);
                if (next.typ() != LEFT_BRACE) {
                    error(next, "Run-time Error: Expected a left parentheses token. None were provided.");
                    return EXIT_FAILURE;
                }
                
                std::vector<std::vector<Token>> trycontents;
                int nested = 0;
                Token fis;
                while (true) {
                    outer++;
                    std::vector<Token> cline = *outer;
                    if (cline.front().typ() == _EOF) {
                        error(current, "Run-time Error: Unending if statement.");
                        return EXIT_FAILURE;
                    }
                    fis = cline.front();
                    trycontents.push_back(cline);
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
                if (trycontents.back().size() == 1) {
                    error(trycontents.back().back(), "Run-time Error: Expected a ; at the end of the try statement.");
                    return EXIT_FAILURE;
                } else {
                    if (trycontents.back()[1].typ() != SEMICOLON && trycontents.back()[1].typ() != CATCH) {
                        error(trycontents.back()[1], "Run-time Error: Expected a ; at the end of the try statement.");
                        return EXIT_FAILURE;
                    }

                    std::vector<std::vector<Token>> catchcontents;
                    if (trycontents.back()[1].typ() == CATCH) {
                        nested = 0;
                        while (true) {
                            outer++;
                            std::vector<Token> cline = *outer;
                            if (cline.front().typ() == _EOF) {
                                error(current, "Run-time Error: Unending else.");
                                return EXIT_FAILURE;
                            }
                            fis = cline.front();
                            catchcontents.push_back(cline);
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
                        if (catchcontents.back().size() == 1) {
                            error(catchcontents.back().back(), "Run-time Error: Expected a ; at the end of the catch statement.");
                            return EXIT_FAILURE;
                        } else {
                            if (catchcontents.back()[1].typ() != SEMICOLON) {
                                error(catchcontents.back()[1], "Run-time Error: Expected a ; at the end of the catch statement.");
                                return EXIT_FAILURE;
                            }
                            catchcontents.pop_back();
                        }
                    }
                    trycontents.pop_back();
                    std::cerr.setstate(std::ios_base::failbit);
                    int result = runtime(catchcontents, scope, error_occurred, limit, precision, return_variable);
                    std::cerr.clear();
                    if (result == EXIT_FAILURE) {
                        if (!catchcontents.empty()) {
                            int result = runtime(catchcontents, scope, error_occurred, limit, precision, return_variable);
                            if (result == 1) {
                                return EXIT_FAILURE;
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
                    return EXIT_FAILURE;
                }
                if (stmt.back().typ() != LEFT_BRACE) {
                    error(next, "Run-time Error: Expected a left bracket token. None were provided.");
                    return EXIT_FAILURE;
                }
                std::vector<Token> final = *outer;
                bool err = false;
                //finding the inside of 
                std::vector<Token> params;
                int ns = 1;
                inner += 2;
                while (true) {//while ( foo ( a ) )
                    if ((*inner).typ() == LEFT_PAREN)
                        ns++;
                    else if ((*inner).typ() == RIGHT_PAREN)
                        ns--;
                    if ((*inner).typ() == RIGHT_PAREN && ns == 0)
                        break;
                    params.push_back(*inner);
                    inner++;
                }
                if (err) {
                    error(current, "Run-time Error: Param error.");
                    return EXIT_FAILURE;
                }

                std::vector<std::vector<Token>> whilecontents;
                int nested = 0;
                Token fis;
                while (true) {
                    outer++;
                    std::vector<Token> cline = *outer;
                    if (cline.front().typ() == _EOF) {
                        error(current, "Run-time Error: Unending while loop.");
                        return EXIT_FAILURE;
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
                    return EXIT_FAILURE;
                }
                whilecontents.pop_back();
                int stop = 0;
                while (boolsolve(params, scope, limit, precision, scopes, scope_indices, error_occurred)) {
                    stop++;
                    if (stop == limit) {
                        error(current, "Terminate after control finds repeating while loop, limit: " + std::to_string(limit));
                        return EXIT_FAILURE;
                    }
                    int result = runtime(whilecontents, scope, error_occurred, limit, precision, return_variable);
                    if (result == 1) {
                        return EXIT_FAILURE;
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
                    return EXIT_FAILURE;
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
                            error(*token, "Expected an identifier.");
                            return EXIT_FAILURE;
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
                        return EXIT_FAILURE;
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
                    return EXIT_FAILURE;
                }
                func_body.pop_back();
                scope.function_bodies.push_back(func_body);
                fun_scope_names.push_back((*std::next(inner)).str());
                fun_scopes.push_back(scope);
                break;
            } else if (current.typ() == ELSE) {
                error(current, "Run-time Error: Stray else.");
                return EXIT_FAILURE;
            } else if (current.typ() == RIGHT_BRACE || current.typ() == LEFT_BRACE || current.typ() == RIGHT_PAREN || current.typ() == LEFT_PAREN) {
                error(current, "Run-time Error: Stray token.");
                return EXIT_FAILURE;
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
                        return_variable.push_back(Token(getVarVal(*its, scope, error_occurred), (*its).lines(), (*its).col(), t, (*its).actual_line(), (*its).filename()));
                    } else {
                        return_variable.push_back(*its);
                    }
                }
                return 47;
            } else if (current.typ() == DISPOSE) {
                if ((*std::next(inner)).typ() != LEFT_PAREN) {
                    error(*std::next(inner), "Run-time Error: Incorrect formatting of call.");
                    return EXIT_FAILURE;
                }
                bool eroc = false;
                auto call_params = findParams(stmt, inner, COMMA, scope.names, eroc);
                if (eroc) {
                    return EXIT_FAILURE;
                }
                if (call_params.size() != 1) {
                    error(current, "Run-time Error: Expected 1 parameter. Received " + std::to_string(call_params.size()) + ".");
                    return EXIT_FAILURE;
                } else if (call_params[0].size() != 1 || call_params[0][0].typ() != IDENTIFIER) {
                    error(current, "Run-time Error: Expected a single identifier as a parameter. Received " + std::to_string(call_params.size()) + ".");
                    return EXIT_FAILURE;
                }
                for (int i = 0; i < scope.names.size(); i++) {
                    if (scope.names[i] == call_params[0][0].str()) {
                        scope.names.erase(scope.names.begin()+i);
                        scope.values.erase(scope.values.begin()+i);
                        i--;
                    }
                }
                for (int i = 0; i < scope.immutables.size(); i++) {
                    if (scope.immutables[i] == call_params[0][0].str()) {
                        scope.immutables.erase(scope.immutables.begin()+i);
                        i--;
                    }
                }
                for (int i = 0; i < scope.function_names.size(); i++) {
                    if (scope.function_names[i] == call_params[0][0].str()) {
                        scope.function_names.erase(scope.function_names.begin()+i);
                        scope.function_params.erase(scope.function_params.begin()+i);
                        scope.function_bodies.erase(scope.function_bodies.begin()+i);
                        i--;
                    }
                }
                break;
            }
        }
    }
    return EXIT_SUCCESS;
}