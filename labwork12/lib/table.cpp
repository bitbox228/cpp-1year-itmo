#include "table.h"

using namespace MyCoolNamespace;

std::string ToUpper(const std::string& word) {
    std::string new_word;
    for (size_t i = 0; i < word.size(); i++) {
        new_word += std::toupper(word[i]);
    }
    return new_word;
}

static bool IsBool(const std::string& str) {
    if (str == "1" || str == "0") {
        return true;
    }
    auto str_upper = ToUpper(str);
    if (str_upper == "TRUE" || str_upper == "FALSE") {
        return true;
    }
    return false;
}

static bool ParseBool(const std::string& str) {
    if (str == "1") {
        return true;
    } else if (str == "0") {
        return false;
    }
    auto str_upper = ToUpper(str);
    if (str_upper == "TRUE") {
        return true;
    }
    return false;
}

static bool IsInt(const std::string& str) {
    static const std::regex pattern(R"((-|\+)?\d+)");
    return std::regex_match(str, pattern);
}

static bool IsFloat(const std::string& str) {
    static const std::regex pattern(R"((-|\+)?\d+(.?\d+))");
    return std::regex_match(str, pattern);
}

static bool IsNull(const std::string& str) {
    auto str_upper = ToUpper(str);
    return (str_upper == "NULL");
}

void Table::AddRow(const std::vector<std::string>& values, const std::vector<std::string>& keys) {
    if (values.size() != keys.size()) {
        throw LogicErrorException("Counts of values and columns aren't equal.");
    }
    std::vector<std::string> parsed_values;
    ParseValues(values, parsed_values);
    Tuple row(keys_order_.size(), std::nullopt);
    for (size_t i = 0; i < keys.size(); i++) {
        if (!keys_.contains(keys[i])) {
            throw LogicErrorException("Nonexistent key.");
        }
        auto type = keys_[keys[i]].type;
        if (IsNull(parsed_values[i])) {
            if (keys_[keys[i]].not_null) {
                throw LogicErrorException("NOT NULL value can't be NULL.");
            }
            continue;
        }
        switch (type) {
            case BOOL:
                if (!IsBool(parsed_values[i])) {
                    throw SyntaxErrorException("Invalid BOOL value.");
                }
                keys_[keys[i]].width = std::max(keys_[keys[i]].width, parsed_values[i].size());
                row[keys_[keys[i]].number] = ParseBool(parsed_values[i]);
                break;
            case INT:
                if (!IsInt(parsed_values[i])) {
                    throw SyntaxErrorException("Invalid INT value.");
                }
                keys_[keys[i]].width = std::max(keys_[keys[i]].width, parsed_values[i].size());
                row[keys_[keys[i]].number] = std::stoi(parsed_values[i]);
                break;
            case FLOAT:
            case DOUBLE:
                if (!IsFloat(parsed_values[i])) {
                    throw SyntaxErrorException("Invalid FLOAT value.");
                }
                keys_[keys[i]].width = std::max(keys_[keys[i]].width, parsed_values[i].size());
                row[keys_[keys[i]].number] = std::stod(parsed_values[i]);
                break;
            case VARCHAR:
                keys_[keys[i]].width = std::max(keys_[keys[i]].width, parsed_values[i].size());
                row[keys_[keys[i]].number] = parsed_values[i];
                break;
        }
        if (with_primary && primary_key_ == keys[i]) {
            for (auto roww : rows_) {
                if (roww[keys_[keys[i]].number] == row[keys_[keys[i]].number]) {
                    throw LogicErrorException("Primary key repetition");
                }
            }
        }
    }
    for (size_t i = 0; i < row.size(); i++) {
        if (row[i] == std::nullopt && keys_[keys_order_[i]].not_null) {
            throw LogicErrorException("NOT NULL value can't be NULL.");
        }
    }
    rows_.push_back(row);
}

void Table::AddRow(const std::vector<std::string>& values) {
    AddRow(values, keys_order_);
}

void Table::ParseValues(const std::vector<std::string>& values, std::vector<std::string>& parsed_values) {
    for (auto it = values.begin(); it != values.end(); it++) {
        if (it->front() == it->back() && it->front() == '\'' || it->front() == '\"') {
            parsed_values.push_back(it->substr(1, it->size() - 2));
        } else {
            parsed_values.push_back(*it);
        }
    }
}

