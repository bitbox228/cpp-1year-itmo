#include "topostfix.h"

std::unordered_map<std::string, int> InfixToPostfix::operations_priority_ = {
        {"NOT", 3},
        {"AND", 2},
        {"OR",  1}
};

std::unordered_set<std::string> InfixToPostfix::operations_ = {"(", ")", "NOT", "AND", "OR"};

void InfixToPostfix::ToUpper(std::string& word) {
    for (size_t i = 0; i < word.size(); i++) {
        word[i] = std::toupper(word[i]);
    }
}

size_t InfixToPostfix::IsOperator(size_t pos, const std::string& logic_expr) {
    while (pos != logic_expr.size() && logic_expr[pos] == ' ') {
        ++pos;
    }
    std::string op;
    for (size_t i = pos; i < std::min(logic_expr.size(), pos + 3); i++) {
        op += toupper(logic_expr[i]);
        if (operations_.contains(op)) {
            return op.size();
        }
    }
    return 0;
}

size_t InfixToPostfix::IsNotOperator(size_t pos, const std::string& logic_expr) {
    std::string not_op;
    while (pos != logic_expr.size() && logic_expr[pos] == ' ') {
        ++pos;
    }
    while (pos != logic_expr.size()) {
        size_t size = IsOperator(pos, logic_expr);
        if (size != 0) {
            return not_op.size();
        }
        while (pos != logic_expr.size() && logic_expr[pos] == ' ') {
            ++pos;
            not_op += ' ';
        }
        not_op += logic_expr[pos++];
    }
    return not_op.size();
}

std::string InfixToPostfix::Parse(size_t& pos, const std::string& logic_expr, size_t len) {
    std::string word;
    while (pos != logic_expr.size() && logic_expr[pos] == ' ') {
        ++pos;
    }
    for (size_t i = 0; i < len; i++, pos++) {
        word += logic_expr[pos];
    }
    return word;
}

std::vector<std::string> InfixToPostfix::Parse(const std::string& logic_expr) {
    // TODO: CHECK FOR DOUBLE OP's + BRACKETS
    size_t pos = 0;
    std::stack<std::string> operators;
    std::vector<std::string> output;
    while (pos != logic_expr.size()) {
        size_t is_op = IsOperator(pos, logic_expr);
        if (is_op != 0) {
            std::string op = Parse(pos, logic_expr, is_op);
            ToUpper(op);
            if (op == "(") {
                operators.push(op);
            } else if (op == ")") {
                while (!operators.empty() && operators.top() != "(") {
                    output.push_back(operators.top());
                    operators.pop();
                }
                operators.pop();
            } else if (op == "NOT") {
                if (!operators.empty() && operators.top() == op) {
                    operators.pop();
                } else {
                    operators.push(op);
                }
            } else if (op == "AND" || op == "OR") {
                while (!operators.empty() && operations_priority_[operators.top()] >= operations_priority_[op]) {
                    output.push_back(operators.top());
                    operators.pop();
                }
                operators.push(op);
            }
            continue;
        }
        size_t is_not_op = IsNotOperator(pos, logic_expr);
        if (is_not_op != 0) {
            std::string not_op = Parse(pos, logic_expr, is_not_op);
            output.push_back(not_op);
            continue;
        }
        break;
    }
    while (!operators.empty()) {
        output.push_back(operators.top());
        operators.pop();
    }
    return output;
}