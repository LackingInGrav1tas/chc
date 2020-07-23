#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <ctime>
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
            } else if ((*token).typ() == GREATER || (*token).typ() == GREATER_EQUAL || (*token).typ() == LESS || (*token).typ() == LESS_EQUAL || (*token).typ() == EXC_EQUAL || (*token).typ() == EXC || (*token).typ() == EQUAL_EQUAL || (*token).typ() == EQUAL) {
                error(*token, "Run-time Error: Invalid operator.");
                *error_occurred = true;
                return "";
            }
        }
        return '"' + combined + '"';
    } else if (tokens.size() == 1)
        return getVarVal(tokens.back(), scope, error_occurred);
    std::vector<Token> shunted = destackify( shunting_yard_algorithm( stackify(tokens) ) );//destackify( shunting_yard_algorithm( stackify(tokens) ) );    
    //if shunted has more than one argument
    if (shunted.front().typ() == STRING || getVarVal(shunted.front(), scope, error_occurred).at(0) == '"' || getVarVal(shunted.front(), scope, error_occurred) == "\n") {
        //it is a string variable.
        std::string combined;
        for (auto current_token = shunted.begin(); current_token != shunted.end(); current_token++) {
            if ((*current_token).typ() == GREATER || (*current_token).typ() == GREATER_EQUAL || (*current_token).typ() == LESS || (*current_token).typ() == LESS_EQUAL || (*current_token).typ() == EXC_EQUAL || (*current_token).typ() == EXC || (*current_token).typ() == EQUAL_EQUAL || (*current_token).typ() == EQUAL) {
                error(*current_token, "Run-time Error: Invalid operator.");
                *error_occurred = true;
                return "";
            }
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
                std::vector<Type> ops = { MINUS, PLUS, SLASH, STAR };
                for (i; i < shunted.size(); i++) {//finding an operator
                    if (in(shunted[i].typ(), ops)) {
                        break;
                    } else if (shunted[i].typ() == STRING) {
                        error(shunted[i], R"(Run-time Error: String )" + shunted[i].str() + R"( attempted to be added to a number.)");
                        *error_occurred = true;
                        return "";
                    } else if (shunted[i].typ() == GREATER || shunted[i].typ() == GREATER_EQUAL || shunted[i].typ() == LESS || shunted[i].typ() == LESS_EQUAL || shunted[i].typ() == EXC_EQUAL || shunted[i].typ() == EXC || shunted[i].typ() == EQUAL_EQUAL || shunted[i].typ() == EQUAL) {
                        error(shunted[i], "Run-time Error: Invalid operator.");
                        *error_occurred = true;
                        return "";
                    }
                }
                if (shunted[i].typ() == MINUS) {
                    double a = 2;
                    double b = 2;
                    if (shunted[i-1].typ() == IDENTIFIER) {
                        a = std::stod(getVarVal(shunted[i-1], scope, error_occurred));
                    } else {
                        if (shunted[i-1].typ() == TTRUE)
                            a = 1.0;
                        else if (shunted[i-1].typ() == TFALSE)
                            a = 0;
                        else
                            a = std::stod(shunted[i-1].str());
                    }                    
                    if (shunted[i-2].typ() == IDENTIFIER) {
                        b = std::stod(getVarVal(shunted[i-2], scope, error_occurred));
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
                } else if (shunted[i].typ() == PLUS) {
                        double a;
                        double b;
                        if (shunted[i-1].typ() == IDENTIFIER) {
                            a = std::stod(getVarVal(shunted[i-1], scope, error_occurred));
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
                            b = std::stod(getVarVal(shunted[i-2], scope, error_occurred));
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
                        shorten(shortened);
                        shunted.erase((shunted.begin()+i-2), shunted.begin()+(i+1));
                        shunted.insert(shunted.begin(), Token(shortened, 0, 0, NUMBER, ""));
                } else if (shunted[i].typ() == STAR) {
                    double a;
                    double b;
                    if (shunted[i-1].typ() == IDENTIFIER) {
                        a = std::stod(getVarVal(shunted[i-1], scope, error_occurred));
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
                        b = std::stod(getVarVal(shunted[i-2], scope, error_occurred));
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
                    shorten(shortened);
                    shunted.erase((shunted.begin()+i-2), shunted.begin()+(i+1));
                    shunted.insert(shunted.begin(), Token(shortened, 0, 0, NUMBER, ""));
                } else if (shunted[i].typ() == SLASH) {
                    double a;
                    double b;
                    if (shunted[i-1].typ() == IDENTIFIER) {
                        a = std::stod(getVarVal(shunted[i-1], scope, error_occurred));
                    } else {
                        if (shunted[i-1].typ() == TTRUE)
                            a = 1.0;
                        else if (shunted[i-1].typ() == TFALSE)
                            a = 0;
                        else
                            a = std::stod(shunted[i-1].str());
                    }                    
                    if (shunted[i-2].typ() == IDENTIFIER) {
                        b = std::stod(getVarVal(shunted[i-2], scope, error_occurred));
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
                    shorten(shortened);
                    shunted.erase((shunted.begin()+i-2), shunted.begin()+(i+1));
                    shunted.insert(shunted.begin(), Token(shortened, 0, 0, NUMBER, ""));
                }
            }
            return shunted.back().str();
        } catch (...) {
            //it is a string variable.
            std::string combined;
            for (auto current_token = shunted.begin(); current_token != shunted.end(); current_token++) {
                if ((*current_token).typ() == GREATER || (*current_token).typ() == GREATER_EQUAL || (*current_token).typ() == LESS || (*current_token).typ() == LESS_EQUAL || (*current_token).typ() == EXC_EQUAL || (*current_token).typ() == EXC || (*current_token).typ() == EQUAL_EQUAL || (*current_token).typ() == EQUAL) {
                    error(*current_token, "Run-time Error: Invalid operator.");
                    *error_occurred = true;
                    return "";
                }
                if ((*current_token).typ() == NUMBER || (*current_token).typ() == TTRUE || (*current_token).typ() == TFALSE) {//supporting adding numbers
                    combined += (*current_token).str();
                } else if (in((*current_token).str(), scope.names) || (*current_token).str().at(0) == '@') {//if it's an or macro
                    std::string val = getVarVal((*current_token), scope, error_occurred);
                    if (val.at(0) == '"') {
                        combined += val.substr(1, val.length()-2);
                    } else {
                        combined += val;
                    }
                } else if ((*current_token).str().at(0) == '"')
                    combined += (*current_token).str().substr(1, (*current_token).str().length()-2);
                else if ((*current_token).typ() != PLUS && (*current_token).typ() != MINUS && (*current_token).typ() != STAR && (*current_token).typ() != SLASH)//it's a string literal
                    combined += getVarVal((*current_token), scope, error_occurred).substr(1, getVarVal((*current_token), scope, error_occurred).length()-2);
            }
            return '"' + combined + '"';
        }
    }
}

bool boolsolve(std::vector<Token> tokens, Scope scope, int limit, int precision, std::vector<Scope> &scopes, std::vector<std::string> &scope_indices, bool *error_occurred) {
    std::vector<Token> stmt = tokens;
    int result = handle_functions(stmt, scope, limit, precision, error_occurred);
    if (result == 1)
        return 1;
    if (stmt.size() == 1) {
        if (getVarVal(stmt[0], scope, error_occurred) == "true")
            return true;
        else if (getVarVal(stmt[0], scope, error_occurred) == "false")
            return false;
        else if (getVarVal(stmt[0], scope, error_occurred) == "0")
            return false;
        else if (getVarVal(stmt[0], scope, error_occurred) != "" && getVarVal(stmt[0], scope, error_occurred) != R"("")")
            return true;
        else
            return false;
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
                } else if (times == 0) lhs.push_back((*token));
                else if (times == 1) rhs.push_back((*token));
                else
                    std::cout << "Something is wrong with C++.\n";
            }
            if (rhs.empty()) {
                if (lhs[0].typ() == TFALSE || getVarVal(lhs[0], scope, error_occurred) == "false" || getVarVal(lhs[0], scope, error_occurred) == "0" || getVarVal(lhs[0], scope, error_occurred) == R"("")" || getVarVal(lhs[0], scope, error_occurred) == "")
                    final.push_back(false);
                else 
                    final.push_back(true);
            } else {
                bool ev = false;
                final.push_back( evaluate( lhs[0], op, rhs[0], scope, &ev ) );
                if (ev) return false;
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