#include "database.h"

static const size_t kNullWidth = 4;

using namespace MyCoolNamespace;

std::vector<std::string> MyCoolDataBase::SplitString(const std::string& args, char sep) {
    std::regex separator(R"(\s*)" + ("\\" + std::string(1, sep)) + R"(\s*)");
    std::vector<std::string> result;
    std::sregex_token_iterator iter(args.begin(), args.end(), separator, -1);
    std::sregex_token_iterator end;
    while (iter != end) {
        result.push_back(*iter);
        ++iter;
    }
    return result;
}

void MyCoolDataBase::ToUpper(std::string& word) {
    for (size_t i = 0; i < word.size(); i++) {
        word[i] = std::toupper(word[i]);
    }
}

void MyCoolDataBase::Parse(const std::string& query) {
    std::string first_word;
    std::stringstream ss(query);
    ss >> first_word;
    ToUpper(first_word);

    if (first_word == "CREATE") {
        CreateTable(query);
    } else if (first_word == "DROP") {
        DropTable(query);
    } else if (first_word == "INSERT") {
        InsertInto(query);
    } else if (first_word == "SELECT") {
        SelectFrom(query);
    } else if (first_word == "DELETE") {
        DeleteFrom(query);
    } else if (first_word == "UPDATE") {
        Update(query);
    }
    else {
        throw QueryException(std::string("Invalid statement."));
    }
}

void MyCoolDataBase::CreateTable(const std::string& query) {
    static const std::regex pattern(R"(\s*CREATE\s+TABLE\s+(\w+)\s*\(\s*(.+)\s*\)\s*;\s*)",
                                    std::regex_constants::icase);
    static const std::regex foreign_key_pattern(R"(\s*FOREIGN\s+KEY\s*\((\w+)\)\s*REFERENCES\s+(\w+)\s*\(\s*(\w+)\s*\)\s*)", std::regex_constants::icase);
    std::smatch s;

    if (!std::regex_match(query, s, pattern)) {
        throw SyntaxErrorException("Invalid syntax of CREATE TABLE query");
    }
    std::string table_name = s[1].str();
    if (tables_.contains(table_name)) {
        throw LogicErrorException(std::string("Table already ") + table_name + std::string(" exists"));
    }
    std::vector<std::string> columns = SplitString(s[2].str(), ',');
    std::vector<std::string> keys_order;
    std::unordered_map<std::string, Table::ColumnInfo> keys;
    std::string primary_key;

    for (auto column: columns) {
        if (std::regex_match(column, s, foreign_key_pattern)) {
            auto column_name = s[1].str();
            auto foreign_table_name = s[2].str();
            auto foreign_column_name = s[3].str();
            DeleteSpaces(column_name);
            DeleteSpaces(foreign_table_name);
            DeleteSpaces(foreign_column_name);
            if (!keys.contains(column_name) || !tables_.contains(foreign_table_name)) {
                throw LogicErrorException("Invalid foreign key.");
            }
            if (!tables_[foreign_table_name].IsColumn(foreign_column_name)) {
                throw LogicErrorException("Foreign key is not a column.");
            }
            if (keys[column_name].type != tables_[foreign_table_name].ColumnType(foreign_column_name)) {
                throw LogicErrorException("Not equal types of foreign keys.");
            }
            keys[column_name].foreign_key = std::make_pair(&tables_[foreign_table_name], tables_[foreign_table_name].ColumnNumber(foreign_column_name));
            continue;
        }
        auto column_words = SplitString(column, ' ');
        if (column_words.empty()) {
            throw SyntaxErrorException("Empty argument.");
        }
        if (column_words.size() % 2 == 1) {
            throw SyntaxErrorException("Not enough arguments.");
        }
        std::string key;
        bool primary = false;
        bool not_null = false;
        TYPE type = NONE;
        for (size_t i = 0; i < column_words.size() / 2; i++) {
            std::string line = column_words[2 * i] + column_words[2 * i + 1];
            ToUpper(line);
            if (line == "NOTNULL") {
                not_null = true;
            } else if (line == "PRIMARYKEY" && primary_key.empty()) {
                primary = true;
            } else if (line == "PRIMARYKEY" && !primary_key.empty()) {
                throw LogicErrorException(std::string("Multiple primary keys."));
            } else if (key.empty()) {
                key = column_words[2 * i];
                ToUpper(column_words[2 * i + 1]);
                if (column_words[2 * i + 1] == "BOOL") {
                    type = BOOL;
                } else if (column_words[2 * i + 1] == "INT") {
                    type = INT;
                } else if (column_words[2 * i + 1] == "FLOAT") {
                    type = FLOAT;
                } else if (column_words[2 * i + 1] == "DOUBLE") {
                    type = DOUBLE;
                } else {
                    static const std::regex varchar_pattern(R"(VARCHAR\((\d+)\))");
                    std::smatch varchar_s;
                    if (!std::regex_match(column_words[2 * i + 1], varchar_s, varchar_pattern)) {
                        throw SyntaxErrorException(std::string("Invalid type."));
                    }
                    type = VARCHAR;
                }
            } else {
                throw SyntaxErrorException("Invalid argument.");
            }
        }
        if (key.empty()) {
            throw SyntaxErrorException("Empty key.");
        }
        if (primary) {
            primary_key = key;
        }
        if (keys.contains(key)) {
            throw LogicErrorException(std::string("Key ") + key + std::string(" already exists."));
        }
        keys[key] = Table::ColumnInfo(keys_order.size(), type, not_null, std::max(key.size(), kNullWidth));
        keys_order.push_back(key);
    }
    if (primary_key.empty()) {
        primary_key = keys_order[0];
    }
    tables_[table_name] = Table(keys, keys_order, primary_key);
}