std::ostream& MyCoolNamespace::operator<<(std::ostream& os, Table& table) {
    for (auto it = table.keys_order_.begin(); it != table.keys_order_.end(); it++) {
        os << std::left << std::setw(table.keys_[*it].width) << *it << ' ';
    }
    os << std::endl;
    for (size_t i = 0; i < table.rows_.size(); i++) {
        for (size_t j = 0; j < table.rows_[i].size(); j++) {
            std::string output;
            if (table.rows_[i][j] == std::nullopt) {
                output = "NULL";
            } else {
                switch (table.keys_[table.keys_order_[j]].type) {
                    case BOOL:
                        output = (std::get<bool>(*table.rows_[i][j])) ? "TRUE" : "FALSE";
                        break;
                    case INT:
                        output = std::to_string(std::get<int>(*table.rows_[i][j]));
                        break;
                    case FLOAT:
                        output = std::to_string(std::get<float>(*table.rows_[i][j]));
                        break;
                    case DOUBLE:
                        output = std::to_string(std::get<double>(*table.rows_[i][j]));
                        break;
                    case VARCHAR:
                        output = std::get<std::string>(*table.rows_[i][j]);
                        break;
                }
            }
            os << std::left << std::setw(table.keys_[table.keys_order_[j]].width) << output << ' ';
        }
        os << std::endl;
    }
    return os;
}


void Table::PrintWithCondition(const std::vector<std::string>& keys, const std::vector<std::string>& condition) {
    for (auto key: keys) {
        if (!keys_.contains(key)) {
            throw LogicErrorException("Nonexistent key.");
        }
    }
    std::vector<Tuple> accepting_rows;
    for (auto row: rows_) {
        if (IsRowAccepting(row, condition)) {
            accepting_rows.push_back(row);
        }
    }
    for (auto key: keys) {
        std::cout << std::left << std::setw(keys_[key].width) << key << ' ';
    }
    std::cout << std::endl;
    for (auto row: accepting_rows) {
        for (auto key: keys) {
            std::string output;
            if (row[keys_[key].number] == std::nullopt) {
                output = "NULL";
            } else {
                switch (keys_[key].type) {
                    case BOOL:
                        output = (std::get<bool>(*row[keys_[key].number])) ? "TRUE" : "FALSE";
                        break;
                    case INT:
                        output = std::to_string(std::get<int>(*row[keys_[key].number]));
                        break;
                    case FLOAT:
                        output = std::to_string(std::get<float>(*row[keys_[key].number]));
                        break;
                    case DOUBLE:
                        output = std::to_string(std::get<double>(*row[keys_[key].number]));
                        break;
                    case VARCHAR:
                        output = std::get<std::string>(*row[keys_[key].number]);
                        break;
                }
            }
            std::cout << std::left << std::setw(keys_[key].width) << output << ' ';
        }
        std::cout << std::endl;
    }
}

void MyCoolNamespace::DeleteSpaces(std::string& str) {
    while (str.back() == ' ') {
        str.pop_back();
    }
}

