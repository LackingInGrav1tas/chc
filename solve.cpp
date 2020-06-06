#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <ctime>
#include <winuser.h>
#include <winbase.h>
#include "header.hpp"

std::string solve(std::vector<Token> tokens, std::vector<std::string> names, std::vector<std::string> values, bool *error_occurred) {
    std::vector<Token> shunted = destackify( shunting_yard_algorithm( stackify(tokens) ) );//destackify( shunting_yard_algorithm( stackify(tokens) ) );
    if (shunted.size() == 1) {
        if (shunted.back().typ() == NUMBER || shunted.back().typ() == STRING) {
            if (shunted.back().typ() == NUMBER) {
                return shunted.back().str();
            } else {
                return shunted.back().str();
            }
        } else {
            return getVarVal(shunted.back(), names, values, error_occurred);
        }
    } else {
        //if shunted has more than one argument
        if (shunted.front().typ() == STRING || getVarVal(shunted.front(), names, values, error_occurred).at(0) == '"' || getVarVal(shunted.front(), names, values, error_occurred) == "\n") {
            //it is a string variable
            std::string combined;
            for (auto current_token = shunted.begin(); current_token != shunted.end(); current_token++) {
                Token ct = *current_token;
                if (ct.typ() == NUMBER) {//supporting adding numbers
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
        } else {//it is a number variable
            while (shunted.size() > 1) {
                int i = 0;
                bool con = true;
                std::vector<Type> ops = { MINUS, PLUS, SLASH, STAR };
                for (i; i < shunted.size(); i++) {//finding an operator
                    if (in(shunted[i].typ(), ops)) {
                        break;
                    } else if (shunted[i].typ() == STRING) {
                        error(shunted[i], R"(Run-time Error: String")" + shunted[i].str() + R"("attempted to be added to a number.)");
                        *error_occurred = true;
                        con = false;
                        break;
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
                            //std::cout << "literal" << std::endl;
                            a = std::stod(shunted[i-1].str());
                        }                    
                        if (shunted[i-2].typ() == IDENTIFIER) {
                            std::string gv = getVarVal(shunted[i-2], names, values, error_occurred);
                            //std::cout << "identifier -> " << gv << " .str() -> " << shunted[i-2].str() << " std::stod -> " << std::stod(gv) << std::endl;
                            b = std::stod(gv);
                        } else {
                            //std::cout << "literal" << std::endl;
                            b = std::stod(shunted[i-2].str());
                        }
                        double c = a-b;
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
                            a = std::stod(shunted[i-1].str());
                        }                    
                        if (shunted[i-2].typ() == IDENTIFIER) {
                            std::string gv = getVarVal(shunted[i-2], names, values, error_occurred);
                            //std::cout << "identifier -> " << gv << " .str() -> " << shunted[i-2].str() << " std::stod -> " << std::stod(gv) << std::endl;
                            b = std::stod(gv);
                        } else {
                            //std::cout << "literal" << std::endl;
                            b = std::stod(shunted[i-2].str());
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
                            a = std::stod(shunted[i-1].str());
                        }                    
                        if (shunted[i-2].typ() == IDENTIFIER) {
                            std::string gv = getVarVal(shunted[i-2], names, values, error_occurred);
                            //std::cout << "identifier -> " << gv << " .str() -> " << shunted[i-2].str() << " std::stod -> " << std::stod(gv) << std::endl;
                            b = std::stod(gv);
                        } else {
                            //std::cout << "literal" << std::endl;
                            b = std::stod(shunted[i-2].str());
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
                            a = std::stod(shunted[i-1].str());
                        }                    
                        if (shunted[i-2].typ() == IDENTIFIER) {
                            std::string gv = getVarVal(shunted[i-2], names, values, error_occurred);
                            //std::cout << "identifier -> " << gv << " .str() -> " << shunted[i-2].str() << " std::stod -> " << std::stod(gv) << std::endl;
                            b = std::stod(gv);
                        } else {
                            //std::cout << "literal" << std::endl;
                            b = std::stod(shunted[i-2].str());
                        }
                        double c = a/b;
                        //std::cout << a << "/" << b << "=" << c << "\n";
                        //std::cout << shunted[i-1].str() << " / " << shunted[i-2].str() << " = " << c << "\n";
                        shunted.erase((shunted.begin()+i-2), shunted.begin()+(i+1));
                        Token fnd(std::to_string(c), 0, 0, NUMBER, "");
                        shunted.insert(shunted.begin(), fnd);
                    }
                }
            }
            return shunted.back().str();
        }
    }
}
/*
int main() {/*
    Token a("55", 0, 0, NUMBER, "");
    Token b("-", 0, 0, MINUS, "");
    Token c("identifier", 0, 0, IDENTIFIER, "");
    std::vector<std::string> n = { "identifier" };
    std::vector<std::string> v = { "15" };
    Token d("*", 0, 0, STAR, "");
    Token e("87", 0, 0, NUMBER, "");
    
    Token a(R"("This ")", 0, 0, STRING, "");
    Token b("+", 0, 0, PLUS, "");
    Token c("identifier", 0, 0, IDENTIFIER, "");
    std::vector<std::string> n = { "identifier" };
    std::vector<std::string> v = { R"("is a ")" };
    Token d("+", 0, 0, PLUS, "");
    Token e(R"("sentence.")", 0, 0, STRING, "");
    std::vector<Token> all = { a, b, c, d, e };
    bool okey = true;
    std::cout << solve(all, n, v, &okey) << std::endl;
}*/