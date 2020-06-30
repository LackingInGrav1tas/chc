#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stack>
#include <utility>

enum Type {    
  BLANK, ERR, _EOF,

  // Single-character tokens.                      
  LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
  COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR, 

  // One or two character tokens.                  
  EXC, EXC_EQUAL,                                
  EQUAL, EQUAL_EQUAL,                              
  GREATER, GREATER_EQUAL,                          
  LESS, LESS_EQUAL,                                

  // Literals.                                     
  IDENTIFIER, STRING, NUMBER, CONSTANT,

  // Keywords.                                     
  AND, CLASS, ELSE, TFALSE, FUN, FOR, IF, NIL, OR,  
  PRINT, RETURN, SUPER, SELF, TTRUE, WHILE, RUN,
  DEFINE, IMMUTABLE, DO, HASH, SLEEP, BREAK, AWARE,
  _VOID_FUNC_HOLDER, TOKEN_INPUT, WRITETO, ASSERT,
  LENGTH, RPRINT, FPRINT, RFPRINT, THROW, EVAL
                                           
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
    Type type;
    int precedence;
    std::map<std::string, int> precedence_lookup;
    Assoc associativity;
    SType syhtype;
    std::vector<std::string> right = { "^" };
    std::vector<std::string> operators = { "+", "-", "/", "*", "%", "(", ")", "^", ">", "<", "<=", "=<",
                                           ">=", "=>", "!", "!=" };
    public:
        Token(std::string init_lexeme="", int init_line=0, int init_column=0, Type init_type=BLANK, std::string init_lin="") {
            lexeme = init_lexeme;
            line = init_line;
            type = init_type;
            column = init_column;
            lin = init_lin;
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
        std::string actual_line() {
            return lin;
        }
        int lines() {
            return line;
        }
        int col() {
            return column;
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
};

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

template < typename T>
std::pair<bool, int > findInVector(const std::vector<T>  & vecOfElements, const T  & element)
{
	std::pair<bool, int > result;
 
	// Find given element in vector
	auto it = std::find(vecOfElements.begin(), vecOfElements.end(), element);
 
	if (it != vecOfElements.end())
	{
		result.second = distance(vecOfElements.begin(), it);
		result.first = true;
	}
	else
	{
		result.first = false;
		result.second = -1;
	}
 
	return result;
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

std::vector<std::vector<Token>> get_statements(std::vector<Token> tokens);

std::vector<std::string> get_lexemes(std::vector<Token> tokens);

std::string getString(char x);

void error(Token token, std::string message);

Type keyword(std::string full);

Type doubleChar(std::string full);

Type singleChar(char current_char);

std::vector<Token> lex(std::string f, bool *error_occurred, int &limit);

std::vector<std::vector<Token>> get_lines(std::vector<Token> tokens);

void errorCheck(std::vector<Token> line, bool *error_occurred);

std::stack<Token> shunting_yard_algorithm(std::stack<Token> input_stack);

std::string getVarVal(Token token, std::vector<std::string> varnames, std::vector<std::string> varvalues, bool *error_occurred);//

std::vector<std::vector<Token>> statementize(std::vector<Token> tokens);

std::string solve(std::vector<Token> tokens, std::vector<std::string> names, std::vector<std::string> values, bool *error_occurred);

int runtime(std::vector<std::vector<Token>> statements, std::vector<std::string> &names, std::vector<std::string> &values, std::vector<std::string> &immutables, bool *error_occurred, int limit, std::vector<std::vector<std::vector<Token>>> function_bodies, std::vector<std::string> function_names, std::vector<std::string> aware_functions, std::vector<std::vector<std::string>> function_params, std::vector<Token> &return_variable);

bool evaluate(Token lhs, Token op, Token rhs, std::vector<std::string> names, std::vector<std::string> values, bool *error_occurred);

bool boolsolve(std::vector<Token> tokens, std::vector<std::string> names, std::vector<std::string> values, bool *error_occurred);

bool isOp(Token token);

std::vector<std::vector<Token>> findParams(std::vector<Token> &line, std::vector<Token>::iterator start, Type delimiter);

std::string hash(std::string source);

void cli();