bool Table::IsRowAccepting(const Tuple& row, const std::vector<std::string> condition) {
    std::stack<std::string> stack;
    static const std::regex pattern1(R"((.+)\s*(>=|<=|<>|!=|IS)\s*(.+))", std::regex_constants::icase);
    static const std::regex pattern2(R"((.+)\s*(>|<|=)\s*(.+))", std::regex_constants::icase);
    std::smatch s;
    for (auto str: condition) { /// =, >, <, >=, <=, <>, !=, IS
        if (str == "NOT") {
            if (stack.empty()) {
                throw SyntaxErrorException("Invalid logic expression.");
            }
            auto expr = stack.top();
            stack.pop();
            if (!std::regex_match(expr, s, pattern1)) {
                if (!std::regex_match(expr, s, pattern2)) {
                    auto res = GetValueFromUnaryLogicExpr(expr);
                    if (res == std::nullopt) {
                        throw SyntaxErrorException("Invalid logic expression.");
                    }
                    if (res) {
                        stack.push("false");
                    } else {
                        stack.push("true");
                    }
                    continue;
                }
                auto s1_str = s[1].str();
                DeleteSpaces(s1_str);
                bool val = GetValueFromLogicExpr(s1_str, s[3].str(), ToUpper(s[2].str()), row);
                if (val) {
                    stack.push("false");
                } else {
                    stack.push("true");
                }
            }
            auto s1_str = s[1].str();
            DeleteSpaces(s1_str);
            bool val = GetValueFromLogicExpr(s1_str, s[3].str(), ToUpper(s[2].str()), row);
            if (val) {
                stack.push("false");
            } else {
                stack.push("true");
            }
        } else if (str == "AND" || str == "OR") {
            if (stack.empty()) {
                throw SyntaxErrorException("Invalid logic expression.");
            }
            auto expr1 = stack.top();
            bool res1;
            stack.pop();
            if (stack.empty()) {
                throw SyntaxErrorException("Invalid logic expression.");
            }
            auto expr2 = stack.top();
            bool res2;
            stack.pop();
            if (!std::regex_match(expr1, s, pattern1)) {
                if (!std::regex_match(expr1, s, pattern2)) {
                    auto res = GetValueFromUnaryLogicExpr(expr1);
                    if (res == std::nullopt) {
                        throw SyntaxErrorException("Invalid logic expression.");
                    }
                    res1 = *res;
                } else {
                    auto s1_str = s[1].str();
                    DeleteSpaces(s1_str);
                    res1 = GetValueFromLogicExpr(s1_str, s[3].str(), ToUpper(s[2].str()), row);
                }
            } else {
                auto s1_str = s[1].str();
                DeleteSpaces(s1_str);
                res1 = GetValueFromLogicExpr(s1_str, s[3].str(), ToUpper(s[2].str()), row);
            }
            if (!std::regex_match(expr2, s, pattern1)) {
                if (!std::regex_match(expr2, s, pattern2)) {
                    auto res = GetValueFromUnaryLogicExpr(expr2);
                    if (res == std::nullopt) {
                        throw SyntaxErrorException("Invalid logic expression.");
                    }
                    res2 = *res;
                } else {
                    auto s1_str = s[1].str();
                    DeleteSpaces(s1_str);
                    res2 = GetValueFromLogicExpr(s1_str, s[3].str(), ToUpper(s[2].str()), row);
                }
            } else {
                auto s1_str = s[1].str();
                DeleteSpaces(s1_str);
                res2 = GetValueFromLogicExpr(s1_str, s[3].str(), ToUpper(s[2].str()), row);
            }
            if (str == "AND") {
                if (res1 && res2) {
                    stack.push("true");
                } else {
                    stack.push("false");
                }
            } else {
                if (res1 || res2) {
                    stack.push("true");
                } else {
                    stack.push("false");
                }
            }
        } else {
            stack.push(str);
        }
    }
    if (stack.size() != 1) {
        throw SyntaxErrorException("Invalid logic expression.");
    }
    auto expr = stack.top();
    stack.pop();
    if (!std::regex_match(expr, s, pattern1)) {
        if (!std::regex_match(expr, s, pattern2)) {
            auto res = GetValueFromUnaryLogicExpr(expr);
            if (res == std::nullopt) {
                throw SyntaxErrorException("Invalid logic expression.");
            }
            return *res;
        } else {
            auto s1_str = s[1].str();
            DeleteSpaces(s1_str);
            return GetValueFromLogicExpr(s1_str, s[3].str(), ToUpper(s[2].str()), row);
        }
    }
    auto s1_str = s[1].str();
    DeleteSpaces(s1_str);
    return GetValueFromLogicExpr(s1_str, s[3].str(), ToUpper(s[2].str()), row);
}

