#include "dbexcept.h"

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <stack>

class InfixToPostfix {

    static std::unordered_map<std::string, int> operations_priority_;
    static std::unordered_set<std::string> operations_;

    static void ToUpper(std::string& word);

    static size_t IsOperator(size_t pos, const std::string& logic_expr);

    static size_t IsNotOperator(size_t pos, const std::string& logic_expr);

    static std::string Parse(size_t& pos, const std::string& logic_expr, size_t len);

public:

    static std::vector<std::string> Parse(const std::string& logic_expr);
};
