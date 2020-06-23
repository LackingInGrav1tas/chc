#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <ctime>
#include <winuser.h>
#include <winbase.h>
#include <shlobj.h>
#include "wtypes.h"
#include <winsock.h>
#include <limits>
#include "header.hpp"


std::string getString(char x) { 
    std::string s(1, x); 
    return s;
}

void error(Token token, std::string message="") {
    std::cout << "\n";
    std::string a = std::to_string(token.lines()) + "| ";
    std::cerr << a << token.actual_line() << "\n" << message << "\n\n";
}

Type singleChar(char current_char) {
    Type enumed;
    if (current_char == '!') {
        enumed = EXC;
    } else if (current_char == '+') {
        enumed = PLUS;
    } else if (current_char == '-') {
        enumed = MINUS;
    } else if (current_char == '/') {
        enumed = SLASH;
    } else if (current_char == '*') {
        enumed = STAR;
    } else if (current_char == ',') {
        enumed = COMMA;
    } else if (current_char == ';') {
        enumed = SEMICOLON;
    } else if (current_char == ')') {
        enumed = RIGHT_PAREN;
    } else if (current_char == '(') {
        enumed = LEFT_PAREN;
    } else if (current_char == '}') {
        enumed = RIGHT_BRACE;
    } else if (current_char == '{') {
        enumed = LEFT_BRACE;
    } else if (current_char == '<') {
        enumed = LESS;
    } else if (current_char == '>') {
        enumed = GREATER;
    } else if (current_char == '=') {
        enumed = EQUAL;
    } else if (current_char == '.') {
        enumed = DOT;
    }
    return enumed;
}

Type doubleChar(std::string full) {
    Type enumed;
    if (full == "!=") {
        enumed = EXC_EQUAL;
    } else if (full == "==") {
        enumed = EQUAL_EQUAL;
    } else if (full == ">=") {
        enumed = GREATER_EQUAL;
    } else if (full == "<=") {
        enumed = LESS_EQUAL;
    }
    return  enumed;
}

Type keyword(std::string full) {
    Type ret;
    if (full == "and") {
        ret = AND;
    } else if (full == "class") {
        ret = CLASS;
    } else if (full == "else") {
        ret = ELSE;
    } else if (full == "false") {
        ret = TFALSE;
    } else if (full == "fun") {
        ret = FUN;
    } else if (full == "for") {
        ret = FOR;
    } else if (full == "if") {
        ret = IF;
    } else if (full == "nil") {
        ret = NIL;
    } else if (full == "or") {
        ret = OR;
    } else if (full == "print") {
        ret = PRINT;
    } else if (full == "return") {
        ret = RETURN;
    } else if (full == "super") {
        ret = SUPER;
    } else if (full == "self") {
        ret = SELF;
    } else if (full == "true") {
        ret = TTRUE;
    } else if (full == "while") {
        ret = WHILE;
    } else if (full == "run") {
        ret = RUN;
    } else if (full == "define") {
        ret = DEFINE;
    } else if (full == "immutable") {
        ret = IMMUTABLE;
    } else if (full == "do") {
        ret = DO;
    } else if (full == "hash") {
        ret = HASH;
    } else if (full == "sleep") {
        ret = SLEEP;
    } else if (full == "break") {
        ret = BREAK;
    } else if (full == "aware") {
        ret = AWARE;
    }
    return ret;
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

std::string GetClip() {
    std::string text;
    if (OpenClipboard(NULL)) 
    {
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

void GetDesktopResolution(int& horizontal, int& vertical) {
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    horizontal = desktop.right;
    vertical = desktop.bottom;
}

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

std::string getVarVal(Token token, std::vector<std::string> varnames, std::vector<std::string> varvalues,  bool *error_occurred) {//
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
                return '"' + GetClip() + '"';
            } else if (target == "@home") {
                char path[ MAX_PATH ];
                if (SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, path) != S_OK) {
                    return "Could not retrieve home dir.";
                } else {
                    std::string p = path;
                    return std::to_string(p.front());
                } 
            } else if (target == "@desktopW") {
                int w, h;
                GetDesktopResolution(w, h);
                return std::to_string(w);
            } else if (target == "@desktopH") {
                int w, h;
                GetDesktopResolution(w, h);
                return std::to_string(h);
            } else if (target == "@EOL") {
                return "\n";
            } else if (target == "@environment") {
                std::string returnvariable = "name:value-> ";
                for (int i = 0; i < varnames.size(); i++) {
                    returnvariable += varnames[i] + ":" + varvalues[i] + " | ";
                }
                return '"' + returnvariable + '"';
            } else if (target == "@IP") {
                return '"' + IP() + '"';
            } else if (target == "@inf") {
                return std::to_string(std::numeric_limits<double>::max());
            } else {
                *error_occurred = true;
                error(token, "Run-time Error: " + token.str() + " is an undefined macro.");
            }
        } else {
            std::pair<bool, int> found = findInV(varnames, target);
            if (found.first == false) {
                std::vector<std::string> ops = { "+", "-", "*", "/" };
                if (in(token.str(), ops)) {
                    return token.str();
                } else {
                    if (token.str().at(0) != '"' && token.str().at(0) != '0' && token.str().at(0) != '1' && token.str().at(0) != '2' && token.str().at(0) != '3' && token.str().at(0) != '4' && token.str().at(0) != '5' && token.str().at(0) != '6' && token.str().at(0) != '7' && token.str().at(0) != '8' && token.str().at(0) != '9' && token.str() != "true" && token.str() != "false" && (token.str().at(0) != '-')) {
                        *error_occurred = true;
                    } else {
                        //std::cout << "in gvv: " << token.str() << std::endl;
                        return token.str();
                    }
                }
            } else {
                std::string value = varvalues[found.second];
                //std::cout << "in gvv (value): " << value << std::endl;
                return value;
            }
        }
    }
    return "  ";
}
/*
if (in(cur.typ(), literals) && in(next.typ(), literals)) {
    error(cur, "Two literals next to each other. Did you want to have an operator between them?");
    *error_occurred = true;
}
if (cur.typ() == IDENTIFIER && next.typ() == IDENTIFIER) {
    error(cur, "Two identifiers next to each other. Did mean to put a space in between them?");
    *error_occurred = true;
}
*/

