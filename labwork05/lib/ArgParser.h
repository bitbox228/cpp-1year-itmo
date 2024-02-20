#pragma once

#include <map>
#include <string>
#include <string_view>
#include <vector>
#include <iostream>

namespace ArgumentParser {

    class ArgParser {

    private:
        std::string name;

        std::string ArgName(const std::string_view& arg);

        std::string ArgValue(const std::string_view& arg);

        struct ArgumentValue {
            std::string name;
            char short_name = '\0';
            std::string description;
            bool isDescribed = false;

            bool isDefault = false;
            bool isValued = false;
            bool isStored = false;
            bool isPositional = false;
            bool isMulti = false;

            bool isInt = false;
            bool isString = false;
            bool isBool = false;

            uint64_t min_args_count = 0;

            struct StringArgument {
                std::string value;
                std::string* value_pointer = nullptr;
                std::vector<std::string> values = {};
                std::vector<std::string>* values_pointer = nullptr;
            };

            struct IntArgument {
                int value = 0;
                int* value_pointer = nullptr;
                std::vector<int> values = {};
                std::vector<int>* values_pointer = nullptr;
            };

            struct BoolArgument {
                bool value = false;
                bool* value_pointer = nullptr;
            };

            void ValueArgument(const std::string& value);

            void ValuePositional(const std::string& value);

            StringArgument string_arg;
            IntArgument int_arg;
            BoolArgument bool_arg;

            ArgumentValue();

            std::string ConvertArgToHelpLine();
        };

        struct Help {
            bool isHelp = false;

            std::string name;
            char short_name = '\0';
            std::string description;

            std::vector<std::string> help_description;

            std::string ConvertHelpToHelpLine();

            Help();
        };

        std::vector <ArgumentValue> args;

        Help help_arg;

        ArgumentValue* new_arg = nullptr;

        ArgumentValue* positional_arg = nullptr;

        bool CheckParse();

    public:
        explicit ArgParser(std::string name);

        ArgParser& MultiValue();

        ArgParser& MultiValue(uint64_t min_args_count);

        ArgParser& Positional();

        ///StringArgs

        ArgParser& AddStringArgument(const std::string& long_arg);

        ArgParser& AddStringArgument(const char short_arg, const std::string& long_arg);

        ArgParser& AddStringArgument(const std::string& long_arg, const std::string& description);

        ArgParser& AddStringArgument(const char short_arg, const std::string& long_arg, const std::string& description);

        void Default(const std::string& value);

        void Default(const char* value);

        void StoreValue(std::string& value);

        void StoreValue(std::vector<std::string>& values);

        std::string GetStringValue(const std::string& arg);

        std::string GetStringValue(const std::string& arg, uint64_t pos);

        ///IntArgs

        ArgParser& AddIntArgument(const std::string& long_arg);

        ArgParser& AddIntArgument(const char short_arg, const std::string& long_arg);

        ArgParser& AddIntArgument(const std::string& long_arg, const std::string& description);

        ArgParser& AddIntArgument(const char short_arg, const std::string& long_arg, const std::string& description);

        void Default(int value);

        void StoreValue(int& value);

        void StoreValues(std::vector<int>& values);

        int GetIntValue(const std::string& arg);

        int GetIntValue(const std::string& arg, uint64_t pos);

        ///BoolArgs

        ArgParser& AddFlag(const std::string& long_arg);

        ArgParser& AddFlag(const char short_arg, const std::string& long_arg);

        ArgParser& AddFlag(const std::string& long_arg, const std::string& description);

        ArgParser& AddFlag(const char short_arg, const std::string& long_arg, const std::string& description);

        void Default(bool value);

        void StoreValue(bool& value);

        bool GetFlag(const std::string& arg);

        //Help

        void AddHelp(char short_arg, const std::string& long_arg, const std::string& description);

        bool Help();

        std::string HelpDescription();

        ///Parse

        bool Parse(const std::vector<std::string>& args);

        bool Parse(int argc, char** argv);
    };

}