void MyCoolDataBase::PrintTablesList() {
    for (auto pair: tables_) {
        std::cout << pair.first << ' ';
    }
    std::cout << '\n';
}

void MyCoolDataBase::DropTable(const std::string& query) {
    static const std::regex pattern(R"(\s*DROP\s+TABLE\s+(\w+)\s*;\s*)", std::regex_constants::icase);
    std::smatch s;

    if (!std::regex_match(query, s, pattern)) {
        throw SyntaxErrorException("Invalid syntax of DROP TABLE query");
    }
    std::string table_name = s[1].str();

    if (!tables_.contains(table_name)) {
        throw LogicErrorException(std::string("Table ") + table_name + std::string(" doesn't exists."));
    }
    tables_.erase(table_name);
    std::cout << "Dropped table " << table_name << ". ";
    if (tables_.empty()) {
        std::cout << "There are no remaining tables.\n";
    } else {
        std::cout << "Remaining tables:\n";
        PrintTablesList();
    }
    std::cout << '\n';
}

void MyCoolDataBase::InsertInto(const std::string& query) {
    static const std::regex pattern_w_columns(
            R"(\s*INSERT\s+INTO\s+(\w+)\s*\(\s*(.+)\s*\)\s*VALUES\s*\(\s*(.*)\s*\)\s*;\s*)",
            std::regex_constants::icase);
    static const std::regex pattern_wout_columns(R"(\s*INSERT\s+INTO\s+(\w+)\s+VALUES\s*\(\s*(.*)\s*\)\s*;\s*)",
                                                 std::regex_constants::icase);
    std::smatch s1;
    std::smatch s2;
    if (std::regex_match(query, s1, pattern_w_columns)) {
        auto table_name = s1[1].str();
        if (!tables_.contains(table_name)) {
            throw LogicErrorException(std::string("Table ") + table_name + std::string(" doesn't exists."));
        }
        auto keys = SplitString(s1[2].str(), ',');
        auto v = s1[3].str();
        DeleteSpaces(v);
        auto values = SplitString(v, ',');
        tables_[table_name].AddRow(values, keys);
        //std::cout << tables_[table_name];
    } else if (std::regex_match(query, s2, pattern_wout_columns)) {
        auto table_name = s2[1].str();
        if (!tables_.contains(table_name)) {
            throw LogicErrorException(std::string("Table ") + table_name + std::string(" doesn't exists."));
        }
        auto v = s2[2].str();
        DeleteSpaces(v);
        auto values = SplitString(v, ',');
        tables_[table_name].AddRow(values);
        //std::cout << tables_[table_name];
    } else {
        throw SyntaxErrorException("Invalid syntax of INSERT INTO query.");
    }
}

