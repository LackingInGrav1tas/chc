#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <ctime>
#include <winuser.h>
#include <winbase.h>
#include <windows.h>
#include "header.hpp"

int runtime(std::vector<std::vector<Token>> statements, std::vector<std::string> &names, std::vector<std::string> &values, std::vector<std::string> &immutables, bool *error_occurred, int limit, std::vector<std::vector<std::vector<Token>>> function_bodies, std::vector<std::string> function_names, std::vector<std::string> aware_functions, std::vector<std::vector<std::string>> function_params, std::vector<Token> &return_variable) {
    for (auto outer = statements.begin(); outer < statements.end(); std::advance(outer, 1)) {
        std::vector<Token> stmt = *outer;
        int size = 0;
        int index = 0;
        for (auto token = stmt.begin(); token < stmt.end(); token++) {
            if (in((*token).str(), function_names)) {
                Token token_copy((*token).str(), (*token).lines(), (*token).col(), (*token).typ(), (*token).actual_line());
                auto call_params = findParams(stmt, token, COMMA);
                if (call_params.empty()) {
                    //std::cout << "IN" << std::endl;
                    if ((*std::next(token)).typ() != LEFT_PAREN) {
                        error(*std::next(token), "Run-time Error: Expected function parameters.");
                        return 1;
                    }
                    //std::cout << "finding params" << std::endl;
                    int n = std::next(token) - stmt.begin();
                    auto nd = std::next(token);
                    int nested = 0;
                    while (true) {
                        nd++;
                        //std::cout << (*nd).str() << " ";
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
                    nd--;
                    std::vector<Token> new_params(std::next(token), nd);
                    //std::cout << "\nfind in v" << std::endl;
                    std::pair<bool, int> found = findInV(function_names, (*token).str());
                    std::vector<std::vector<Token>> to_execute;
                    //std::cout << "found.first: " << found.first << std::endl;
                    //std::cout << "found.second: " << found.second << std::endl;
                    if (found.first) {
                        //std::cout << "fn[f.s]" << function_names[found.second] << std::endl;
                        //std::cout << "token.s" << (*token).str() << std::endl;
                        //std::cout << "fp[f.s].s" << function_params[found.second].size() << std::endl;
                        //std::cout << "np.s" << new_params.size() << std::endl;
                        if (function_params[found.second].size() == new_params.size()) {
                            //std::cout << "placement funcs" << std::endl;
                            for (int i = 0; i < new_params.size(); i++) {
                                std::vector<Token> to_send = { Token(function_params[found.second][i], 0, 0, IDENTIFIER, ""), Token("=", 0, 0, EQUAL, ""), Token(new_params[i].str(), 0, 0, IDENTIFIER, ""), Token(";", 0, 0, SEMICOLON, "") };
                                to_execute.push_back(to_send);
                            }
                        }
                        //std::cout << "rest of func" << std::endl;
                        for (auto b = function_bodies[found.second].begin(); b < function_bodies[found.second].end(); b++) {
                            to_execute.push_back(*b);
                        }
                        //std::cout << "rt" << std::endl;
                        std::vector<Token> rv;
                        int result;
                        if (!in((*token).str(), aware_functions)) {
                            //std::cout << "!in((*token).str(), aware_functions)" << std::endl;
                            std::vector<std::string> n, v, i, fn, aw;
                            std::vector<std::vector<std::vector<Token>>> f;
                            std::vector<std::vector<std::string>> fp;
                            result = runtime(to_execute, n, v, i, error_occurred, limit, f, fn, aw, fp, rv);
                        } else {
                            //std::cout << "f\n";
                            result = runtime(to_execute, names, values, immutables, error_occurred, limit, function_bodies, function_names, aware_functions, function_params, rv);
                        }
                        if (result == 1) {
                            return 1;
                        }
                        //std::cout << "RV: " << rv << std::endl;
                        Token replacement;
                        if (rv.empty()) {
                            replacement = Token("_void_func_holder", -47, -47, _VOID_FUNC_HOLDER, "_void_func_holder");
                        }// else if (rv == "true") {
                        //    replacement = Token("true", -47, -47, TTRUE, "");
                        //} else if (rv == "false") {
                        //    replacement = Token("false", -47, -47, TFALSE, "");
                        //} else if (rv.at(0) == '"') {
                        //    replacement = Token(rv, -47, -47, STRING, "");
                        //} else {
                        //    replacement = Token(rv, -47, -47, NUMBER, "");
                        //}
                        //std::cout << "stmt.s: " << stmt.size() << std::endl;
                        for (auto ato = stmt.begin(); ato < stmt.end(); ato++) {
                            //std::cout << (*ato).str() << " ";
                        }
                        //std::cout << "\n";
                        /*std::vector<Token> ers(std::next(token), std::next(std::next(nd)));
                        std::cout << "\nerasing: ";
                        for (auto oop = ers.begin(); oop < ers.end(); oop++) {
                            std::cout << (*oop).str() << " ";
                        }
                        std::cout << std::endl;*/
                        stmt.erase(std::next(token), std::next(std::next(nd)));
                        for (auto ato = stmt.begin(); ato < stmt.end(); ato++) {
                            //std::cout << (*ato).str() << " ";
                        }
                        //std::cout << "replacing: " << stmt[index].str() << std::endl;
                        stmt[index] = replacement;
                        //std::cout << "\nindex: " << index << std::endl;//stmt.s: " << stmt.size() << std::endl;
                        for (auto ato = stmt.begin(); ato < stmt.end(); ato++) {
                            //std::cout << (*ato).str() << " ";
                        }
                        //std::cout << "\n";
                    } else {
                        error((*token), "Run-time Error: Call of a non-existent function.");
                        return 1;
                    }
                } else {
                    std::pair<bool, int> found = findInV(function_names, (*token).str());
                    if (found.first) {
                        if (call_params.size() != function_params[found.second].size()) {
                            std::string msg = "Run-time Error: Received " + std::to_string(call_params.size()) + " params but expected " + std::to_string(function_params[found.second].size());
                            error(*token, msg);
                            return 1;
                        }
                        //use the technique from EQUAL to descern the value of the things, then add them to the environment upon executing
                        std::vector<std::string> n, v, im, fn, aw;
                        std::vector<std::vector<std::vector<Token>>> f;
                        std::vector<std::vector<std::string>> fp;
                        std::string rv;
                        if (in((*token).str(), aware_functions)) {
                            n = names;
                            v = values;
                            im = immutables;
                            fn = function_names;
                            aw = aware_functions;
                            f = function_bodies;
                            fp = function_params;
                        }
                        for (int i = 0; i < call_params.size(); i++) {
                            n.push_back(function_params[found.second][i]);
                            if (in(function_params[found.second][i], immutables))
                                im.push_back(function_params[found.second][i]);
                            bool err = false;
                            v.push_back(solve(call_params[i], names, values, &err));
                            if (err) {
                                error(*token, "Run-time Error: Evaluation Error.");
                                return 1;
                            }
                        }
                        std::vector<Token> return_val;
                        bool er = false;
                        int result = runtime(function_bodies[found.second], n, v, im, &er, limit, f, fn, aw, fp, return_val);
                        if (result == 1) {
                            return result;
                        }
                        int ct = token - stmt.begin();
                        int nested = 0;
                        //std::cout << "\nbefore while" << std::endl;
                        while (true) {
                            //std::cout << "ct: " << ct << "\nnested: " << nested << "\ncurrent: " << stmt[ct].str() << std::endl;
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
                        //std::cout << std::endl;
                        for (int b = 0; b < 0; b++) {//stmt.size()
                            std::cout << stmt[b].str() << " ";
                        }
                        //std::cout << std::endl;
                        //std::cout << "after while" << std::endl;
                        //std::cout << "\nstmt.s: " << stmt.size() << std::endl;
                    }
                }
            }
            index++;
        }
        for (auto inner = stmt.begin(); inner < stmt.end(); inner++) {
            size++;
            Token current = *inner;
            if (current.typ() == EQUAL) {//setting variable values
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
                                return 1;
                            }
                        }
                    }
                    n++;
                    if (stmt[n].typ() != SEMICOLON) {
                        error(current, "Warning: It's prudent to postfix the statement with a semicolon.");
                    };
                    std::vector<Token> segmented(std::next(inner), nd);
                    /*bool err = false;
                    std::string solved = solve(segmented, names, values, &err);
                    if (err) {
                        error(current, "Run-time Error: Evauation Error");
                        return 1;
                    }
                    if (solved.at(0) == '"') {
                        solved = solved.substr(1, solved.length()-2);
                    }*/
                    std::string solved;
                    for (auto ato = segmented.begin(); ato < segmented.end(); ato++) {
                        if ((*ato).syhtyp() == TERMINAL) {
                            std::string currentString = getVarVal(*ato, names, values, error_occurred);
                            if (currentString.at(0) == '"') {
                                currentString = currentString.substr(1, currentString.length()-2);
                            }
                            solved += currentString;
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
                    int result = runtime(whilecontents, names, values, immutables, error_occurred, limit, function_bodies, function_names, aware_functions, function_params, return_variable);
                    if (result == 1) {
                        return 1;
                    } else if (result == 47) {
                        break;
                    } else if (result == 44) {
                        break;
                    }
                } while (boolsolve(params, names, values, error_occurred));
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
                    std::string solved = solve(segmented, names, values, &err);
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

                    if (boolsolve(params, names, values, error_occurred)) {
                        int result = runtime(ifcontents, names, values, immutables, error_occurred, limit, function_bodies, function_names, aware_functions, function_params, return_variable);
                        if (result == 1) {
                            return 1;
                        } else if (result == 47) {
                            return 47;
                        }
                    } else {
                        if (!elsecontents.empty()) {
                            int result = runtime(elsecontents, names, values, immutables, error_occurred, limit, function_bodies, function_names, aware_functions, function_params, return_variable);
                            if (result == 1) {
                                return 1;
                            } else if (result == 47) {
                                return 47;
                            }
                        }
                    }
                    break;
                }
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
                //std::cout << "passed ifs" << std::endl;
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
                
                //std::cout << "passed for" << std::endl;

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
                //std::cout << "after while loop" << std::endl;
                if (whilecontents.back()[1].typ() != SEMICOLON) {
                    error(whilecontents.back()[0], "Run-time Error: Expected a ; at the end of the while statement.");
                    return 1;
                }
                whilecontents.pop_back();
                int stop = 0;
                //std::cout << "|\nboolsolve: " << boolsolve(params, names, values, error_occurred) << std::endl;
                while (boolsolve(params, names, values, error_occurred)) {
                    stop++;
                    //std::cout << "loop " << stop << "\n";
                    if (stop == limit) {
                        error(current, "Terminate after control finds repeating while loop, limit: " + std::to_string(limit));
                        return 1;
                    }
                    int result = runtime(whilecontents, names, values, immutables, error_occurred, limit, function_bodies, function_names, aware_functions, function_params, return_variable);
                    if (result == 1) {
                        return 1;
                    } else if (result == 47) {
                        break;
                    } else if (result == 44) {
                        break;
                    }
                }
                break;
                //std::cout << "end of while.";
            } else if (current.typ() == FUN) {
                if ((*std::next(inner)).typ() != IDENTIFIER) {
                    error(*std::next(inner), "Run-time Error: Inadequite function name.");
                    return 1;
                } else {
                    function_names.push_back((*std::next(inner)).str());
                }
                //std::cout << "prevstr: " << (*std::prev(inner)).str() << "|" << (*std::prev(inner)).typ() << std::endl;
                if ((*std::prev(inner)).typ() == AWARE) {
                    aware_functions.push_back((*std::next(inner)).str());
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
                            error(current, "Run-time Error: CHC is a poorly written language, therefore it only takes identifiers and commas in the function header.");
                            return 1;
                        }
                    }
                    if ((*token).typ() == LEFT_PAREN) {
                        ps++;
                    } else if ((*token).typ() == RIGHT_PAREN) {
                        ps--;
                    }
                }
                function_params.push_back(current_params);
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
                        } else if (cline.back().typ() == LEFT_BRACE) {
                            nested++;
                        }
                    }
                }
                //std::cout << "after while loop" << std::endl;
                if (func_body.back()[1].typ() != SEMICOLON) {
                    error(func_body.back()[1], "Run-time Error: Expected a ; at the end of the function declaration.");
                    return 1;
                }
                func_body.pop_back();
                function_bodies.push_back(func_body);
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
                        if (getVarVal(*its, names, values, error_occurred).at(0) == '"') {
                            t = STRING;
                        } else if (getVarVal(*its, names, values, error_occurred) == "true") {
                            t = TTRUE;
                        } else if (getVarVal(*its, names, values, error_occurred) == "false") {
                            t = TFALSE;
                        } else {
                            t = NUMBER;
                        }
                        return_variable.push_back(Token(getVarVal(*its, names, values, error_occurred), (*its).lines(), (*its).col(), t, (*its).actual_line()));
                    } else {
                        return_variable.push_back(*its);
                    }
                }
                return 0;
            }
            //std::cout << "\n*error_occurred\n" << *error_occurred;
        }
    }
    return 0;
}