#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <ctime>
#include <shlobj.h>
#include "wtypes.h"
#include <winsock.h>
#include <limits>
#include <functional>
#include "header.hpp"


std::string getString(char x) { 
    std::string s(1, x); 
    return s;
}

void error(Token token, std::string message="") {
    std::cout << "\nin " << token.filename() << ":";
    std::string a = std::to_string(token.lines()) + "| ";
    std::cerr << "\n" << a << token.actual_line() << "\n";
    for (int i = 0; i < a.length()+token.col()-2; i++) {
        std::cerr << " ";
    }
    std::cerr << "^\n" << message << "\n( col: " << token.col() << "  token: " << token.str() << " )\n" << std::endl;
}

Type singleChar(char current_char) {
    if (current_char == '!') return EXC;
    else if (current_char == '+') return PLUS;
    else if (current_char == '-') return MINUS;
    else if (current_char == '/') return SLASH;
    else if (current_char == '*') return STAR;
    else if (current_char == ',') return COMMA;
    else if (current_char == ';') return SEMICOLON;
    else if (current_char == ')') return RIGHT_PAREN;
    else if (current_char == '(') return LEFT_PAREN;
    else if (current_char == '}') return RIGHT_BRACE;
    else if (current_char == '{') return LEFT_BRACE;
    else if (current_char == '<') return LESS;
    else if (current_char == '>') return GREATER;
    else if (current_char == '=') return EQUAL;
    else if (current_char == '.') return DOT;
}

Type doubleChar(std::string full) {
    if (full == "!=") return EXC_EQUAL;
    else if (full == "==") return EQUAL_EQUAL;
    else if (full == ">=") return GREATER_EQUAL;
    else if (full == "<=") return LESS_EQUAL;
    else if (full == "<-") return ARROW;
    else if (full == "--") return MINUS_MINUS;
    else if (full == "++") return PLUS_PLUS;
    else if (full == "+=") return PLUS_EQUALS;
    else if (full == "-=") return MINUS_EQUALS;
    else if (full == "*=") return STAR_EQUALS;
    else if (full == "/=") return SLASH_EQUALS;
}

Type keyword(std::string full) {
    if (full == "and") return AND;
    else if (full == "class") return CLASS;
    else if (full == "else") return ELSE;
    else if (full == "false") return TFALSE;
    else if (full == "fun") return FUN;
    else if (full == "for") return FOR;
    else if (full == "if") return IF;
    else if (full == "nil") return NIL;
    else if (full == "or") return OR;
    else if (full == "print") return PRINT;
    else if (full == "return") return RETURN;
    else if (full == "true") return TTRUE;
    else if (full == "while") return WHILE;
    else if (full == "run") return RUN;
    else if (full == "immutable") return IMMUTABLE;
    else if (full == "do") return DO;
    else if (full == "hash") return HASH;
    else if (full == "sleep") return SLEEP;
    else if (full == "break") return BREAK;
    else if (full == "aware") return AWARE;
    else if (full == "input") return TOKEN_INPUT;
    else if (full == "writeto") return WRITETO;
    else if (full == "assert") return ASSERT;
    else if (full == "length") return LENGTH;
    else if (full == "rprint") return RPRINT;
    else if (full == "fprint") return FPRINT;
    else if (full == "rfprint") return RFPRINT;
    else if (full == "throw") return THROW;
    else if (full == "eval") return EVAL;
    else if (full == "continue") return CONTINUE;
    else if (full == "rand") return RAND;
    else if (full == "at") return AT;
    else if (full == "dispose") return DISPOSE;
    else if (full == "save_scope") return SAVE_SCOPE;
    else if (full == "set_scope") return SET_SCOPE;
    else if (full == "to_str") return STR;
    else if (full == "to_num") return TOKEN_INT;
    else if (full == "is_string") return IS_STRING;
    else if (full == "is_number") return IS_NUMBER;
    else if (full == "is_bool") return IS_BOOL;
    else if (full == "solve") return SOLVE;
}