void MyCoolDataBase::SelectFrom(const std::string& query) {
    static const std::regex pattern_wout_where(R"(\s*SELECT\s+(.+)\s+FROM\s+(\w+)\s*;\s*)",std::regex_constants::icase);
    static const std::regex pattern_w_where(R"(\s*SELECT\s+(.+)\s+FROM\s+(\w+)\s+WHERE\s+(.+)\s*;\s*)",std::regex_constants::icase);
    static const std::regex inner_join_wout_where(R"(\s*SELECT\s+(.+)\s+FROM\s+(\w+)\s+(\w+)\s+JOIN\s+(\w+)\s+ON\s+(\w+\.\w+)\s*=\s*(\w+\.\w+)\s*;\s*)", std::regex_constants::icase);
    static const std::regex inner_join_w_where(R"(\s*SELECT\s+(.+)\s+FROM\s+(\w+)\s+(\w+)\s+JOIN\s+(\w+)\s+ON\s+(\w+\.\w+)\s*=\s*(\w+\.\w+)\s+WHERE\s+(.+)\s*;\s*)", std::regex_constants::icase);
    std::smatch s;
    if (std::regex_match(query, s, pattern_w_where)) {
        auto table_name = s[2].str();
        if (!tables_.contains(table_name)) {
            throw LogicErrorException(std::string("Table ") + table_name + std::string(" doesn't exists."));
        }
        auto condition_str = s[3].str();
        DeleteSpaces(condition_str);
        auto condition = InfixToPostfix::Parse(condition_str);
        auto keys_str = s[1].str();
        DeleteSpaces(keys_str);
        if (keys_str == "*") {
            tables_[table_name].PrintWithCondition(condition);
        } else {
            auto keys = SplitString(keys_str, ',');
            tables_[table_name].PrintWithCondition(keys, condition);
        }
    } else if (std::regex_match(query, s, pattern_wout_where)) {
        auto table_name = s[2].str();
        if (!tables_.contains(table_name)) {
            throw LogicErrorException(std::string("Table ") + table_name + std::string(" doesn't exists."));
        }
        auto keys_str = s[1].str();
        DeleteSpaces(keys_str);
        if (keys_str == "*") {
            tables_[table_name].PrintWithoutCondition();
        } else {
            auto keys = SplitString(keys_str, ',');
            tables_[table_name].PrintWithoutCondition(keys);
        }
    } else if (std::regex_match(query, s, inner_join_wout_where)) {
        auto join_type = s[3].str();
        ToUpper(join_type);
        auto table1_name = s[2].str();
        auto table2_name = s[4].str();
        if (!tables_.contains(table1_name)) {
            throw LogicErrorException(std::string("Table ") + table1_name + std::string(" doesn't exists."));
        }
        if (!tables_.contains(table2_name)) {
            throw LogicErrorException(std::string("Table ") + table2_name + std::string(" doesn't exists."));
        }
        auto column_str1 = s[5].str();
        auto column_str2 = s[6].str();
        DeleteSpaces(column_str1);
        DeleteSpaces(column_str2);
        auto column1 = SplitString(column_str1, '.');
        auto column2 = SplitString(column_str2, '.');
        std::string table1_column, table2_column;
        if (column1[0] == table1_name && column2[0] == table2_name) {
            table1_column = column1[1];
            table2_column = column2[1];
        } else if (column1[0] == table2_name && column2[0] == table1_name) {
            table1_column = column2[1];
            table2_column = column1[1];
        } else {
            throw LogicErrorException("Invalid keys on INNER JOIN expr");
        }
        auto keys_str = s[1].str();
        DeleteSpaces(keys_str);

        if (keys_str == "*") {
            Table joined;
            if (join_type == "INNER") {
                joined = InnerJoin(tables_[table1_name], tables_[table2_name], table1_column, table2_column);
            } else if (join_type == "LEFT") {
                joined = LeftJoin(tables_[table1_name], tables_[table2_name], table1_column, table2_column);
            } else if (join_type == "RIGHT") {
                joined = LeftJoin(tables_[table2_name], tables_[table1_name], table2_column, table1_column);
            } else {
                throw SyntaxErrorException("Invalid type of JOIN.");
            }
            joined.PrintWithoutCondition();
        } else {
            auto keys = SplitString(keys_str, ',');
            std::vector<std::string> columns;
            for (auto key : keys) {
                auto column = SplitString(key, '.');
                if (column[0] != table1_name && column[0] != table2_name || !tables_[column[0]].IsColumn(column[1])) {
                    throw LogicErrorException("Invalid column key(s).");
                }
                columns.push_back(column[1]);
            }
            Table joined;
            if (join_type == "INNER") {
                joined = InnerJoin(tables_[table1_name], tables_[table2_name], table1_column, table2_column);
            } else if (join_type == "LEFT") {
                joined = LeftJoin(tables_[table1_name], tables_[table2_name], table1_column, table2_column);
            } else if (join_type == "RIGHT") {
                joined = LeftJoin(tables_[table2_name], tables_[table1_name], table2_column, table1_column);
            } else {
                throw SyntaxErrorException("Invalid type of JOIN.");
            }
            joined.PrintWithoutCondition(columns);
        }
    } else if (std::regex_match(query, s, inner_join_w_where)) {
        auto join_type = s[3].str();
        ToUpper(join_type);
        auto table1_name = s[2].str();
        auto table2_name = s[4].str();
        if (!tables_.contains(table1_name)) {
            throw LogicErrorException(std::string("Table ") + table1_name + std::string(" doesn't exists."));
        }
        if (!tables_.contains(table2_name)) {
            throw LogicErrorException(std::string("Table ") + table2_name + std::string(" doesn't exists."));
        }
        auto column_str1 = s[5].str();
        auto column_str2 = s[6].str();
        DeleteSpaces(column_str1);
        DeleteSpaces(column_str2);
        auto column1 = SplitString(column_str1, '.');
        auto column2 = SplitString(column_str2, '.');
        std::string table1_column, table2_column;
        if (column1[0] == table1_name && column2[0] == table2_name) {
            table1_column = column1[1];
            table2_column = column2[1];
        } else if (column1[0] == table2_name && column2[0] == table1_name) {
            table1_column = column2[1];
            table2_column = column1[1];
        } else {
            throw LogicErrorException("Invalid keys on INNER JOIN expr");
        }
        auto condition_str = s[7].str();
        DeleteSpaces(condition_str);
        auto condition = InfixToPostfix::Parse(condition_str);
        auto keys_str = s[1].str();
        DeleteSpaces(keys_str);
        if (keys_str == "*") {
            Table joined;
            if (join_type == "INNER") {
                joined = InnerJoin(tables_[table1_name], tables_[table2_name], table1_column, table2_column);
            } else if (join_type == "LEFT") {
                joined = LeftJoin(tables_[table1_name], tables_[table2_name], table1_column, table2_column);
            } else if (join_type == "RIGHT") {
                joined = LeftJoin(tables_[table2_name], tables_[table1_name], table2_column, table1_column);
            } else {
                throw SyntaxErrorException("Invalid type of JOIN.");
            }
            joined.PrintWithCondition(condition);
        } else {
            auto keys = SplitString(keys_str, ',');
            std::vector<std::string> columns;
            for (auto key : keys) {
                auto column = SplitString(key, '.');
                if (column[0] != table1_name && column[0] != table2_name || !tables_[column[0]].IsColumn(column[1])) {
                    throw LogicErrorException("Invalid column key(s).");
                }
                columns.push_back(column[1]);
            }
            Table joined;
            if (join_type == "INNER") {
                joined = InnerJoin(tables_[table1_name], tables_[table2_name], table1_column, table2_column);
            } else if (join_type == "LEFT") {
                joined = LeftJoin(tables_[table1_name], tables_[table2_name], table1_column, table2_column);
            } else if (join_type == "RIGHT") {
                joined = LeftJoin(tables_[table2_name], tables_[table1_name], table2_column, table1_column);
            } else {
                throw SyntaxErrorException("Invalid type of JOIN.");
            }
            joined.PrintWithCondition(columns, condition);
        }
    }
    else {
        throw SyntaxErrorException("Invalid syntax of SELECT query.");
    }
    std::cout << '\n';
}

