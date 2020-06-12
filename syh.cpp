#include <iostream>
#include <vector>
#include <map>
#include <stack>
#include <string>
#include "header.hpp"

std::stack<Token> shunting_yard_algorithm(std::stack<Token> input_stack) {
    std::stack<Token> output_stack;
    std::stack<Token> operator_stack;
    while (!input_stack.empty()) {
        //std::cout << input_stack.top().str() << "\n";
        if (input_stack.top().syhtyp() == TERMINAL) {
            //std::cout << "top of input stack == terminal: pushing " << input_stack.top().str() << " from input to output" << std::endl;
            output_stack.push(input_stack.top());
            input_stack.pop();
        } else {
            if (input_stack.top().str() == "(") {
                //std::cout << "top of input stack == (: pushing " << input_stack.top().str() << " from input to operator" << std::endl;
                operator_stack.push(input_stack.top());
                input_stack.pop();
            } else if (input_stack.top().str() == ")") {
                while (operator_stack.top().str() != "(") {
                    //std::cout << "top of operator stack != (: pushing " << operator_stack.top().str() << " from operator to output" << std::endl;
                    output_stack.push(operator_stack.top());
                    operator_stack.pop();
                }
                input_stack.pop();
            } else if (input_stack.top().syhtyp() == OPERATOR) {
                if (operator_stack.size() == 0 || operator_stack.top().prec() < input_stack.top().prec()) {
                    //std::cout << "top of operator stack precedence > top: pushing " << input_stack.top().str() << " from input to operator" << std::endl;
                    operator_stack.push(input_stack.top());
                    input_stack.pop();
                } else {
                    while (!operator_stack.empty()) {
                        if (operator_stack.top().prec() > input_stack.top().prec() || (operator_stack.top().prec() == input_stack.top().prec() && operator_stack.top().asso() == LEFT) && operator_stack.top().str() != "(") {
                            //std::cout << "while !operator_stack.empty(): pushing " << operator_stack.top().str() << " from operator to output" << std::endl;
                            if (operator_stack.top().str() != "(") {output_stack.push(operator_stack.top());}
                            operator_stack.pop();
                            //std::cout << operator_stack.size() << std::endl;
                        } else {
                            break;
                        }
                    }
                    //std::cout << "precedence < top: pushing " << input_stack.top().str() << " from input to operator" << std::endl;
                    operator_stack.push(input_stack.top());
                    input_stack.pop();
                }
            }
        }
    }
    while (!operator_stack.empty()) {
        //std::cout << "doing the rest: pushing " << operator_stack.top().str() << " from operator to output" << std::endl;
        if (operator_stack.top().str() != "(") {output_stack.push(operator_stack.top());}
        operator_stack.pop();
    }
    return output_stack;
}
/*
int main() {/*
    Token a("3", TERMINAL);
    Token b("+", OPERATOR);
    Token c("4", TERMINAL);
    Token d("*", OPERATOR);
    Token e("2", TERMINAL);
    Token f("/", OPERATOR);
    Token g("(", OPERATOR);
    Token h("1", TERMINAL);
    Token i("-", OPERATOR);
    Token j("5", TERMINAL);
    Token k(")", OPERATOR);
    Token l("^", OPERATOR);
    Token m("2", TERMINAL);
    Token n("^", OPERATOR);
    Token o("3", TERMINAL);* /
    Token a("!", OPERATOR);
    Token b("(", OPERATOR);
    Token c("i", TERMINAL);
    Token d("!=", OPERATOR);
    Token e("true", TERMINAL);
    Token f(")", OPERATOR);
    std::stack<Token> input;// = { a, b, c, d, e, f, g, h, i, j, k, l, m, n, o };
    input.push(f);
    input.push(e);
    input.push(d);
    input.push(c);
    input.push(b);
    input.push(a);
    std::cout << "Infix: ";
    std::stack<Token> po = input;
    while (!po.empty()) {
        std::cout << po.top().str();
        po.pop();
    }
    std::stack<Token> postfix = shunting_yard_algorithm(input);
    std::cout << "\nPostfix: ";
    std::stack<std::string> zz;
    while (!postfix.empty()) {
        zz.push(postfix.top().str());
        postfix.pop();
    }
    while (!zz.empty()) {
        std::cout << zz.top();
        zz.pop();
    }
}*/