bool
Table::GetValueFromLogicExpr(const std::string& lhs, const std::string& rhs, const std::string& op, const Tuple& row) {
    bool is_lhs_column;
    bool is_rhs_column;
    auto lhs_p = lhs;
    auto rhs_p = rhs;
    if (lhs.front() == lhs.back() && lhs.front() == '\'' || lhs.front() == '\"') {
        lhs_p = lhs.substr(1, lhs.size() - 2);
    }
    if (rhs.front() == rhs.back() && rhs.front() == '\'' || rhs.front() == '\"') {
        rhs_p = rhs.substr(1, rhs.size() - 2);
    }
    if (keys_.contains(lhs_p)) {
        is_lhs_column = true;
    } else {
        is_lhs_column = false;
    }
    if (keys_.contains(rhs_p)) {
        is_rhs_column = true;
    } else {
        is_rhs_column = false;
    }
    Value lhs_v;
    Value rhs_v;
    if (is_lhs_column) {
        lhs_v = row[keys_[lhs_p].number];
    } else {
        if (IsInt(lhs_p)) {
            lhs_v = std::stoi(lhs_p);
        } else if (IsNull(lhs_p)) {
            lhs_v = std::nullopt;
        } else if (IsBool(lhs_p)) {
            lhs_v = ParseBool(lhs_p);
        } else if (IsFloat(lhs_p)) {
            lhs_v = std::stod(lhs_p);
        } else {
            lhs_v = lhs_p;
        }
    }
    if (is_rhs_column) {
        rhs_v = row[keys_[rhs_p].number];
    } else {
        if (IsInt(rhs_p)) {
            rhs_v = std::stoi(rhs_p);
        } else if (IsNull(rhs_p)) {
            rhs_v = std::nullopt;
        } else if (IsBool(rhs_p)) {
            rhs_v = ParseBool(rhs_p);
        } else if (IsFloat(rhs_p)) {
            rhs_v = std::stod(rhs_p);
        } else {
            rhs_v = rhs_p;
        }
    }
    if (op == "=" || op == "IS") {
        return lhs_v == rhs_v;
    } else if (op == "<>" || op == "!=") {
        return lhs_v != rhs_v;
    } else if (op == ">") {
        return lhs_v > rhs_v;
    } else if (op == ">=") {
        return lhs_v >= rhs_v;
    } else if (op == "<") {
        return lhs_v < rhs_v;
    } else if (op == "<=") {
        return lhs_v <= rhs_v;
    }
    return false;
}

std::optional<bool> Table::GetValueFromUnaryLogicExpr(const std::string& expr) const {
    std::string exprr;
    bool quotes = false;
    if (expr.front() == expr.back() && expr.front() == '\'' || expr.front() == '\"') {
        exprr = expr.substr(1, expr.size() - 2);
        quotes = true;
    } else {
        exprr = expr;
    }
    if (IsBool(exprr)) {
        return ParseBool(exprr);
    } else if (IsInt(exprr)) {
        return static_cast<bool>(std::stoi(exprr));
    } else if (IsFloat(exprr)) {
        return static_cast<bool>(std::stod(exprr));
    } else if (quotes) {
        return false;
    }
    return std::nullopt;
}

void Table::PrintWithoutCondition(const std::vector<std::string>& keys) {
    PrintWithCondition(keys, {"true"});
}

void Table::PrintWithCondition(const std::vector<std::string>& condition) {
    PrintWithCondition(keys_order_, condition);
}

void Table::PrintWithoutCondition() {
    PrintWithCondition(keys_order_, {"true"});
}

void Table::DeleteRows(const std::vector<std::string>& condition) {
    std::set<Tuple> to_delete;
    for (Tuple row: rows_) {
        if (IsRowAccepting(row, condition)) {
            to_delete.insert(row);
        }
    }
    for (Tuple row: to_delete) {
        auto it = std::find(rows_.begin(), rows_.end(), row);
        rows_.erase(it);
    }
}

Table
MyCoolNamespace::InnerJoin(Table& lhs, Table& rhs, const std::string& left_column, const std::string& right_column) {
    if (!lhs.keys_.contains(left_column) || !rhs.keys_.contains(right_column)) {
        throw LogicErrorException("Invalid column(s).");
    }
    std::unordered_map<std::string, Table::ColumnInfo> keys;
    for (auto value: lhs.keys_) {
        keys.insert(value);
    }
    for (auto value: rhs.keys_) {
        value.second.number += lhs.keys_order_.size();
        if (!keys.contains(value.first)) {
            keys.insert(value);
        }
    }
    std::vector<std::string> keys_order;
    for (auto value: lhs.keys_order_) {
        keys_order.push_back(value);
    }
    for (auto value: rhs.keys_order_) {
        keys_order.push_back(value);
    }
    std::string primary_key;
    Table joined(keys, keys_order, primary_key);
    for (auto row1: lhs.rows_) {
        for (auto row2: rhs.rows_) {
            if (row1[lhs.keys_[left_column].number] == row2[rhs.keys_[right_column].number]) {
                Tuple new_row;
                for (auto value: row1) {
                    new_row.push_back(value);
                }
                for (auto value: row2) {
                    new_row.push_back(value);
                }
                joined.rows_.push_back(new_row);
            }
        }
    }
    return joined;
}

bool Table::IsColumn(const std::string& key) {
    return keys_.contains(key);
}

