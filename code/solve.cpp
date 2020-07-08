#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <ctime>
#include <winuser.h>
#include <winbase.h>
#include "header.hpp"
#include <assert.h>
#include <sstream>
#include <iomanip>

std::string solve(std::vector<Token> tokens, Scope scope, bool *error_occurred, int precision) {
    bool err = false;
    if (getVarVal(tokens.front(), scope, &err).at(0) == '"') {
        std::string combined = "";
        for (auto token = tokens.begin(); token < tokens.end(); token++) {
            if ((*token).syhtyp() == TERMINAL) {
                std::string gvv = getVarVal(*token, scope, error_occurred);
                if (gvv.at(0) == '"') {
                    combined += gvv.substr(1, gvv.length()-2);
                } else {
                    combined += gvv;
                }
            }
        }
        return '"' + combined + '"';
    }
    std::vector<Token> shunted = destackify( shunting_yard_algorithm( stackify(tokens) ) );//destackify( shunting_yard_algorithm( stackify(tokens) ) );
    if (shunted.size() == 1) {
        return getVarVal(shunted.back(), scope, error_occurred);
    } else {      
        //if shunted has more than one argument
        if (shunted.front().typ() == STRING || getVarVal(shunted.front(), scope, error_occurred).at(0) == '"' || getVarVal(shunted.front(), scope, error_occurred) == "\n") {
            //it is a string variable.
            std::string combined;
            for (auto current_token = shunted.begin(); current_token != shunted.end(); current_token++) {
                Token ct = *current_token;
                if (ct.typ() == NUMBER || ct.typ() == TTRUE || ct.typ() == TFALSE) {//supporting adding numbers
                    combined += ct.str();
                } else if (in(ct.str(), scope.names) || ct.str().at(0) == '@') {//if it's a macro or identifier
                    std::string val = getVarVal(ct, scope, error_occurred);
                    if (error_occurred) {
                        return "";
                    }
                    if (val.at(0) == '"') {
                        combined += val.substr(1, val.length()-2);
                    } else {
                        combined += val;
                    }
                } else if (ct.str().at(0) == '"') {
                    combined += ct.str().substr(1, ct.str().length()-2);
                } else if (ct.typ() != PLUS && ct.typ() != MINUS && ct.typ() != STAR && ct.typ() != SLASH) {//it's a string identifier
                    combined += getVarVal(ct, scope, error_occurred).substr(1, getVarVal(ct, scope, error_occurred).length()-2);
                }
            }
            return '"' + combined + '"';
        } else {//it is a number variable. Boolean is co-opted
            try {
                while (shunted.size() > 1) {
                    int i = 0;
                    bool con = true;
                    std::vector<Type> ops = { MINUS, PLUS, SLASH, STAR };
                    for (i; i < shunted.size(); i++) {//finding an operator
                        if (in(shunted[i].typ(), ops)) {
                            break;
                        } else if (shunted[i].typ() == STRING) {
                            error(shunted[i], R"(Run-time Error: String )" + shunted[i].str() + R"( attempted to be added to a number.)");
                            *error_occurred = true;
                            con = false;
                            return "";
                        }
                    }
                    if (con) {
                        if (shunted[i].typ() == MINUS) {
                            double a = 2;
                            double b = 2;
                            if (shunted[i-1].typ() == IDENTIFIER) {
                                std::string gv = getVarVal(shunted[i-1], scope, error_occurred);
                                a = std::stod(gv);
                            } else {
                                if (shunted[i-1].typ() == TTRUE) {
                                    a = 1.0;
                                } else if (shunted[i-1].typ() == TFALSE) {
                                    a = 0;
                                } else {
                                    a = std::stod(shunted[i-1].str());
                                }
                            }                    
                            if (shunted[i-2].typ() == IDENTIFIER) {
                                std::string gv = getVarVal(shunted[i-2], scope, error_occurred);
                                b = std::stod(gv);
                            } else {
                                if (shunted[i-2].typ() == TTRUE) {
                                    b = 1.0;
                                } else if (shunted[i-2].typ() == TFALSE) {
                                    b = 0;
                                } else {
                                    b = std::stod(shunted[i-2].str());
                                }
                            }
                            double c = b-a;
                            std::string shortened = to_string_with_precision(c, precision);
                            shorten(shortened);
                            shunted.erase((shunted.begin()+i-2), shunted.begin()+(i+1));
                            shunted.insert(shunted.begin(), Token(shortened, 0, 0, NUMBER, ""));
                        }
                        if (shunted[i].typ() == PLUS) {
                            double a = 2;
                            double b = 2;
                            if (shunted[i-1].typ() == IDENTIFIER) {
                                std::string gv = getVarVal(shunted[i-1], scope, error_occurred);
                                a = std::stod(gv);
                            } else {
                                if (shunted[i-1].typ() == TTRUE) {
                                    a = 1.0;
                                } else if (shunted[i-1].typ() == TFALSE) {
                                    a = 0;
                                } else {
                                    a = std::stod(shunted[i-1].str());
                                }
                            }                    
                            if (shunted[i-2].typ() == IDENTIFIER) {
                                std::string gv = getVarVal(shunted[i-2], scope, error_occurred);
                                b = std::stod(gv);
                            } else {
                                if (shunted[i-2].typ() == TTRUE) {
                                    b = 1.0;
                                } else if (shunted[i-2].typ() == TFALSE) {
                                    b = 0;
                                } else {
                                    b = std::stod(shunted[i-2].str());
                                }
                            }
                            double c = a+b;
                            std::string shortened = to_string_with_precision(c, precision);
                            while (shortened.back() == '0') {
                                shortened.pop_back();
                            }
                            if (shortened.back() == '.')
                                shortened.pop_back();
                            shunted.erase((shunted.begin()+i-2), shunted.begin()+(i+1));
                            Token fnd(shortened, 0, 0, NUMBER, "");
                            shunted.insert(shunted.begin(), fnd);
                        }
                        if (shunted[i].typ() == STAR) {
                            double a = 2;
                            double b = 2;
                            if (shunted[i-1].typ() == IDENTIFIER) {
                                std::string gv = getVarVal(shunted[i-1], scope, error_occurred);
                                a = std::stod(gv);
                            } else {
                                if (shunted[i-1].typ() == TTRUE) {
                                    a = 1.0;
                                } else if (shunted[i-1].typ() == TFALSE) {
                                    a = 0;
                                } else {
                                    a = std::stod(shunted[i-1].str());
                                }
                            }                    
                            if (shunted[i-2].typ() == IDENTIFIER) {
                                std::string gv = getVarVal(shunted[i-2], scope, error_occurred);
                                b = std::stod(gv);
                            } else {
                                if (shunted[i-2].typ() == TTRUE) {
                                    b = 1.0;
                                } else if (shunted[i-2].typ() == TFALSE) {
                                    b = 0;
                                } else {
                                    b = std::stod(shunted[i-2].str());
                                }
                            }
                            double c = a*b;
                            std::string shortened = to_string_with_precision(c, precision);
                            while (shortened.back() == '0') {
                                shortened.pop_back();
                            }
                            if (shortened.back() == '.')
                                shortened.pop_back();
                            shunted.erase((shunted.begin()+i-2), shunted.begin()+(i+1));
                            Token fnd(shortened, 0, 0, NUMBER, "");
                            shunted.insert(shunted.begin(), fnd);
                        }
                        if (shunted[i].typ() == SLASH) {
                            double a = 2;
                            double b = 2;
                            if (shunted[i-1].typ() == IDENTIFIER) {
                                std::string gv = getVarVal(shunted[i-1], scope, error_occurred);
                                a = std::stod(gv);
                            } else {
                                if (shunted[i-1].typ() == TTRUE) {
                                    a = 1.0;
                                } else if (shunted[i-1].typ() == TFALSE) {
                                    a = 0;
                                } else {
                                    a = std::stod(shunted[i-1].str());
                                }
                            }                    
                            if (shunted[i-2].typ() == IDENTIFIER) {
                                std::string gv = getVarVal(shunted[i-2], scope, error_occurred);
                                b = std::stod(gv);
                            } else {
                                if (shunted[i-2].typ() == TTRUE) {
                                    b = 1.0;
                                } else if (shunted[i-2].typ() == TFALSE) {
                                    b = 0;
                                } else {
                                    b = std::stod(shunted[i-2].str());
                                }
                            }
                            double c = b/a;
                            std::string shortened = to_string_with_precision(c, precision);
                            while (shortened.back() == '0') {
                                shortened.pop_back();
                            }
                            if (shortened.back() == '.')
                                shortened.pop_back();
                            shunted.erase((shunted.begin()+i-2), shunted.begin()+(i+1));
                            Token fnd(shortened, 0, 0, NUMBER, "");
                            shunted.insert(shunted.begin(), fnd);
                        }
                    }
                }
                return shunted.back().str();
            } catch (...) {
                //it is a string variable.
                std::string combined;
                for (auto current_token = shunted.begin(); current_token != shunted.end(); current_token++) {
                    Token ct = *current_token;
                    if (ct.typ() == NUMBER || ct.typ() == TTRUE || ct.typ() == TFALSE) {//supporting adding numbers
                        combined += ct.str();
                    } else if (in(ct.str(), scope.names) || ct.str().at(0) == '@') {//if it's an or macro
                        std::string val = getVarVal(ct, scope, error_occurred);
                        if (val.at(0) == '"') {
                            combined += val.substr(1, val.length()-2);
                        } else {
                            combined += val;
                        }
                    } else if (ct.str().at(0) == '"') {
                        combined += ct.str().substr(1, ct.str().length()-2);
                    } else if (ct.typ() != PLUS && ct.typ() != MINUS && ct.typ() != STAR && ct.typ() != SLASH) {//it's a string literal
                        combined += getVarVal(ct, scope, error_occurred).substr(1, getVarVal(ct, scope, error_occurred).length()-2);
                    }
                }
                return '"' + combined + '"';
            }
        }
    }
}

