#include "table.h"

#include <unordered_map>
#include <string>

namespace MyCoolNamespace {

    class MyCoolDataBase {
        std::string name_;
        std::unordered_map<std::string, Table> tables_;
    private:
        static std::vector<std::string> SplitString(const std::string& args, char sep);

        static void ToUpper(std::string& word);

        void CreateTable(const std::string& query);

        void DropTable(const std::string& query);

        void InsertInto(const std::string& query);

        void DeleteFrom(const std::string& query);

        void SelectFrom(const std::string& query);

        void PrintTablesList();

        void Update(const std::string& query);

    public:

        MyCoolDataBase(const std::string& name = "DataBase") : name_(name) {
        }

        void Parse(const std::string& query);

        void SaveDB(const std::string& path);

        void LoadDB(const std::string& path);
    };
}