std::vector<std::string> get_lexemes(std::vector<Token> tokens) {
    std::vector<std::string> lexemes;
    for (std::vector<Token>::iterator token = tokens.begin(); token < tokens.end(); token++) {
        Token current = *token;
        lexemes.push_back(current.str());
    }
    return lexemes;
}

std::vector<std::vector<Token>> statementize(std::vector<Token> tokens) {
    std::vector<std::vector<Token>> statementalized;
    std::vector<Token> current_statement;
    for (std::vector<Token>::iterator c = tokens.begin(); c != tokens.end(); c++) {
        Token current = *c;
        current_statement.push_back(current);
        if (current.typ() == SEMICOLON || current.typ() == LEFT_BRACE || current.typ() == _EOF) {
            if (!current_statement.empty()) {
                statementalized.push_back(current_statement);
            }
            current_statement.clear();
        }
    }
    return statementalized;
}

//kinda my code
std::string getClip() {
    std::string text;
    if (OpenClipboard(NULL)) {
        HANDLE clip;
        clip = GetClipboardData(CF_TEXT);
            // lock and copy
        text = (LPSTR)GlobalLock(clip);
            // unlock 
        GlobalUnlock(clip);
        CloseClipboard();
    }
	return text;
}

//https://stackoverflow.com/a/122240/13132049
std::string IP() {
    WSADATA wsa_Data;
    int wsa_ReturnCode = WSAStartup(0x101,&wsa_Data);

    // Get the local hostname
    char szHostName[255];
    gethostname(szHostName, 255);
    struct hostent *host_entry;
    host_entry=gethostbyname(szHostName);
    char * szLocalIP;
    szLocalIP = inet_ntoa (*(struct in_addr *)*host_entry->h_addr_list);
    WSACleanup();
    return szLocalIP;
}

void shorten(std::string &str) {
    while (str.back() == '0')
        str.pop_back();
    if (str.back() == '.')
        str.pop_back();
}

std::string getVarVal(Token token, Scope scope,  bool *error_occurred) {//
    std::string target = token.str();
    if (target.length() == 0) {
        return "";
    } else {
        if (token.str().at(0) == '@') {
            const std::time_t now = std::time(nullptr);
            const std::tm calendar_time = *std::localtime( std::addressof(now));
            if (target == "@sec") {
                return '"' + std::to_string(calendar_time.tm_sec) + '"';
            } else if (target == "@min") {
                return '"' + std::to_string(calendar_time.tm_min) + '"';
            } else if (target == "@hour") {
                return '"' + std::to_string(calendar_time.tm_hour) + '"';
            } else if (target == "@mday") {
                return '"' + std::to_string(calendar_time.tm_mday) + '"';
            } else if (target == "@yday") {
                return '"' + std::to_string(calendar_time.tm_yday) + '"';
            } else if (target == "@mon") {
                return '"' + std::to_string(calendar_time.tm_mon) + '"';
            } else if (target == "@year") {
                return '"' + std::to_string(calendar_time.tm_year + 1900) + '"';
            } else if (target == "@clipboard") {
                return '"' + getClip() + '"';
            } else if (target == "@home") {
                char path[ MAX_PATH ];
                if (SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, path) != S_OK) {
                    return "Could not retrieve home dir.";
                } else {
                    std::string p = path;
                    return std::to_string(p.front());
                } 
            } else if (target == "@desktopW") {
                //int w, h;
                //GetDesktopResolution(w, h);
                return "0";
            } else if (target == "@desktopH") {
                //int w, h;
                //GetDesktopResolution(w, h);
                return "0";
            } else if (target == "@EOL") {
                return "\n";
            } else if (target == "@environment") {
                std::string returnvariable = "name:value-> ";
                for (int i = 0; i < scope.names.size(); i++) {
                    returnvariable += scope.names[i] + ":" + scope.values[i] + " | ";
                }
                return '"' + returnvariable + '"';
            } else if (target == "@IP") {
                return '"' + IP() + '"';
            } else if (target == "@inf") {
                return std::to_string(std::numeric_limits<double>::max());
            } else if (target == "@write") {
                return "@write";
            } else if (target == "@append") {
                return "@append";
            } else {
                *error_occurred = true;
                error(token, "Run-time Error: " + token.str() + " is an undefined macro.");
            }
        } else {
            std::pair<bool, int> found = findInV(scope.names, target);
            if (found.first == false) {
                std::vector<std::string> ops = { "+", "-", "*", "/" };
                if (in(token.str(), ops)) {
                    return token.str();
                } else {
                    if (token.str().at(0) != '"' && token.str().at(0) != '0' && token.str().at(0) != '1' && token.str().at(0) != '2' && token.str().at(0) != '3' && token.str().at(0) != '4' && token.str().at(0) != '5' && token.str().at(0) != '6' && token.str().at(0) != '7' && token.str().at(0) != '8' && token.str().at(0) != '9' && token.str() != "true" && token.str() != "false" && (token.str().at(0) != '-')) {
                        *error_occurred = true;
                    } else {
                        return token.str();
                    }
                }
            } else {
                return scope.values[found.second];
            }
        }
    }
    return "  ";
}

