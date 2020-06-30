#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <ctime>
#include <winuser.h>
#include <winbase.h>
#include "header.hpp"
#include <assert.h>

std::string solve(std::vector<Token> tokens, std::vector<std::string> names, std::vector<std::string> values, bool *error_occurred) {
    std::vector<Token> shunted = destackify( shunting_yard_algorithm( stackify(tokens) ) );//destackify( shunting_yard_algorithm( stackify(tokens) ) );
    if (shunted.size() == 1) {
        return getVarVal(shunted.back(), names, values, error_occurred);
        /*
        if (shunted.back().typ() == NUMBER || shunted.back().typ() == STRING || shunted.back().typ() == TTRUE || shunted.back().typ() == TFALSE) {
            if (shunted.back().typ() == STRING) {
                return shunted.back().str();
            } else {
                return shunted.back().str();
            }
        } else {
            return getVarVal(shunted.back(), names, values, error_occurred);
        }*/
    } else {      
        //if shunted has more than one argument
        if (shunted.front().typ() == STRING || getVarVal(shunted.front(), names, values, error_occurred).at(0) == '"' || getVarVal(shunted.front(), names, values, error_occurred) == "\n") {
            //it is a string variable.
            std::string combined;
            for (auto current_token = shunted.begin(); current_token != shunted.end(); current_token++) {
                Token ct = *current_token;
                if (ct.typ() == NUMBER || ct.typ() == TTRUE || ct.typ() == TFALSE) {//supporting adding numbers
                    combined += ct.str();
                } else if (in(ct.str(), names) || ct.str().at(0) == '@') {//if it's an or macro
                    std::string val = getVarVal(ct, names, values, error_occurred);
                    if (val.at(0) == '"') {
                        combined += val.substr(1, val.length()-2);
                    } else {
                        combined += val;
                    }
                } else if (ct.str().at(0) == '"') {
                    combined += ct.str().substr(1, ct.str().length()-2);
                } else if (ct.typ() != PLUS && ct.typ() != MINUS && ct.typ() != STAR && ct.typ() != SLASH) {//it's a string literal
                    combined += getVarVal(ct, names, values, error_occurred).substr(1, getVarVal(ct, names, values, error_occurred).length()-2);
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
                                std::string gv = getVarVal(shunted[i-1], names, values, error_occurred);
                                //std::cout << "identifier -> " << gv << " .str() -> " << shunted[i-1].str() << " std::stod -> " << std::stod(gv) << std::endl;
                                a = std::stod(gv);
                            } else {
                                if (shunted[i-1].typ() == TTRUE) {
                                    a = 1.0;
                                } else if (shunted[i-1].typ() == TFALSE) {
                                    a = 0;
                                } else {
                                    a = std::stod(shunted[i-1].str());
                                }
                                //std::cout << "literal" << std::endl;
                            }                    
                            if (shunted[i-2].typ() == IDENTIFIER) {
                                std::string gv = getVarVal(shunted[i-2], names, values, error_occurred);
                                //std::cout << "identifier -> " << gv << " .str() -> " << shunted[i-2].str() << " std::stod -> " << std::stod(gv) << std::endl;
                                b = std::stod(gv);
                            } else {
                                //std::cout << "literal" << std::endl;
                                if (shunted[i-2].typ() == TTRUE) {
                                    b = 1.0;
                                } else if (shunted[i-2].typ() == TFALSE) {
                                    b = 0;
                                } else {
                                    b = std::stod(shunted[i-2].str());
                                }
                            }
                            double c = b-a;
                            //std::cout << a << "-" << b << "=" << c << "\n";
                            //std::cout << shunted[i-1].str() << " - " << shunted[i-2].str() << " = " << c << "\n";
                            shunted.erase((shunted.begin()+i-2), shunted.begin()+(i+1));
                            Token fnd(std::to_string(c), 0, 0, NUMBER, "");
                            shunted.insert(shunted.begin(), fnd);
                        }
                        if (shunted[i].typ() == PLUS) {
                            double a = 2;
                            double b = 2;
                            if (shunted[i-1].typ() == IDENTIFIER) {
                                std::string gv = getVarVal(shunted[i-1], names, values, error_occurred);
                                //std::cout << "identifier -> " << gv << " .str() -> " << shunted[i-1].str() << " std::stod -> " << std::stod(gv) << std::endl;
                                a = std::stod(gv);
                            } else {
                                //std::cout << "literal" << std::endl;
                                if (shunted[i-1].typ() == TTRUE) {
                                    a = 1.0;
                                } else if (shunted[i-1].typ() == TFALSE) {
                                    a = 0;
                                } else {
                                    a = std::stod(shunted[i-1].str());
                                }
                            }                    
                            if (shunted[i-2].typ() == IDENTIFIER) {
                                std::string gv = getVarVal(shunted[i-2], names, values, error_occurred);
                                //std::cout << "identifier -> " << gv << " .str() -> " << shunted[i-2].str() << " std::stod -> " << std::stod(gv) << std::endl;
                                b = std::stod(gv);
                            } else {
                                //std::cout << "literal" << std::endl;
                                if (shunted[i-2].typ() == TTRUE) {
                                    b = 1.0;
                                } else if (shunted[i-2].typ() == TFALSE) {
                                    b = 0;
                                } else {
                                    b = std::stod(shunted[i-2].str());
                                }
                            }
                            double c = a+b;
                            //std::cout << a << "+" << b << "=" << c << "\n";
                            //std::cout << shunted[i-1].str() << " + " << shunted[i-2].str() << " = " << c << "\n";
                            shunted.erase((shunted.begin()+i-2), shunted.begin()+(i+1));
                            Token fnd(std::to_string(c), 0, 0, NUMBER, "");
                            shunted.insert(shunted.begin(), fnd);
                        }
                        if (shunted[i].typ() == STAR) {
                            double a = 2;
                            double b = 2;
                            if (shunted[i-1].typ() == IDENTIFIER) {
                                std::string gv = getVarVal(shunted[i-1], names, values, error_occurred);
                                //std::cout << "identifier -> " << gv << " .str() -> " << shunted[i-1].str() << " std::stod -> " << std::stod(gv) << std::endl;
                                a = std::stod(gv);
                            } else {
                                //std::cout << "literal" << std::endl;
                                if (shunted[i-1].typ() == TTRUE) {
                                    a = 1.0;
                                } else if (shunted[i-1].typ() == TFALSE) {
                                    a = 0;
                                } else {
                                    a = std::stod(shunted[i-1].str());
                                }
                            }                    
                            if (shunted[i-2].typ() == IDENTIFIER) {
                                std::string gv = getVarVal(shunted[i-2], names, values, error_occurred);
                                //std::cout << "identifier -> " << gv << " .str() -> " << shunted[i-2].str() << " std::stod -> " << std::stod(gv) << std::endl;
                                b = std::stod(gv);
                            } else {
                                //std::cout << "literal" << std::endl;
                                if (shunted[i-2].typ() == TTRUE) {
                                    b = 1.0;
                                } else if (shunted[i-2].typ() == TFALSE) {
                                    b = 0;
                                } else {
                                    b = std::stod(shunted[i-2].str());
                                }
                            }
                            double c = a*b;
                            //std::cout << a << "*" << b << "=" << c << "\n";
                            //std::cout << shunted[i-1].str() << " * " << shunted[i-2].str() << " = " << c << "\n";
                            shunted.erase((shunted.begin()+i-2), shunted.begin()+(i+1));
                            Token fnd(std::to_string(c), 0, 0, NUMBER, "");
                            shunted.insert(shunted.begin(), fnd);
                        }
                        if (shunted[i].typ() == SLASH) {
                            double a = 2;
                            double b = 2;
                            if (shunted[i-1].typ() == IDENTIFIER) {
                                std::string gv = getVarVal(shunted[i-1], names, values, error_occurred);
                                //std::cout << "identifier -> " << gv << " .str() -> " << shunted[i-1].str() << " std::stod -> " << std::stod(gv) << std::endl;
                                a = std::stod(gv);
                            } else {
                                //std::cout << "literal" << std::endl;
                                if (shunted[i-1].typ() == TTRUE) {
                                    a = 1.0;
                                } else if (shunted[i-1].typ() == TFALSE) {
                                    a = 0;
                                } else {
                                    a = std::stod(shunted[i-1].str());
                                }
                            }                    
                            if (shunted[i-2].typ() == IDENTIFIER) {
                                std::string gv = getVarVal(shunted[i-2], names, values, error_occurred);
                                //std::cout << "identifier -> " << gv << " .str() -> " << shunted[i-2].str() << " std::stod -> " << std::stod(gv) << std::endl;
                                b = std::stod(gv);
                            } else {
                                //std::cout << "literal" << std::endl;
                                if (shunted[i-2].typ() == TTRUE) {
                                    b = 1.0;
                                } else if (shunted[i-2].typ() == TFALSE) {
                                    b = 0;
                                } else {
                                    b = std::stod(shunted[i-2].str());
                                }
                            }
                            double c = b/a;
                            //std::cout << a << "/" << b << "=" << c << "\n";
                            //std::cout << shunted[i-1].str() << " / " << shunted[i-2].str() << " = " << c << "\n";
                            shunted.erase((shunted.begin()+i-2), shunted.begin()+(i+1));
                            Token fnd(std::to_string(c), 0, 0, NUMBER, "");
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
                    } else if (in(ct.str(), names) || ct.str().at(0) == '@') {//if it's an or macro
                        std::string val = getVarVal(ct, names, values, error_occurred);
                        if (val.at(0) == '"') {
                            combined += val.substr(1, val.length()-2);
                        } else {
                            combined += val;
                        }
                    } else if (ct.str().at(0) == '"') {
                        combined += ct.str().substr(1, ct.str().length()-2);
                    } else if (ct.typ() != PLUS && ct.typ() != MINUS && ct.typ() != STAR && ct.typ() != SLASH) {//it's a string literal
                        combined += getVarVal(ct, names, values, error_occurred).substr(1, getVarVal(ct, names, values, error_occurred).length()-2);
                    }
                }
                return '"' + combined + '"';
            }
        }
    }
}

