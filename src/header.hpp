#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stack>
#include <utility>
#include <sstream>
#include <iomanip>

enum Type {    
  BLANK, ERR, _EOF,

  // Single-character tokens.                      
  LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
  COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR, 

  // One or two character tokens.                  
  EXC, EXC_EQUAL,                                
  EQUAL, EQUAL_EQUAL,                              
  GREATER, GREATER_EQUAL,                          
  LESS, LESS_EQUAL, MINUS_MINUS,
  PLUS_PLUS, PLUS_EQUALS, MINUS_EQUALS,
  STAR_EQUALS, SLASH_EQUALS,

  // Literals.                                     
  IDENTIFIER, STRING, NUMBER, CONSTANT,

  // Keywords.                                     
  AND, TOKEN_STRUCT, ELSE, TFALSE, FUN, FOR, IF, NIL, OR,
  PRINT, RETURN, TTRUE, WHILE, RUN,
  IMMUTABLE, DO, HASH, SLEEP, BREAK, AWARE,
  _VOID_FUNC_HOLDER, TOKEN_INPUT, WRITETO, ASSERT,
  LENGTH, RPRINT, FPRINT, RFPRINT, THROW, EVAL, CONTINUE,
  RAND, AT, ARROW, DISPOSE, SET_SCOPE, SAVE_SCOPE, STR, TOKEN_INT,
  IS_STRING, IS_NUMBER, IS_BOOL, SOLVE, TRY, CATCH, GETCONTENTS, USE,
  DISABLE, PASTE, CUTBACK, Ok, Err
};

template <class T>
bool in(T item, std::vector<T> v) {
    bool ret = false;
    for (typename std::vector<T>::iterator i = v.begin(); i != v.end(); i++) {
        if (item == *i) {
            ret = true;
        }
    }
    return ret;
}

enum SType { OPERATOR, TERMINAL };
enum Assoc { RIGHT, LEFT };

class Token {
    std::string lexeme;
    int line;
    int column;
    std::string lin;
    std::string file;
    Type type;
    int precedence;
    std::map<std::string, int> precedence_lookup;
    Assoc associativity;
    SType syhtype;
    std::vector<std::string> right = { "^" };
    std::vector<std::string> operators = { "+", "-", "/", "*", "%", "(", ")", "^", ">", "<", "<=", "=<",
                                           ">=", "=>", "!", "!=" };
    public:
        Token(std::string init_lexeme="", int init_line=0, int init_column=0, Type init_type=BLANK, std::string init_lin="", std::string filename="unknown") {
            lexeme = init_lexeme;
            line = init_line;
            type = init_type;
            column = init_column;
            lin = init_lin;
            file = filename;
            precedence_lookup.insert(std::pair<std::string, int>(">", 2));
            precedence_lookup.insert(std::pair<std::string, int>("<", 2));
            precedence_lookup.insert(std::pair<std::string, int>(">=", 2));
            precedence_lookup.insert(std::pair<std::string, int>("<=", 2));
            precedence_lookup.insert(std::pair<std::string, int>("=>", 2));
            precedence_lookup.insert(std::pair<std::string, int>("=<", 2));
            precedence_lookup.insert(std::pair<std::string, int>("!=", 2));
            precedence_lookup.insert(std::pair<std::string, int>("!", 2));
            precedence_lookup.insert(std::pair<std::string, int>("+", 2));
            precedence_lookup.insert(std::pair<std::string, int>("-", 2));
            precedence_lookup.insert(std::pair<std::string, int>("/", 3));
            precedence_lookup.insert(std::pair<std::string, int>("*", 3));
            precedence_lookup.insert(std::pair<std::string, int>("%", 3));
            precedence_lookup.insert(std::pair<std::string, int>("^", 4));
            precedence_lookup.insert(std::pair<std::string, int>("(", 1));
            precedence_lookup.insert(std::pair<std::string, int>(")", 1));
            precedence = precedence_lookup[lexeme];
            if (in(lexeme, right)) {
                associativity = RIGHT;
            } else {
                associativity = LEFT;
            }
            if (in(lexeme, operators)) {
                syhtype = OPERATOR;
            } else {
                syhtype = TERMINAL;
            }
        }
        std::string str() {
            return lexeme;
        }
        void setStr(std::string newline) {
            lexeme = newline;
        }
        std::string actual_line() {
            return lin;
        }
        void setAcLine(std::string newline) {
            lin = newline;
        }
        int lines() {
            return line;
        }
        void setLineNum(int newline) {
            line = newline;
        }
        int col() {
            return column;
        }
        void setCol(int newline) {
            column = newline;
        }
        Type typ() {
            return type;
        }
        SType syhtyp() {
            return syhtype;
        }
        int prec() {
            return precedence;
        }
        Assoc asso() {
            return associativity;
        }
        std::string filename() {
            return file;
        }
};