Table
MyCoolNamespace::LeftJoin(Table& lhs, Table& rhs, const std::string& left_column, const std::string& right_column) {
    if (!lhs.keys_.contains(left_column) || !rhs.keys_.contains(right_column)) {
        throw LogicErrorException("Invalid column(s).");
    }
    std::unordered_map<std::string, Table::ColumnInfo> keys;
    for (auto value: lhs.keys_) {
        keys.insert(value);
    }
    for (auto value: rhs.keys_) {
        value.second.number += lhs.keys_order_.size();
        if (!keys.contains(value.first)) {
            keys.insert(value);
        }
    }
    std::vector<std::string> keys_order;
    for (auto value: lhs.keys_order_) {
        keys_order.push_back(value);
    }
    for (auto value: rhs.keys_order_) {
        keys_order.push_back(value);
    }
    std::string primary_key;
    Table joined(keys, keys_order, primary_key);
    for (auto row1: lhs.rows_) {
        bool is_null = true;
        for (auto row2: rhs.rows_) {
            if (row1[lhs.keys_[left_column].number] == row2[rhs.keys_[right_column].number]) {
                is_null = false;
                Tuple new_row;
                for (auto value: row1) {
                    new_row.push_back(value);
                }
                for (auto value: row2) {
                    new_row.push_back(value);
                }
                joined.rows_.push_back(new_row);
            }
        }
        if (is_null) {
            Tuple new_row;
            for (auto value: row1) {
                new_row.push_back(value);
            }
            for (size_t i = 0; i < rhs.keys_order_.size(); i++) {
                new_row.push_back(std::nullopt);
            }
            joined.rows_.push_back(new_row);
        }
    }
    return joined;
}

void Table::UpdateValues(const std::vector<std::pair<std::string, std::string>>& values, const std::vector<std::string>& condition) {
    std::map<size_t, Value> new_values;
    for (auto pair : values) {
        if (!keys_.contains(pair.first)) {
            throw LogicErrorException("Nonexistent key.");
        }
        auto value_str = pair.second;
        if (value_str.front() == value_str.back() && (value_str.front() == '\'' || value_str.front() == '\"')) {
            value_str = value_str.substr(1, value_str.size() - 2);
        }
        Value value;
        if (IsNull(value_str)) {
            value = std::nullopt;
        } else {
            switch (keys_[pair.first].type) {
                case INT:
                    value = std::stoi(value_str);
                    break;
                case FLOAT:
                    value = std::stod(value_str);
                case DOUBLE:
                    value = std::stod(value_str);
                    break;
                case BOOL:
                    value = ParseBool(value_str);
                    break;
                case VARCHAR:
                    value = value_str;
                    break;
            }
        }
        if (new_values.contains(keys_[pair.first].number)) {
            throw LogicErrorException("Multiple keys in UPDATE.");
        }
        new_values[keys_[pair.first].number] = value;
    }
    for (auto& row : rows_) {
        if (IsRowAccepting(row, condition)) {
            for (auto pair : new_values) {
                row[pair.first] = pair.second;
            }
        }
    }
}

void Table::UpdateValues(const std::vector<std::pair<std::string, std::string>>& values) {
    UpdateValues(values, {"true"});
}

void Table::WriteToFile(std::ofstream& file) {
    file << keys_order_.size() <<'\n';
    for (auto key : keys_order_) {
        file << key << ' ' << keys_[key].number << ' ' << keys_[key].width  << ' ' << keys_[key].type << ' ' << keys_[key].not_null << '\n';
    }
    file << primary_key_ << '\n';
    file << rows_.size() << '\n';
    for (auto row : rows_) {
        size_t i = 0;
        for (auto val : row) {
            if (val == std::nullopt) {
                file << R"('NULL')";
            } else {
                switch (keys_[keys_order_[i]].type) {
                    case INT:
                        file << '\'' << std::get<int>(*val) << '\'' << ' ';
                        break;
                    case FLOAT:
                        file << '\'' << std::get<float>(*val) << '\'' << ' ';
                        break;
                    case DOUBLE:
                        file << '\'' << std::get<double>(*val) << '\'' << ' ';
                        break;
                    case BOOL:
                        file << '\'' << std::get<bool>(*val) << '\'' << ' ';
                        break;
                    case VARCHAR:
                        file << '\'' << std::get<std::string>(*val) << '\'' << ' ';
                        break;
                }
            }
            ++i;
        }
        file << '\n';
    }
}

TYPE Table::ColumnType(const std::string& key) {
    return keys_[key].type;
}

size_t Table::ColumnNumber(const std::string& key) {
    return keys_[key].number;
}
