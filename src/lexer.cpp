#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "header.hpp"
#include <iterator>
#include <sstream>
#include <windows.h>
#include <climits>

std::string ExeDir(int times_back=1) {
    char buffer[MAX_PATH];
    GetModuleFileName( NULL, buffer, MAX_PATH );
    std::string::size_type pos = std::string( buffer ).find_last_of( "\\/" );
    std::string path = std::string( buffer ).substr( 0, pos);;
    std::vector<std::string> indiv;
    std::string current = "";
    for (int i = 0; i < path.size(); i++) {
        std::string p = std::string(1, path[i]);
        if (p == R"(\)") {
            indiv.push_back(current);
            current = "";
        } else {
            current += p;
        }
    }
    for (int i = 1; i < times_back; i++) {
        indiv.pop_back();
    }
    const char* const delim = "\\";
    std::ostringstream imploded;
    std::copy(indiv.begin(), indiv.end(),
        std::ostream_iterator<std::string>(imploded, delim));
    std::string dir = imploded.str();
    return dir;
}

std::vector<std::string> keywords = { "and", "class", "else", "false", "fun", "for", "if", "nil", "or",
                                      "print", "return", "true", "while", "run",
                                      "immutable", "do", "hash", "sleep", "break", "aware", "input", "writeto",
                                      "assert", "length", "rprint", "fprint", "rfprint", "throw", "eval", "continue",
                                      "rand", "at", "dispose", "set_scope", "save_scope", "to_str", "to_num", "is_string", "is_number", "is_bool",
                                      "solve", "try", "catch", "getcontents", "use", "disable" };

std::vector<char> recognized_chars = { '(', ')', '.', '=', '+', '-', '*', '/', '{', '}', ',', '!', '<', '>', ';', 'a', 'b', 'c',
                                       'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u',
                                       'v', 'w', 'x', 'y', 'z', ' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
                                       'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '1', '2', '3', '4',
                                       '5', '6', '7', '8', '9', '0', '"', '#', '_', '@' };

std::vector<char> important_characters = { '(', ')', '=', '+', '-', '*', '/', '{', '}', ',', '!', '<', '>', ';' };

std::vector<char> doubleable = { '=', '+', '-', '/', '*', '>', '<', '!' };

