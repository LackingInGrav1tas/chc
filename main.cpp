#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <chrono>
#include "header.hpp"

std::ifstream::pos_type filesize(const char* filename) {
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg(); 
}

std::vector<std::string> TokenList = {"BLANK", "ERROR", "EOF", "LEFT_PAREN", "RIGHT_PAREN", "LEFT_BRACE", "RIGHT_BRACE", "COMMA",
                                      "DOT", "MINUS", "PLUS", "SEMICOLON", "SLASH", "STAR", "EXC", "EXC_EQUAL", "EQUAL",
                                      "EQUAL_EQUAL", "GREATER", "GREATER_EQUAL", "LESS", "LESS_EQUAL", "IDENTIFIER",
                                      "STRING", "NUMBER", "CONSTANT", "AND", "CLASS", "ELSE", "FALSE", "FUN", "FOR", "IF", "NIL", "OR",
                                      "PRINT", "RETURN", "SUPER", "SELF", "TRUE", "WHILE", "RUN", "DEFINE", "IMMUTABLE", "DO", "HASH", "SLEEP", "ELIF"};
//g++ main.cpp lexer.cpp functions.cpp solve.cpp syh.cpp runtime.cpp -o interpreter -lws2_32 && cls && interpreter "c:\users\owner\desktop\cpp\edu\compiler\2nd attempt\test.chc
int main(int argc, char** argv) {
    bool i = false;
    bool c = false;
    for (int j = 1; j < argc; j++) {
        if (*argv[j] == 'c') {c = true;}
        if (*argv[j] == 'i') {i = true;}
    }
    //timer
    auto start = std::chrono::steady_clock::now();
    //actual compiling part starts here:
    bool error_occurred = false;
    std::vector<Token> one = lex(argv[1], &error_occurred);
    one.push_back(Token("", 0, 0, _EOF, ""));
    errorCheck(one, &error_occurred);
    std::vector<std::vector<Token>> two = statementize(one);
    int exit_status = 0;
    if (!error_occurred) {
        std::vector<std::string> n;
        std::vector<std::string> v;
        std::vector<std::string> i;
        try {
            exit_status = runtime(two, n, v, i, &error_occurred);
        } catch(...) {
            std::cerr << "Error: Unknown Error.\nMost likely a statement not finished with a semicolon, e.g.\nif () {\n#do stuff#\n} <- ; needed.";
        }
    } else {
        std::cout << "RUNTIME TERMINATED\n";
        exit_status = 1;
    }
    auto end = std::chrono::steady_clock::now();
    //if the user added the postfix command "i"
    if (i) {
        //writing info to file
        std::string ar = argv[1];
        std::string newpath = ar.substr(0, ar.length() - 4) + "_info.txt";
        std::ofstream infofile;
        infofile.open (newpath);
        std::cout << "\n\nWRITING INFO (" << newpath << ")\nnewpath opened...\n";
        infofile << "Source size: " << filesize(argv[1]) << "b\n";// << "  Compiled size: " << filesize(final file) << "b\n\n";
        std::cout << "source size...\n";
        std::cout << "date...\n";
        time_t now = time(0);
        char* dt = ctime(&now);
        infofile << "Date/time of compilation: " << dt;
        std::cout << "compilation time...\n";
        infofile << "Time taken for compilation: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n\n";
        std::string tokenspo = "";
        std::string typespo = "";
        for (std::vector<Token>::iterator it = one.begin(); it < one.end(); it++) {
            Token current = *it;
            tokenspo += current.str() + "  ";
            typespo += TokenList[current.typ()] + "  ";
            if (current.typ() == SEMICOLON) {typespo += "\n";}
        }
        std::cout << "token printout...\n";
        infofile << tokenspo << "\n\n";
        std::cout << "type printout...\n";
        infofile << typespo << "\n";
        std::cout << "INFO COMPLETED";
        infofile.close();
    }
    return exit_status;
}