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