std::vector<Token> lex(std::vector<std::string> f, bool *error_occurred, int &limit, int &precision, std::string file_name) {
    std::vector<Token> tokens;
    std::vector<std::string> dismissed;
    //PREPROCESSING
    std::string preline;
    for (auto fileline = f.begin(); fileline < f.end(); fileline++) {
        preline = *fileline;
        if (preline.length() > 1) {
            if (preline.at(0) == '$') {
                std::vector<std::string> preprocessors = { "$" };
                std::string current;
                //finding preprocessor commands
                for (int i = 1; i < preline.length(); i++) {
                    if (preline.at(i) == '"') {
                        i++;
                        for (; preline.at(i) != '"'; i++) {
                            current += preline.at(i);
                        }
                        if (!current.empty()) {
                            preprocessors.push_back(current);
                            current.clear();
                            continue;
                        }
                    } else if (preline.at(i) == ' ' || preline.at(i) == '$') {
                        if (!current.empty()) {
                            preprocessors.push_back(current);
                            current.clear();
                            continue;
                        }
                    }
                    current += preline.at(i);
                }

                if (!current.empty()) {
                    preprocessors.push_back(current);
                    current.clear();
                }
                if (preprocessors.size() > 2) {
                    //setting up dismissing functions
                    if (preprocessors[1] == "dismiss") {
                        for (int i = 2; i < preprocessors.size(); i++) {
                            dismissed.push_back(preprocessors[i]);
                        }
                    }
                    //changing the while loop limit
                    if (preprocessors[1] == "limit") {
                        try {
                            limit = std::stoi(preprocessors[2]);
                        } catch (...) {
                            continue;
                        }
                    }
                    //changing the number precision
                    if (preprocessors[1] == "precision") {
                        try {
                            precision = std::stoi(preprocessors[2]);
                        } catch (...) {
                            if (preprocessors[2] == "inf")
                                precision = INT_MAX;
                            continue;
                        }
                    }
                    //adding files
                    if (preprocessors[1] == "import") {
                        std::string import_file = preprocessors[2];
                        if (import_file.length() > 4) {
                            if (import_file.substr(0, 4) == "lib:") {
                                import_file = ExeDir(0) + R"(lib\)" + import_file.substr(4, import_file.length()) + ".chc";
                            }
                        }
                        std::ifstream file(import_file);
                        std::vector<std::string> string_vec;
                        std::string current_line;
                        while (std::getline(file, current_line)) {
                            string_vec.push_back(current_line);
                        }
                        auto import_tokens = lex(string_vec, error_occurred, limit, precision, import_file);
                        tokens.insert(tokens.end(), import_tokens.begin(), import_tokens.end());
                    }
                }
            }
        }
    }
    if (!dismissed.empty()) {
        //finding dismissed keywords in keywords
        std::vector<std::vector<std::string>::iterator> to_del;
        for (auto it = keywords.begin(); it < keywords.end(); it++) {
            if (in(*it, dismissed)) {
                to_del.push_back(it);
            }
        }
        //deleting dismissed keywords from keywords
        for (auto it = to_del.begin(); it < to_del.end(); it++) {
            keywords.erase(*it);
        }
    }
    
    //SCANNING
    std::string line;
    int row = 0;
    int col = 0;
    for (auto current_line = f.begin(); current_line < f.end(); current_line++) {
        line = *current_line;
        row++;
        col = 0;
        if (line.length() > 0) {
            if (line.at(0) == '$')//ignore preprocessors
                continue;
            else {
                std::string current_token = "";
                //iterating though the file, char by char
                for (std::string::iterator current_char = line.begin(); current_char != line.end(); current_char++) {
                    col++;
                    //checks if the character is acceptable
                    Token errorcheck(getString(*current_char), row, col,  ERR, line, file_name);
                    if (!in(*current_char , recognized_chars)) {
                        std::string errormsg = std::string("Compile-time Error: Unrecognized character: ") + *current_char;
                        error(errorcheck, errormsg);
                        *error_occurred = true;
                    }
                    //checks if the current character is independent of anything it might be attached to
                    if (*current_char == '"') {//it's a string literal
                        std::string literal = "";
                        current_char++;
                        literal += '"';
                        for (current_char; *current_char != '"'; current_char++) {
                            literal += *current_char;
                            col++;
                            if (col >= line.length()) {
                                *error_occurred == true;
                                error(Token(literal, row, col, STRING, line, file_name), "Compile-time Error: Unending string.");
                                break;
                            }
                        }
                        literal += '"';
                        tokens.push_back(Token(literal, row, col, STRING, line, file_name));
                    } else if (*current_char == '#') {//it's a comment
                        current_char++;
                        for (current_char; *current_char != '#'; current_char++) {
                            col++;
                            if (col >= line.length())
                                break;
                        }
                    } else if (in(*current_char, important_characters)) {
                        if (!current_token.empty()) {
                            Type enumed;
                            //checking if token is a keyword
                            if (!in(current_token, keywords)) {
                                //at this point, the literal is either an identifier or a number. This will check.
                                if ((current_token.at(0) - 48) >= 0 && (current_token.at(0) - 48) <= 9) {
                                    enumed = NUMBER;
                                } else {
                                    enumed = IDENTIFIER;
                                }
                            } else {
                                enumed = keyword(current_token);
                            }
                            tokens.push_back(Token(current_token, row, col, enumed, line, file_name));
                            current_token = "";
                        }
                        //checks if the character is connectable, ex: !=
                        if (in(*current_char, doubleable)) {
                            if (!in(*std::prev(current_char), doubleable) && !in(*std::next(current_char), doubleable)) {
                                tokens.push_back(Token(getString(*current_char), row, col, singleChar(*current_char), line, file_name));
                            } else if (in(*std::prev(current_char), doubleable)) {
                                if (in(*current_char, doubleable)) {
                                    std::string done = getString(*std::prev(current_char)) + getString(*current_char);
                                    tokens.push_back(Token(done, row, col, doubleChar(done), line, file_name));
                                }
                            }
                        } else {
                            tokens.push_back(Token(getString(*current_char), row, col, singleChar(*current_char), line, file_name));
                        }
                    //checks if it's whitespace/space in betwixt tokens
                    } else if (*current_char == ' ') {
                        if (!current_token.empty()) {
                            Type enumed;
                            //checking if token is a keyword
                            if (!in(current_token, keywords)) {
                                //at this point, the literal is either an identifier or a number. This will check.
                                if ((current_token.at(0) - 48) >= 0 && (current_token.at(0) - 48) <= 9) {
                                    enumed = NUMBER;
                                } else {
                                    enumed = IDENTIFIER;
                                }
                            } else {
                                enumed = keyword(current_token);
                            }
                            tokens.push_back(Token(current_token, row, col, enumed, line, file_name));
                            current_token = "";
                        }
                    //anything else it adds to the token
                    } else {
                        current_token += *current_char;
                    }
                }
            }
        }
    }
    //string form of tokenized list
    std::vector<std::string> n;
    for (std::vector<Token>::iterator it = tokens.begin(); it != tokens.end(); it++) {
        n.push_back((*it).str());
    }
    //checking if all brackets and parentheses are closed
    int plcount = std::count(n.begin(), n.end(), "(");
    int prcount = std::count(n.begin(), n.end(), ")");
    int blcount = std::count(n.begin(), n.end(), "{");
    int brcount = std::count(n.begin(), n.end(), "}");
    if (plcount > prcount) {
        std::vector<Token>::iterator it = tokens.end();
        bool errcont = true;
        while (errcont) {
            if ((*it).str() == "(")
                errcont = false;
            else
                it--;
        }
        error(*it, "Compile-time Error: Unclosed parentheses.");
        *error_occurred = true;
    } else if (plcount < prcount) {
        std::vector<Token>::iterator it = tokens.end();
        bool errcont = true;
        while (errcont) {
            if ((*it).str() == ")")
                errcont = false;
            else
                it--;
        }
        error(*it, "Compile-time Error: Uncalled for end parentheses.");
        *error_occurred = true;
    }
    if (blcount > brcount) {
        std::vector<Token>::iterator it = tokens.end();
        bool errcont = true;
        while (errcont) {
            if ((*it).str() == "{")
                errcont = false;
            else
                it--;
        }
        error(*it, "Compile-time Error: Unclosed bracket.");
        *error_occurred = true;
    } else if (blcount < brcount) {
        std::vector<Token>::iterator it = tokens.end();
        bool errcont = true;
        while (errcont) {
            if ((*it).str() == "}")
                errcont = false;
            else
                it--;
        }
        error(*it, "Compile-time Error: Uncalled for end bracket.");
        *error_occurred = true;
    }
    //replacing valid MINUS NUMBER with 0 MINUS NUMBER
    for (int token_i = 1; token_i < tokens.size()-1; token_i++) {
        if (tokens[token_i].typ() == MINUS && tokens[token_i-1].typ() != NUMBER && tokens[token_i-1].typ() != IDENTIFIER && tokens[token_i-1].typ() != RIGHT_PAREN && tokens[token_i+1].typ() == NUMBER) {
            tokens.insert(tokens.begin()+(token_i), Token("0", tokens[token_i].lines(), tokens[token_i].col(), NUMBER, tokens[token_i].actual_line(), file_name));
            tokens.insert(tokens.begin()+(token_i), Token("(", tokens[token_i].lines(), tokens[token_i].col(), LEFT_PAREN, tokens[token_i].actual_line(), file_name));
            tokens.insert(tokens.begin()+(token_i), Token("solve", tokens[token_i].lines(), tokens[token_i].col(), SOLVE, tokens[token_i].actual_line(), file_name));
            tokens.insert(tokens.begin()+(token_i+5), Token(")", tokens[token_i].lines(), tokens[token_i].col(), RIGHT_PAREN, tokens[token_i].actual_line(), file_name));
        } else if (tokens[token_i].typ() == PLUS_EQUALS) {
            tokens[token_i] = Token("=", tokens[token_i].lines(), tokens[token_i].col(), EQUAL, tokens[token_i].actual_line(), file_name);
            tokens.insert(tokens.begin()+(token_i+1), tokens[token_i-1]);
            tokens.insert(tokens.begin()+(token_i+2), Token("+", tokens[token_i].lines(), tokens[token_i].col(), PLUS, tokens[token_i].actual_line(), file_name));
        } else if (tokens[token_i].typ() == MINUS_EQUALS) {
            tokens[token_i] = Token("=", tokens[token_i].lines(), tokens[token_i].col(), EQUAL, tokens[token_i].actual_line(), file_name);
            tokens.insert(tokens.begin()+(token_i+1), tokens[token_i-1]);
            tokens.insert(tokens.begin()+(token_i+2), Token("-", tokens[token_i].lines(), tokens[token_i].col(), MINUS, tokens[token_i].actual_line(), file_name));
        } else if (tokens[token_i].typ() == STAR_EQUALS) {
            tokens[token_i] = Token("=", tokens[token_i].lines(), tokens[token_i].col(), EQUAL, tokens[token_i].actual_line(), file_name);
            tokens.insert(tokens.begin()+(token_i+1), tokens[token_i-1]);
            tokens.insert(tokens.begin()+(token_i+2), Token("*", tokens[token_i].lines(), tokens[token_i].col(), STAR, tokens[token_i].actual_line(), file_name));
        } else if (tokens[token_i].typ() == SLASH_EQUALS) {
            tokens[token_i] = Token("=", tokens[token_i].lines(), tokens[token_i].col(), EQUAL, tokens[token_i].actual_line(), file_name);
            tokens.insert(tokens.begin()+(token_i+1), tokens[token_i-1]);
            tokens.insert(tokens.begin()+(token_i+2), Token("/", tokens[token_i].lines(), tokens[token_i].col(), SLASH, tokens[token_i].actual_line(), file_name));
        }
    }
    return tokens;
}