void MyCoolDataBase::DeleteFrom(const std::string& query) {
    static const std::regex pattern_w_where(R"(\s*DELETE\s+FROM\s+(\w+)\s+WHERE\s+(.+)\s*;\s*)",
                                            std::regex_constants::icase);
    static const std::regex pattern_wout_where(R"(\s*DELETE\s+FROM\s+(\w+)\s*;\s*)", std::regex_constants::icase);
    std::smatch s;
    if (std::regex_match(query, s, pattern_w_where)) {
        auto table_name = s[1].str();
        if (!tables_.contains(table_name)) {
            throw LogicErrorException(std::string("Table ") + table_name + std::string(" doesn't exists."));
        }
        auto condition_str = s[2].str();
        DeleteSpaces(condition_str);
        auto condition = InfixToPostfix::Parse(condition_str);
        tables_[table_name].DeleteRows(condition);
    } else if (std::regex_match(query, s, pattern_wout_where)) {
        auto table_name = s[1].str();
        if (!tables_.contains(table_name)) {
            throw LogicErrorException(std::string("Table ") + table_name + std::string(" doesn't exists."));
        }
        tables_[table_name].DeleteRows({"true"});
    } else {
        throw SyntaxErrorException("Invalid syntax of DELETE FROM query.");
    }
}

