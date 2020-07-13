#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <chrono>
#include "header.hpp"
#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0
std::ifstream::pos_type filesize(const char* filename) {//not my code
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

int main(int argc, char** argv) {
    bool i = false;
    int limit = 100;
    int precision = 6;
    for (int j = 1; j < argc; j++) {
        if (*argv[j] == 'i') {i = true;}
        std::string full = argv[j];
        if (full.at(0) == 'l') {
            if (full.length() > 3) {
                if (full.at(1) == '=') {
                    try {
                        limit = std::stoi(std::string(full.begin()+2, full.end()));
                    } catch(...) {
                        std::cout << "the correct syntax to set the loop limit is: " << argv[0] << "c:/.../example.chc l=%desired limit%" << std::endl;
                    }
                }
            }
        }
    }
    if (argc < 2) {
        std::cerr << "The accepted format is: " << argv[0] << " c:/.../example.chc OPTIONAL{i, l=?}";
        return EXIT_FAILURE;
    }
    std::string forsubstr = argv[1];
    if (forsubstr.substr(forsubstr.length() - 4) != ".chc") {
        std::cerr << "Expected a .chc file.";
        return EXIT_FAILURE;
    }
    //timer
    auto start = std::chrono::steady_clock::now();
    bool error_occurred = false;
    std::vector<Token> one = lex(argv[1], &error_occurred, limit, precision);//lexing
    one.push_back(Token("", 0, 0, _EOF, "", "outside of files"));
    errorCheck(one, &error_occurred);
    std::vector<std::vector<Token>> two = statementize(one);
    int exit_status = 0;
    if (!error_occurred) {
        Scope scope;
        std::vector<Token> rv;
        try {
            exit_status = runtime(two, scope, &error_occurred, limit, precision, rv);//runtime
        } catch(...) {
            std::cerr << "Error: Unknown Error.";
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
        infofile << "Date/time of executing: " << dt;
        std::cout << "compilation/run time...\n";
        infofile << "Time taken for compilation/runtime: " << std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) << " ms\n\n";
        std::cout << "token printout...\n";
        std::vector<std::string> token_list = {"BLANK", "ERROR", "EOF", "LEFT_PAREN", "RIGHT_PAREN", "LEFT_BRACE", "RIGHT_BRACE", "COMMA",
                "DOT", "MINUS", "PLUS", "SEMICOLON", "SLASH", "STAR", "EXC", "EXC_EQUAL", "EQUAL",
                "EQUAL_EQUAL", "GREATER", "GREATER_EQUAL", "LESS", "LESS_EQUAL", "MINUS_MINUS", "PLUS_PLUS", "PLUS_EQUALS", "MINUS_EQUALS",
                "STAR_EQUALS", "SLASH_EQUALS", "IDENTIFIER", "STRING", "NUMBER", "CONSTANT", "AND", "CLASS", 
                "ELSE", "FALSE", "FUN", "FOR", "IF", "NIL", "OR",
                "PRINT", "RETURN", "TRUE", "WHILE", "RUN", "IMMUTABLE", "DO", "HASH",
                "SLEEP", "BREAK", "AWARE", "_VOID_FUNC_HOLDER", "INPUT", "WRITETO", "ASSERT", "LENGTH", "RPRINT",
                "FPRINT", "RFPRINT", "THROW", "EVAL", "CONTINUE", "RAND", "AT", "ARROW", "DISPOSE", "SET_SCOPE",
                "SAVE_SCOPE", "STR", "INT", "IS_STRING", "IS_NUMBER", "IS_BOOL", "SOLVE"};
        std::string tokenspo = "";
        std::string typespo = "";
        for (std::vector<Token>::iterator it = one.begin(); it < one.end(); it++) {
            tokenspo += (*it).str() + "  ";
            typespo += token_list[(*it).typ()] + "  ";
            if ((*it).typ() == SEMICOLON || (*it).typ() == LEFT_BRACE) {typespo += "\n";}
        }
        infofile << tokenspo << "\n\n";
        std::cout << "type printout...\n";
        infofile << typespo << "\n";
        std::cout << "INFO COMPLETED" << std::endl;
        infofile.close();
        system((R"(")" + newpath + R"(")").c_str());
    }
    return exit_status;
}