//evaluates operation in the form of: lhs op rhs
bool evaluate(Token lhs, Token op, Token rhs, Scope scope, bool *error_occurred) {
    if (op.str() == "==") {
        return getVarVal(lhs, scope, error_occurred) == getVarVal(rhs, scope, error_occurred);
    } else if (op.str() == ">") {
        if (getVarVal(lhs, scope, error_occurred).at(0) != '"' && getVarVal(rhs, scope, error_occurred).at(0) != '"') {
            return std::stod(getVarVal(lhs, scope, error_occurred)) > std::stod(getVarVal(rhs, scope, error_occurred));
        } else {
            error(op, "Run-time Error: Invalid Argument.");
            *error_occurred = true;
        }
    } else if (op.str() == "<") {
        if (getVarVal(lhs, scope, error_occurred).at(0) != '"' && getVarVal(rhs, scope, error_occurred).at(0) != '"') {
            return std::stod(getVarVal(lhs, scope, error_occurred)) < std::stod(getVarVal(rhs, scope, error_occurred));
        } else {
            error(op, "Run-time Error: Invalid Argument.");
            *error_occurred = true;
        }
    } else if (op.str() == ">=" || op.str() == "=>") {
        if (getVarVal(lhs, scope, error_occurred).at(0) != '"' && getVarVal(rhs, scope, error_occurred).at(0) != '"') {
            return std::stod(getVarVal(lhs, scope, error_occurred)) >= std::stod(getVarVal(rhs, scope, error_occurred));
        } else {
            error(op, "Run-time Error: Invalid Argument.");
            *error_occurred = true;
        }
    } else if (op.str() == "<=" || op.str() == "=<") {
        if (getVarVal(lhs, scope, error_occurred).at(0) != '"' && getVarVal(rhs, scope, error_occurred).at(0) != '"') {
            return std::stod(getVarVal(lhs, scope, error_occurred)) <= std::stod(getVarVal(rhs, scope, error_occurred));
        } else {
            error(op, "Run-time Error: Invalid Argument.");
            *error_occurred = true;
        }
    } else if (op.str() == ">") {
        if (getVarVal(lhs, scope, error_occurred).at(0) != '"' && getVarVal(rhs, scope, error_occurred).at(0) != '"') {
            return std::stod(getVarVal(lhs, scope, error_occurred)) > std::stod(getVarVal(rhs, scope, error_occurred));
        } else {
            error(op, "Run-time Error: Invalid Argument.");
            *error_occurred = true;
        }
    } else if (op.str() == "!=") {
        if (getVarVal(lhs, scope, error_occurred).at(0) != '"' && getVarVal(rhs, scope, error_occurred).at(0) != '"') {
            return std::stod(getVarVal(lhs, scope, error_occurred)) != std::stod(getVarVal(rhs, scope, error_occurred));
        } else {
            return getVarVal(lhs, scope, error_occurred) != getVarVal(rhs, scope, error_occurred);
        }
    }
}

bool isOp(Token token) {
    if (token.typ() == GREATER || token.typ() == GREATER_EQUAL || token.typ() == LESS || token.typ() == LESS_EQUAL || token.typ() == EXC_EQUAL || token.typ() == EQUAL_EQUAL) {
        return true;
    } else {
        return false;
    }
}