bool boolsolve(std::vector<Token> tokens, Scope scope, int limit, int precision, std::vector<Scope> &scopes, std::vector<std::string> &scope_indices, bool *error_occurred) {
    std::vector<Type> native_functions = { TOKEN_INPUT, ASSERT, WRITETO, LENGTH, HASH, RPRINT, FPRINT, RFPRINT, THROW, EVAL, RAND, AT, SET_SCOPE, SAVE_SCOPE, STR, TOKEN_INT, IS_STRING, IS_NUMBER, IS_BOOL, SOLVE };
    std::vector<Token> stmt = tokens;
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
            } else if ((*token).typ() == RPRINT) {
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
                stmt.insert(stmt.begin()+ct, Token("_void_func_holder", (*token).lines(), (*token).col(), _VOID_FUNC_HOLDER, (*token).actual_line(), (*token).filename()));
            } else if ((*token).typ() == FPRINT) {
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
                stmt.insert(stmt.begin()+ct, Token("_void_func_holder", (*token).lines(), (*token).col(), _VOID_FUNC_HOLDER, (*token).actual_line(), (*token).filename()));
            } else if ((*token).typ() == RFPRINT) {
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
                stmt.insert(stmt.begin()+ct, Token("_void_func_holder", (*token).lines(), (*token).col(), _VOID_FUNC_HOLDER, (*token).actual_line(), (*token).filename()));
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
    
    if (stmt.size() == 1) {
        if (getVarVal(stmt[0], scope, error_occurred) == "true") {
            return true;
        } else if (getVarVal(stmt[0], scope, error_occurred) == "false") {
            return false;
        } else if (getVarVal(stmt[0], scope, error_occurred) == "0") {
            return false;
        } else if (getVarVal(stmt[0], scope, error_occurred) != "" && getVarVal(stmt[0], scope, error_occurred) != R"("")") {
            return true;
        } else {
            return false;
        }
    } else {
        std::vector<bool> final;
        std::vector<Token> ops;
        std::vector<std::vector<Token>> segments;
        std::vector<Token> current_segment;
        for (auto token = stmt.begin(); token != stmt.end(); token++) {
            if ((*token).typ() == AND || (*token).typ() == OR) {
                ops.push_back(*token);
                segments.push_back(current_segment);
                current_segment.clear();
            } else {
                current_segment.push_back((*token));
            }
        }
        segments.push_back(current_segment);
        current_segment.clear();
        //split each segment by their operator into a 3 vectors, (lhs, op, rhs)
        for (auto segment = segments.begin(); segment != segments.end(); segment++) {
            int times = 0;
            Token op;
            std::vector<Token> lhs, rhs;
            for (auto token = (*segment).begin(); token != (*segment).end(); token++) {
                if (isOp((*token))) {
                    if (times >= 1) {
                        error((*token), "Run-time Error: Too many operators.");
                        *error_occurred = true;
                        return NULL;
                    } else {
                        times++;
                        op = (*token);
                    }
                } else if (times == 0) {
                    lhs.push_back((*token));
                } else if (times == 1) {
                    rhs.push_back((*token));
                } else {
                    std::cout << "Something is wrong with C++.\n";
                }
            }
            if (rhs.empty()) {
                if (lhs[0].typ() == TFALSE || getVarVal(lhs[0], scope, error_occurred) == "false" || getVarVal(lhs[0], scope, error_occurred) == "0" || getVarVal(lhs[0], scope, error_occurred) == R"("")" || getVarVal(lhs[0], scope, error_occurred) == "") {
                    final.push_back(false);
                } else {
                    final.push_back(true);
                }
            } else {
                bool ev = false;
                final.push_back( evaluate( lhs[0], op, rhs[0], scope, &ev ) );
                if (ev)
                    return false;
            }
        }
        while (final.size() > 1) {
            bool left = final[0];
            bool right = final[1];
            final.erase(final.begin());
            final.erase(final.begin());
            Token oper = ops[0];
            ops.erase(ops.begin());
            if (oper.typ() == AND) {
                final.insert(final.begin(), left && right);
            } else if (oper.typ() == OR) {
                final.insert(final.begin(), left || right);
            } else {
                std::cout << "OH GOD OH NO PANIC PANIC PANIC!\nsomething has gone very wrong.";
                *error_occurred = true;
            }
        }
        assert(ops.empty() && final.size() == 1);
        return final[0];
    }
}