bool evaluate(Token lhs, Token op, Token rhs, std::vector<std::string> names, std::vector<std::string> values, bool *error_occurred) {
    if (op.str() == "==") {
        //"clipboard" vs "stop"
        //std::cout << getVarVal(lhs, names, values, error_occurred) << " == " << getVarVal(rhs, names, values, error_occurred) << " -> " << (getVarVal(lhs, names, values, error_occurred) == getVarVal(rhs, names, values, error_occurred)) << std::endl;
        return getVarVal(lhs, names, values, error_occurred) == getVarVal(rhs, names, values, error_occurred);
    } else if (op.str() == ">") {
        if (getVarVal(lhs, names, values, error_occurred).at(0) != '"' && getVarVal(rhs, names, values, error_occurred).at(0) != '"') {
            return std::stod(getVarVal(lhs, names, values, error_occurred)) > std::stod(getVarVal(rhs, names, values, error_occurred));
        } else {
            error(op, "Run-time Error: Invalid Argument.");
            *error_occurred = true;
        }
    } else if (op.str() == "<") {
        if (getVarVal(lhs, names, values, error_occurred).at(0) != '"' && getVarVal(rhs, names, values, error_occurred).at(0) != '"') {
            return std::stod(getVarVal(lhs, names, values, error_occurred)) < std::stod(getVarVal(rhs, names, values, error_occurred));
        } else {
            error(op, "Run-time Error: Invalid Argument.");
            *error_occurred = true;
        }
    } else if (op.str() == ">=" || op.str() == "=>") {
        if (getVarVal(lhs, names, values, error_occurred).at(0) != '"' && getVarVal(rhs, names, values, error_occurred).at(0) != '"') {
            return std::stod(getVarVal(lhs, names, values, error_occurred)) >= std::stod(getVarVal(rhs, names, values, error_occurred));
        } else {
            error(op, "Run-time Error: Invalid Argument.");
            *error_occurred = true;
        }
    } else if (op.str() == "<=" || op.str() == "=<") {
        if (getVarVal(lhs, names, values, error_occurred).at(0) != '"' && getVarVal(rhs, names, values, error_occurred).at(0) != '"') {
            return std::stod(getVarVal(lhs, names, values, error_occurred)) <= std::stod(getVarVal(rhs, names, values, error_occurred));
        } else {
            error(op, "Run-time Error: Invalid Argument.");
            *error_occurred = true;
        }
    } else if (op.str() == ">") {
        if (getVarVal(lhs, names, values, error_occurred).at(0) != '"' && getVarVal(rhs, names, values, error_occurred).at(0) != '"') {
            return std::stod(getVarVal(lhs, names, values, error_occurred)) > std::stod(getVarVal(rhs, names, values, error_occurred));
        } else {
            error(op, "Run-time Error: Invalid Argument.");
            *error_occurred = true;
        }
    } else if (op.str() == "!=") {
        if (getVarVal(lhs, names, values, error_occurred).at(0) != '"' && getVarVal(rhs, names, values, error_occurred).at(0) != '"') {
            return std::stod(getVarVal(lhs, names, values, error_occurred)) != std::stod(getVarVal(rhs, names, values, error_occurred));
        } else {
            return getVarVal(lhs, names, values, error_occurred) != getVarVal(rhs, names, values, error_occurred);
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
    std::vector<Type> OpOk = { STRING, CONSTANT, NUMBER, IDENTIFIER, RIGHT_PAREN, LEFT_PAREN, TTRUE, TFALSE };
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