bool boolsolve(std::vector<Token> tokens, std::vector<std::string> names, std::vector<std::string> values, bool *error_occurred) {
    if (tokens.size() == 1) {
        if (getVarVal(tokens[0], names, values, error_occurred) == "true") {
            return true;
        } else if (getVarVal(tokens[0], names, values, error_occurred) == "false") {
            return false;
        } else if (getVarVal(tokens[0], names, values, error_occurred) == "0") {
            return false;
        } else if (getVarVal(tokens[0], names, values, error_occurred) != "" && getVarVal(tokens[0], names, values, error_occurred) != R"("")") {
            return true;
        } else {
            return false;
        }
    } else {
        std::vector<bool> final;// = { true, false };
        //Token op("||", 0, 0, OR, "");
        std::vector<Token> ops;// = { op };

        /*
        split tokens by AND and OR tokens into a new vector, segments
        split each segment by their operator into a 3 vectors, (lhs, op, rhs)
        push evaluate ( solve( lhs ), op, solve( rhs ) ) to final
        */
        
        //split tokens by AND and OR tokens into a new vector, segments
        std::vector<std::vector<Token>> segments;
        std::vector<Token> current_segment;
        for (auto token = tokens.begin(); token != tokens.end(); token++) {
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
        /*
        std::cout << "segments.size(): " << segments.size() << std::endl << "segments: [";
        for (auto i = segments.begin(); i != segments.end(); i++) {
            std::cout << (*i).back().str() << ", ";
        }
        std::cout << "]\nops.size(): " << ops.size() << std::endl << "ops: [";
        for (auto i = ops.begin(); i != ops.end(); i++) {
            std::cout << (*i).str() << ", ";
        }
        std::cout <<  "]\n\n";
        */
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
                    std::cout << "WTF!!! how could this have happened!\n";
                }
            }/*
            std::cout << "lhs.size(): " << lhs.size() << std::endl << "lhs: [";
            for (auto i = lhs.begin(); i != lhs.end(); i++) {
                std::cout << (*i).str() << ", ";
            }
            std::cout << "]\nrhs.size(): " << rhs.size() << std::endl << "rhs: [";
            for (auto i = rhs.begin(); i != rhs.end(); i++) {
                std::cout << (*i).str() << ", ";
            }
            std::cout <<  "]\nop: " << op.str() << "\n\n";
            */
            if (rhs.empty()) {
                //std::cout << "rhs.empty()\n";
                if (lhs[0].typ() == TFALSE || getVarVal(lhs[0], names, values, error_occurred) == "false" || getVarVal(lhs[0], names, values, error_occurred) == "0" || getVarVal(lhs[0], names, values, error_occurred) == R"("")" || getVarVal(lhs[0], names, values, error_occurred) == "") {
                    //std::cout << "0\n";
                    final.push_back(false);
                } else {
                    //std::cout << "1 " << lhs[0].str() << "\n";
                    final.push_back(true);
                }
            } else {
                //std::cout << evaluate( lhs[0], op, rhs[0], names, values, error_occurred ) << "\n";
                final.push_back( evaluate( lhs[0], op, rhs[0], names, values, error_occurred ) );
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
/*
int main() {
    Token a("i", 0, 0, IDENTIFIER, "");
    Token b("<", 0, 0, LESS, "");
    Token c("5", 0, 0, NUMBER, "");
    Token d("and", 0, 0, AND, "");
    Token e("v", 0, 0, IDENTIFIER, "");
    Token f("==", 0, 0, LESS, "");
    Token g("1", 0, 0, NUMBER, "");
    std::vector<std::string> n = { "i", "v" };
    std::vector<std::string> v = { "1", "1" };
    std::vector<Token> all = { a, b, c, d, e, f, g };
    bool okey = true;
    std::cout << boolsolve(all, n, v, &okey) << "\nend" << std::endl;
}*/