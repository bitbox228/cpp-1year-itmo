#include "tuple.h"
#include "dbexcept.h"
#include "topostfix.h"

#include <unordered_map>
#include <iomanip>
#include <regex>
#include <set>
#include <fstream>

namespace MyCoolNamespace {

    class Table {
    public:
        struct ColumnInfo {
            size_t number;
            TYPE type;
            bool not_null;
            size_t width;
            std::optional<std::pair<Table*, size_t>> foreign_key;

            ColumnInfo() = default;

            ColumnInfo(size_t number, TYPE type, bool not_null, size_t width,
                       std::optional<std::pair<Table*, size_t>> foreign_key = std::nullopt) : number(number),
                                                                                              type(type),
                                                                                              not_null(not_null),
                                                                                              width(width) {}
        };

    private:
        std::unordered_map<std::string, ColumnInfo> keys_;     /// keys with info about columns
        std::vector<std::string> keys_order_;                  /// order of keys in table
        std::string primary_key_;                              /// primary key - unique and !empty()
        std::vector<Tuple> rows_;                              /// all tuples (rows)
        bool with_primary = true;                              /// for joined tables

        static void ParseValues(const std::vector<std::string>& values, std::vector<std::string>& parsed_values);

        bool IsRowAccepting(const Tuple& row, const std::vector<std::string> condition);

        std::optional<bool> GetValueFromUnaryLogicExpr(const std::string& expr) const;

        bool
        GetValueFromLogicExpr(const std::string& lhs, const std::string& rhs, const std::string& op, const Tuple& row);

    public:

        friend std::ostream& operator<<(std::ostream& os, Table& table);

        Table() = default;

        void AddRow(const std::vector<std::string>& values);

        void AddRow(const std::vector<std::string>& values, const std::vector<std::string>& keys);

        void DeleteRows(const std::vector<std::string>& condition);

        void PrintWithCondition(const std::vector<std::string>& condition);

        void PrintWithCondition(const std::vector<std::string>& keys, const std::vector<std::string>& condition);

        void PrintWithoutCondition(const std::vector<std::string>& keys);

        void PrintWithoutCondition();

        bool IsColumn(const std::string& key);

        size_t ColumnNumber(const std::string& key);

        TYPE ColumnType(const std::string& key);

        Table(const std::unordered_map<std::string, ColumnInfo>& keys, const std::vector<std::string>& keys_order,
              const std::string& primary_key) : keys_(keys), keys_order_(keys_order), primary_key_(primary_key),
                                                rows_() {

        }

        friend Table InnerJoin(Table& lhs, Table& rhs, const std::string& left_column, const std::string& right_column);

        friend Table LeftJoin(Table& lhs, Table& rhs, const std::string& left_column, const std::string& right_column);

        void UpdateValues(const std::vector<std::pair<std::string, std::string>>& values,
                          const std::vector<std::string>& condition);

        void UpdateValues(const std::vector<std::pair<std::string, std::string>>& values);

        void WriteToFile(std::ofstream& file);
    };

    void DeleteSpaces(std::string& str);

    Table InnerJoin(Table& lhs, Table& rhs, const std::string& left_column, const std::string& right_column);

    Table LeftJoin(Table& lhs, Table& rhs, const std::string& left_column, const std::string& right_column);
}