void MyCoolDataBase::Update(const std::string& query) {
    static const std::regex pattern_w_where(R"(\s*UPDATE\s+(\w+)\s+SET\s+(.+)\s+WHERE\s+(.+)\s*;\s*)", std::regex_constants::icase);
    static const std::regex pattern_wout_where(R"(\s*UPDATE\s+(\w+)\s+SET\s+(.+)\s*;\s*)", std::regex_constants::icase);
    std::smatch s;
    if (std::regex_match(query, s, pattern_w_where)) {
        auto table_name = s[1].str();
        if (!tables_.contains(table_name)) {
            throw LogicErrorException(std::string("Table ") + table_name + std::string(" doesn't exists."));
        }
        auto upd_str = s[2].str();
        DeleteSpaces(upd_str);
        auto upd = SplitString(upd_str, ',');
        std::vector<std::pair<std::string, std::string>> pairs;
        for (auto p : upd) {
            auto pair = SplitString(p, '=');
            if (pair.size() != 2) {
                throw SyntaxErrorException("Bad syntax.");
            }
            pairs.push_back(std::make_pair(pair[0], pair[1]));
        }
        auto condition_str = s[3].str();
        DeleteSpaces(condition_str);
        auto condition = InfixToPostfix::Parse(condition_str);
        tables_[table_name].UpdateValues(pairs, condition);
    } else if (std::regex_match(query, s, pattern_wout_where)) {
        auto table_name = s[1].str();
        if (!tables_.contains(table_name)) {
            throw LogicErrorException(std::string("Table ") + table_name + std::string(" doesn't exists."));
        }
        auto upd_str = s[2].str();
        DeleteSpaces(upd_str);
        auto upd = SplitString(upd_str, ',');
        std::vector<std::pair<std::string, std::string>> pairs;
        for (auto p : upd) {
            auto pair = SplitString(p, '=');
            if (pair.size() != 2) {
                throw SyntaxErrorException("Bad syntax.");
            }
            pairs.push_back(std::make_pair(pair[0], pair[1]));
        }
        tables_[table_name].UpdateValues(pairs);
    } else {
        throw SyntaxErrorException("Invalid syntax of UPDATE query.");
    }
}

void MyCoolDataBase::SaveDB(const std::string& path) {
    std::ofstream file;
    file.open(path);
    if (!file.is_open()) {
        throw WriteFileException("Invalid path to file.");
    }
    file << name_ << '\n';
    file << tables_.size() << '\n';
    for (auto table : tables_) {
        file << table.first << '\n';
        table.second.WriteToFile(file);
    }
}

std::vector<std::string> SplitStringSep(const std::string& args, std::string sep) {
    std::regex separator(sep);
    std::vector<std::string> result;
    std::sregex_token_iterator iter(args.begin(), args.end(), separator, -1);
    std::sregex_token_iterator end;
    while (iter != end) {
        result.push_back(*iter);
        ++iter;
    }
    return result;
}

void MyCoolDataBase::LoadDB(const std::string& path) {
    std::ifstream file;
    file.open(path);
    if (!file.is_open()) {
        throw WriteFileException("Invalid path to file.");
    }
    tables_.clear();
    std::getline(file, name_);
    size_t tables_count;
    std::string linee;
    std::getline(file, linee);
    tables_count = std::stoull(linee);
    for (size_t i = 0; i < tables_count; ++i) {
        std::string table_name;
        file >> table_name;
        size_t columns_count;
        file >> columns_count;
        std::vector<std::string> keys_order;
        std::unordered_map<std::string, Table::ColumnInfo> keys;
        for (size_t j = 0; j < columns_count; ++j) {
            std::string column_name;
            file >> column_name;
            size_t number, width;
            int type_int;
            TYPE type;
            bool not_null;
            file >> number >> width >> type_int >> not_null;
            type = static_cast<TYPE>(type_int);
            Table::ColumnInfo info(number, type, not_null, width);
            keys_order.push_back(column_name);
            keys[column_name] = info;
        }
        std::string primary_key;
        Table table(keys, keys_order, primary_key);
        file >> primary_key;
        size_t rows_count;
        file >> rows_count;
        std::getline(file, linee);
        for (size_t j = 0; j < rows_count; ++j) {
            std::string line;
            std::getline(file, line);
            DeleteSpaces(line);
            auto split = SplitStringSep(line, std::string(R"('\s')"));
            split.back().pop_back();
            split.front() = split.front().substr(1, split.front().size() - 1);
            table.AddRow(split);
        }
        tables_[table_name] = table;
    }
}