void errorCheck(std::vector<Token> line, bool *error_occurred) {
    std::vector<std::string> variables;
    std::vector<std::vector<Token>> variable_values;
    std::vector<Type> literals = { STRING, NUMBER, TTRUE, TFALSE, IDENTIFIER };
    std::vector<Type> notok = { EQUAL_EQUAL, EQUAL, EXC_EQUAL, GREATER, LESS, LESS_EQUAL, GREATER_EQUAL };
    std::vector<Type> OpOk = { STRING, CONSTANT, NUMBER, IDENTIFIER, RIGHT_PAREN, LEFT_PAREN, TTRUE, TFALSE, LENGTH, AT, EVAL, RAND, STR, TOKEN_INT, TOKEN_INPUT, HASH, IS_STRING, IS_NUMBER, IS_BOOL, SOLVE };
    for (int num_l = 0; num_l < line.size()-1; num_l++) {
        Token cur = line[num_l];
        Token nex = line[num_l+1];
        //ERROR CHECKING
        //if there are two literals together, ex "string literal" 1672
        if (in(cur.typ(), literals) && in(nex.typ(), literals)) {
            error(cur, "Syntax Error: Stray token.");
            *error_occurred = true;
        }
        //if there are two identifiers next to each other, ex variable1 variable2
        //if two non-compatible operators are adjacent
        else if (in(cur.typ(), notok) && in(nex.typ(), notok)) {
            error(cur, "Syntax Error: Stray operator.");
            *error_occurred = true;
        } else if (cur.typ() == IMMUTABLE && nex.typ() != IDENTIFIER) {
            error(cur, "Syntax Error: Immutable not followed by an identifier.");
            *error_occurred = true; 
        } else if ((cur.typ() == STRING || cur.typ() == NUMBER || cur.typ() == TTRUE || cur.typ() == TFALSE) && nex.typ() == LEFT_PAREN) {
            error(cur, "Syntax Error: Stray parentheses.");
            *error_occurred = true;
        } else if ( isOp(cur) && ( !in(nex.typ(), OpOk ) || !in(line[num_l-1].typ(), OpOk ) ) ) {
            error(cur, "Syntax Error: Stray Comparator.");
            *error_occurred = true;
        }
    }
}

std::vector<std::vector<Token>> findParams(std::vector<Token> &line, std::vector<Token>::iterator start, Type delimiter, std::vector<std::string> names, bool &err) {
    std::vector<std::vector<Token>> final;
    std::vector<Token> current;
    std::vector<std::string> constants = { "@EOL", "@sec", "@min", "@hour", "@mday", "@yday", "@mon", "@year", "@clipboard", "@home", "@desktopW", "@desktopH", "@environment", "@IP", "@inf", "@write", "@append" };
    int nested = 0;
    for (auto a = start; a < line.end(); a++) {
        if ((*a).typ() == IDENTIFIER && findInV(names, (*a).str()).first == false && !in((*a).str(), constants) && a != start) {
            error(*a, "Run-time Error: Undefined variable.");
            err = true;
            break;
        }
        //std::cout << "nested: " << nested << std::endl;
        if ((*a).typ() == delimiter) {
            //std::cout << "COMMA TIME!\ncurrent.s: " << current.size() << std::endl;
            if (!current.empty()) {
                final.push_back(current);
                current.clear();
            }
        } else if ((*a).typ() == RIGHT_PAREN && nested == 1) {
            if (!current.empty()) {
                final.push_back(current);
                current.clear();
            }
            break;
        } else if ((*a).typ() != LEFT_PAREN && nested > 0) {
            current.push_back(*a);
        }
        if ((*a).typ() == LEFT_PAREN) {
            nested++;
        } else if ((*a).typ() == RIGHT_PAREN) {
            nested--;
        }
        if (((*a).typ() == RIGHT_PAREN && nested == 1) || (*a).typ() == SEMICOLON) {
            break;
        }
    }
    return final;
}

std::string hash(std::string source) {
    std::hash<std::string> hasher;
    return std::to_string(hasher(source));
}