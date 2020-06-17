#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "header.hpp"

std::vector<std::string> keywords = { "and", "class", "else", "false", "fun", "for", "if", "nil", "or", "print", "return", "super", "self", "true", "while", "run", "define", "immutable", "do", "hash", "sleep", "break", "elif" };

std::vector<char> recognized_chars = { '(', ')', '.', '=', '+', '-', '*', '/', '{', '}', ',', '!', '<', '>', ';', 'a', 'b', 'c',
                                       'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u',
                                       'v', 'w', 'x', 'y', 'z', ' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
                                       'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '1', '2', '3', '4',
                                       '5', '6', '7', '8', '9', '0', '"', '#', '_', '@' };

std::vector<char> important_characters = { '(', ')', '.', '=', '+', '-', '*', '/', '{', '}', ',', '!', '<', '>', ';' };

std::vector<char> doubleable = { '=', '+', '-', '/', '*', '>', '<', '!' };

std::vector<Token> lex(std::string f, bool *error_occurred) {
    std::ifstream file(f);
    if (!file) {
        std::cout << R"(")" << f << R"(")" << " not found.";
        exit(1);
    }
    std::string line;
    int row = 0;
    int col = 0;
    std::vector<Token> tokens;
    while (std::getline(file, line)) {
        row++;
        col = 0;
        std::string current_token = "";
        //iterating though the file, char by char
        for (std::string::iterator current_char = line.begin(); current_char != line.end(); current_char++) {
            col++;
            //checks if the character is acceptable
            Token errorcheck(getString(*current_char), row, col,  ERR, line);
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
                bool r = false;
                for (current_char; *current_char != '"'; current_char++) {
                    literal += *current_char;
                    col++;
                    if (col >= line.length()) {
                        *error_occurred == true;
                        r = true;
                        break;
                    }
                }
                literal += '"';
                Token tok(literal, row, col, STRING, line);
                if (r) {
                    error(tok, "Compile-time Error: Unending string.");
                }
                tokens.push_back(tok);
            } else if (*current_char == '#') {//it's a comment
                current_char++;
                bool r = false;
                std::string literal = "#";
                for (current_char; *current_char != '#'; current_char++) {
                    literal += *current_char;
                    col++;
                    if (col >= line.length()) {
                        *error_occurred == true;
                        r = true;
                        break;
                    }
                }
                literal += '#';
                Token tok(literal, row, col, ERR, line);
                if (r) {
                    error(tok, "Compile-time Error: Unending comment.");
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
                    Token tok(current_token, row, col, enumed, line);
                    tokens.push_back(tok);
                    current_token = "";
                }
                //checks if the character is connectable, ex: !=
                if (in(*current_char, doubleable)) {
                    if (!in(*std::prev(current_char), doubleable) && !in(*std::next(current_char), doubleable)) {
                        Type enumed = singleChar(*current_char);
                        Token tok(getString(*current_char), row, col, enumed, line);
                        tokens.push_back(tok);
                    } else if (in(*std::prev(current_char), doubleable)) {
                        if (in(*current_char, doubleable)) {
                            std::string done = getString(*std::prev(current_char)) + getString(*current_char);
                            Type enumed = doubleChar(done);
                            Token tok(done, row, col, enumed, line);
                            tokens.push_back(tok);
                        }
                    }
                } else {
                    Token tok(getString(*current_char), row, col, singleChar(*current_char), line);
                    tokens.push_back(tok);
                }
            //checks if it's whitespace/space in betwixt tokens
            } else if (*current_char == ' ') {
                if (!current_token.empty()) {
                    Type enumed;
                    //checking if token is a keyword
                    if (!in(current_token, keywords)) {
                        //std::cout << R"(")" << current_token << R"(")" << "\n";
                        //at this point, the literal is either an identifier or a number. This will check.
                        if ((current_token.at(0) - 48) >= 0 && (current_token.at(0) - 48) <= 9) {
                            //std::cout << "num\n";
                            enumed = NUMBER;
                        } else {
                            //std::cout << "ident\n";
                            enumed = IDENTIFIER;
                        }
                    } else {
                        enumed = keyword(current_token);
                    }
                    Token tok(current_token, row, col, enumed, line);
                    tokens.push_back(tok);
                    current_token = "";
                }
            //anything else it adds to the token
            } else {
                current_token += *current_char;
            }
        }
    }

    std::vector<std::string> n;
    for (std::vector<Token>::iterator it = tokens.begin(); it != tokens.end(); it++) {
        Token b = *it;
        n.push_back(b.str());
    }
    //removing comments
    while (in(std::string("//"), n)) {
        std::pair<bool, int> index = findInVector(n, std::string("//"));
        if (index.first) {
            n.erase(n.begin()+index.second);
            tokens.erase(tokens.begin()+index.second);
            while (n[index.second] != "//") {
                n.erase(n.begin()+index.second);
                tokens.erase(tokens.begin()+index.second);
            }
            n.erase(n.begin()+index.second);
            tokens.erase(tokens.begin()+index.second);
        }
    }
    //checking if all brackets and parentheses are closed
    int plcount = std::count(n.begin(), n.end(), "(");
    int prcount = std::count(n.begin(), n.end(), ")");
    int blcount = std::count(n.begin(), n.end(), "{");
    int brcount = std::count(n.begin(), n.end(), "}");
    Token currenttok;
    if (plcount > prcount) {
        std::vector<Token>::iterator it = tokens.end();
        bool errcont = true;
        while (errcont) {
            currenttok = *it;
            if (currenttok.str() == "(") {
                errcont = false;
            } else {
                it--;
            }
        }
        error(currenttok, "Compile-time Error: Unclosed parentheses.\n");
        *error_occurred = true;
    } else if (plcount < prcount) {
        std::vector<Token>::iterator it = tokens.end();
        bool errcont = true;
        while (errcont) {
            currenttok = *it;
            if (currenttok.str() == ")") {
                errcont = false;
            } else {
                it--;
            }
        }
        error(currenttok, "Compile-time Error: Uncalled for end parentheses.\n");
        *error_occurred = true;
    }
    if (blcount > brcount) {
        std::vector<Token>::iterator it = tokens.end();
        bool errcont = true;
        while (errcont) {
            currenttok = *it;
            if (currenttok.str() == "{") {
                errcont = false;
            } else {
                it--;
            }
        }
        error(currenttok, "Compile-time Error: Unclosed bracket.\n");
        *error_occurred = true;
    } else if (blcount < brcount) {
        std::vector<Token>::iterator it = tokens.end();
        bool errcont = true;
        while (errcont) {
            currenttok = *it;
            if (currenttok.str() == "}") {
                errcont = false;
            } else {
                it--;
            }
        }
        error(currenttok, "Compile-time Error: Uncalled for end bracket.\n");
        *error_occurred = true;
    }
    
    return tokens;
}