struct Scope {
    std::vector<std::string> names, values, immutables, function_names, aware_functions, class_names;
    std::vector<std::vector<std::vector<Token>>> function_bodies;
    std::vector<std::vector<std::string>> function_params;
};

template <typename T>//https://stackoverflow.com/a/16606128/13132049 thanks
std::string to_string_with_precision(T a_value, int n) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(n) << a_value;
    return out.str();
}

template <typename S>
std::stack<S> stackify(std::vector<S> vec) {
    std::stack<S> ret;
    for (auto it = vec.rbegin(); it != vec.rend(); it++) {
        ret.push(*it);
    }
    return ret;
}

template <typename U>
std::vector<U> destackify(std::stack<U> sta) {
    std::vector<U> ret;
    while (!sta.empty()) {
        ret.insert(ret.begin(), sta.top());
        sta.pop();
    }
    return ret;
}

template <typename R>
std::pair<bool, int> findInV(std::vector<R> vec, R target) {
    int ret2;
    bool ret1;
    if (in(target, vec)) {
        ret2 = vec.size() + 10;
        for (int i = 0; i <= vec.size(); i++) {
            if (vec[i] == target) {
                ret2 = i;
            }
        }
        if (ret2 == vec.size() + 10) {
            ret1 = false;
        } else {
            ret1 = true;
        }
    } else {
        ret1 = false;
    }
    std::pair<bool, int> ret;
    ret.first = ret1;
    ret.second = ret2;
    return ret;
}

std::vector<std::string> get_lexemes(std::vector<Token> tokens);

std::string getString(char x);

void error(Token token, std::string message);

Type keyword(std::string full);

Type doubleChar(std::string full);

Type singleChar(char current_char);

std::vector<Token> lex(std::vector<std::string> f, bool *error_occurred, int &limit, int &precision, std::string file_name);

std::stack<Token> shunting_yard_algorithm(std::stack<Token> input_stack);

std::string getVarVal(Token token, Scope scope, bool *error_occurred);//

std::vector<std::vector<Token>> statementize(std::vector<Token> tokens, bool &error_occurred);

std::string solve(std::vector<Token> tokens, Scope scope, bool *error_occurred, int precision);

int runtime(std::vector<std::vector<Token>> statements, Scope &scope, bool *error_occurred, int limit, int precision, std::vector<Token> &return_variable);

bool evaluate(Token lhs, Token op, Token rhs, Scope scope, bool *error_occurred);

bool boolsolve(std::vector<Token> tokens, Scope scope, int limit, int precision, std::vector<Scope> &scopes, std::vector<std::string> &scope_indices, bool *error_occurred);

bool isOp(Token token);

std::vector<std::vector<Token>> findParams(std::vector<Token> &line, std::vector<Token>::iterator start, Type delimiter, Scope scope, bool &err, bool exclude_functions=true);

std::string hash(std::string source);

void cli();

void shorten(std::string &str);

int handle_functions(std::vector<Token> &stmt, Scope &scope, int limit, int precision, bool *error_occurred);