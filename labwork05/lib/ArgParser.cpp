#include "ArgParser.h"

#include <utility>

using namespace ArgumentParser;

ArgParser::ArgumentValue::ArgumentValue() = default;

std::string ArgParser::ArgName(const std::string_view& arg) {
    size_t i = 0;
    for (i = 0; i < arg.size(); i++) {
        if (arg[i] == '=') {
            break;
        }
    }
    std::string arg_str{arg};
    arg_str.erase(i, arg.size() - i);
    return arg_str;
}

std::string ArgParser::ArgValue(const std::string_view& arg) {
    size_t i = 0;
    for (i = 0; i < arg.size(); i++) {
        if (arg[i] == '=') {
            break;
        }
    }
    if (i == arg.size()) {
        return "";
    }
    std::string arg_str{arg};
    arg_str.erase(0, i + 1);
    return arg_str;
}

ArgParser::ArgParser(std::string name) {
    this->name = std::move(name);
}

ArgParser& ArgParser::AddStringArgument(const std::string& long_arg) {
    ArgumentValue cur_arg;
    cur_arg.name = long_arg;
    cur_arg.isString = true;
    args.emplace_back(cur_arg);
    this->new_arg = &args.back();
    std::string current_line = new_arg->ConvertArgToHelpLine();
    help_arg.help_description.push_back(current_line);

    return *this;
}

ArgParser& ArgParser::AddStringArgument(const char short_arg, const std::string& long_arg) {
    ArgumentValue cur_arg;
    cur_arg.name = long_arg;
    cur_arg.short_name = short_arg;
    cur_arg.isString = true;
    args.emplace_back(cur_arg);
    this->new_arg = &args.back();
    std::string current_line = new_arg->ConvertArgToHelpLine();
    help_arg.help_description.push_back(current_line);

    return *this;
}

ArgParser& ArgParser::AddStringArgument(const std::string& long_arg, const std::string& description) {
    ArgumentValue cur_arg;
    cur_arg.name = long_arg;
    cur_arg.description = description;
    cur_arg.isString = true;
    args.emplace_back(cur_arg);
    this->new_arg = &args.back();
    std::string current_line = new_arg->ConvertArgToHelpLine();
    help_arg.help_description.push_back(current_line);

    return *this;
}


ArgParser& ArgParser::AddStringArgument(const char short_arg, const std::string& long_arg, const std::string& description) {
    ArgumentValue cur_arg;
    cur_arg.name = long_arg;
    cur_arg.short_name = short_arg;
    cur_arg.isDescribed = true;
    cur_arg.description = description;
    cur_arg.isString = true;
    args.emplace_back(cur_arg);
    this->new_arg = &args.back();
    std::string current_line = new_arg->ConvertArgToHelpLine();
    help_arg.help_description.push_back(current_line);

    return *this;
}

std::string ArgParser::GetStringValue(const std::string& arg) {
    for (size_t i = 0; i < args.size(); i++) {
        if (args[i].name == arg) {
            if (args[i].isStored) {
                return *args[i].string_arg.value_pointer;
            } else {
                return args[i].string_arg.value;
            }
        }
    }
}

std::string ArgParser::GetStringValue(const std::string& arg, uint64_t pos) {
    for (size_t i = 0; i < args.size(); i++) {
        if (args[i].name == arg) {
            if (args[i].isStored) {
                return *(args[i].string_arg.values_pointer->begin() + pos);
            } else {
                return args[i].string_arg.values[pos];
            }
        }
    }
}


void ArgParser::Default(const std::string& value) {
    new_arg->isValued = true;
    new_arg->string_arg.value = value;
    new_arg->isDefault = true;
}

void ArgParser::StoreValue(std::string& value) {
    new_arg->isStored = true;
    new_arg->string_arg.value_pointer = &value;
}

void ArgParser::StoreValue(std::vector<std::string>& values) {
    new_arg->isStored = true;
    new_arg->string_arg.values_pointer = &values;
}


ArgParser& ArgParser::MultiValue() {
    new_arg->isMulti = true;
    new_arg->isValued = true;

    return *this;
}

ArgParser& ArgParser::MultiValue(uint64_t min_args_count) {
    new_arg->isMulti = true;
    new_arg->min_args_count = min_args_count;

    return *this;
}

ArgParser& ArgParser::Positional() {
    positional_arg = new_arg;
    new_arg->isPositional = true;
    new_arg->isValued = true;

    return *this;
}

bool ArgParser::Parse(const std::vector<std::string>& console_args) {
    if (console_args.size() < 1) {
        return false;
    }

    for (size_t i = 1; i < console_args.size(); i++) {
        if (console_args[i][0] == '-') {
            if (console_args[i].size() > 1 && console_args[i][1] == '-') {

                std::string_view arg_view{console_args[i]};
                arg_view.remove_prefix(2);

                std::string arg_name = ArgName(arg_view);
                std::string arg_value = ArgValue(arg_view);

                if (arg_name == help_arg.name) {
                    help_arg.isHelp = true;
                    return true;
                }

                bool isExist = false;
                size_t j = 0;
                for (j = 0; j < args.size(); j++) {
                    if (args[j].name == arg_name) {
                        isExist = true;
                        break;
                    }
                }

                if (isExist) {
                    args[j].ValueArgument(arg_value);
                }
            } else {
                std::string_view arg_view{console_args[i]};
                arg_view.remove_prefix(1);

                std::string arg_names = ArgName(arg_view);
                std::string arg_value = ArgValue(arg_view);

                size_t j = 0;

                for (int k = 0; k < arg_names.size(); k++) {
                    bool isExist = false;

                    if (arg_names[k] == help_arg.short_name) {
                        help_arg.isHelp = true;
                        return true;
                    }

                    for (j = 0; j < args.size(); j++) {
                        if (args[j].short_name == arg_names[k]) {
                            isExist = true;
                            break;
                        }
                    }
                    if (isExist) {
                        args[j].ValueArgument(arg_value);
                    }
                }

            }
        } else {
            if (positional_arg != nullptr) {
                positional_arg->ValuePositional(console_args[i]);
            }
        }
    }

    return CheckParse();
}


ArgParser& ArgParser::AddIntArgument(const std::string& long_arg) {
    ArgumentValue cur_arg;
    cur_arg.name = long_arg;
    cur_arg.isInt = true;
    args.emplace_back(cur_arg);
    this->new_arg = &args.back();
    std::string current_line = new_arg->ConvertArgToHelpLine();
    help_arg.help_description.push_back(current_line);

    return *this;
}

ArgParser& ArgParser::AddIntArgument(const char short_arg, const std::string& long_arg) {
    ArgumentValue cur_arg;
    cur_arg.name = long_arg;
    cur_arg.short_name = short_arg;
    cur_arg.isInt = true;
    args.emplace_back(cur_arg);
    this->new_arg = &args.back();
    std::string current_line = new_arg->ConvertArgToHelpLine();
    help_arg.help_description.push_back(current_line);

    return *this;
}

ArgParser& ArgParser::AddIntArgument(const std::string& long_arg, const std::string& description) {
    ArgumentValue cur_arg;
    cur_arg.name = long_arg;
    cur_arg.description = description;
    cur_arg.isInt = true;
    args.emplace_back(cur_arg);
    this->new_arg = &args.back();
    std::string current_line = new_arg->ConvertArgToHelpLine();
    help_arg.help_description.push_back(current_line);

    return *this;
}

ArgParser& ArgParser::AddIntArgument(const char short_arg, const std::string& long_arg, const std::string& description) {
    ArgumentValue cur_arg;
    cur_arg.name = long_arg;
    cur_arg.short_name = short_arg;
    cur_arg.isDescribed = true;
    cur_arg.description = description;
    cur_arg.isInt = true;
    args.emplace_back(cur_arg);
    this->new_arg = &args.back();
    std::string current_line = new_arg->ConvertArgToHelpLine();
    help_arg.help_description.push_back(current_line);

    return *this;
}

int ArgParser::GetIntValue(const std::string& arg) {
    for (size_t i = 0; i < args.size(); i++) {
        if (args[i].name == arg) {
            if (args[i].isStored) {
                return *args[i].int_arg.value_pointer;
            } else {
                return args[i].int_arg.value;
            }
        }
    }
}

int ArgParser::GetIntValue(const std::string& arg, uint64_t pos) {
    for (size_t i = 0; i < args.size(); i++) {
        if (args[i].name == arg) {
            if (args[i].isStored) {
                return *(args[i].int_arg.values_pointer->begin() + pos);
            } else {
                return args[i].int_arg.values[i];
            }
        }
    }
}

void ArgParser::Default(int value) {
    new_arg->isValued = true;
    new_arg->int_arg.value = value;
    new_arg->isDefault = true;
}

void ArgParser::StoreValue(int& value) {
    new_arg->isStored = true;
    new_arg->int_arg.value_pointer = &value;
}

void ArgParser::StoreValues(std::vector<int>& values) {
    new_arg->isStored = true;
    new_arg->int_arg.values_pointer = &values;
}

ArgParser& ArgParser::AddFlag(const std::string& long_arg) {
    ArgumentValue cur_arg;
    cur_arg.name = long_arg;
    cur_arg.isBool = true;
    cur_arg.isValued = true;
    args.emplace_back(cur_arg);
    this->new_arg = &args.back();
    std::string current_line = new_arg->ConvertArgToHelpLine();
    help_arg.help_description.push_back(current_line);

    return *this;
}

ArgParser& ArgParser::AddFlag(const char short_arg, const std::string& long_arg) {
    ArgumentValue cur_arg;
    cur_arg.name = long_arg;
    cur_arg.short_name = short_arg;
    cur_arg.isBool = true;
    cur_arg.isValued = true;
    args.emplace_back(cur_arg);
    this->new_arg = &args.back();
    std::string current_line = new_arg->ConvertArgToHelpLine();
    help_arg.help_description.push_back(current_line);

    return *this;
}

ArgParser& ArgParser::AddFlag(const std::string& long_arg, const std::string& description) {
    ArgumentValue cur_arg;
    cur_arg.name = long_arg;
    cur_arg.description = description;
    cur_arg.isBool = true;
    cur_arg.isValued = true;
    args.emplace_back(cur_arg);
    this->new_arg = &args.back();
    std::string current_line = new_arg->ConvertArgToHelpLine();
    help_arg.help_description.push_back(current_line);

    return *this;
}

ArgParser& ArgParser::AddFlag(const char short_arg, const std::string& long_arg, const std::string& description) {
    ArgumentValue cur_arg;
    cur_arg.name = long_arg;
    cur_arg.short_name = short_arg;
    cur_arg.isDescribed = true;
    cur_arg.description = description;
    cur_arg.isBool = true;
    cur_arg.isValued = true;
    args.emplace_back(cur_arg);
    this->new_arg = &args.back();
    std::string current_line = new_arg->ConvertArgToHelpLine();
    help_arg.help_description.push_back(current_line);

    return *this;
}

bool ArgParser::GetFlag(const std::string& arg) {
    for (size_t i = 0; i < args.size(); i++) {
        if (args[i].name == arg) {
            if (args[i].isStored) {
                return *args[i].bool_arg.value_pointer;
            } else {
                return args[i].bool_arg.value;
            }
        }
    }
}

void ArgParser::Default(bool value) {
    new_arg->bool_arg.value = value;
    new_arg->isDefault = true;
}

void ArgParser::StoreValue(bool& value) {
    new_arg->isStored = true;
    new_arg->bool_arg.value_pointer = &value;
    *new_arg->bool_arg.value_pointer = false;
}

void ArgParser::Default(const char* value) {
    new_arg->isValued = true;
    new_arg->string_arg.value = value;
    new_arg->isDefault = true;
}

void ArgParser::AddHelp(char short_arg, const std::string& long_arg, const std::string& description) {
    help_arg.name = long_arg;
    help_arg.short_name = short_arg;
    help_arg.description = description;
    help_arg.help_description.push_back(name + '\n');
    help_arg.help_description.push_back(description + '\n');
    help_arg.help_description.emplace_back("\n");
    std::string current_line;
    for (size_t i = 0; i < args.size(); i++) {
        current_line = args[i].ConvertArgToHelpLine();
        help_arg.help_description.push_back(current_line);
    }
}

bool ArgParser::Help() {
    return help_arg.isHelp;
}

bool ArgParser::CheckParse() {
    for (size_t i = 0; i < args.size(); i++) {
        if (args[i].isMulti) {
            if (args[i].isStored) {
                if ((args[i].isString && args[i].string_arg.values_pointer->size() < args[i].min_args_count) ||
                    (args[i].isInt && args[i].int_arg.values_pointer->size() < args[i].min_args_count)) {
                    return false;
                }
            } else {
                if ((args[i].isString && args[i].string_arg.values.size() < args[i].min_args_count) ||
                    (args[i].isInt && args[i].int_arg.values.size() < args[i].min_args_count)) {
                    return false;
                }
            }
        } else if (!args[i].isValued) {
            return false;
        }
    }
    return true;
}

bool ArgParser::Parse(int argc, char** argv) {
    std::vector<std::string> args(argc);
    for (size_t i = 0; i < argc; i++) {
        args[i] = argv[i];
    }

    return Parse(args);
}

std::string ArgParser::HelpDescription() {
    std::string result;
    for (size_t i = 0; i < help_arg.help_description.size(); i++) {
        result += help_arg.help_description[i];
    }
    result.push_back('\n');
    result += help_arg.ConvertHelpToHelpLine();
    return result;
}

std::string ArgParser::ArgumentValue::ConvertArgToHelpLine() {
    std::string current_line;
    if (short_name == '\0') {
        current_line += "     ";
    } else {
        current_line.push_back('-');
        current_line.push_back(short_name);
        current_line += ",  ";
    }

    current_line += "--" + name;

    if (isString) {
        current_line.push_back('=');
        current_line += "<string>";
    } else if (isInt) {
        current_line.push_back('=');
        current_line += "<int>";
    }

    if (isDescribed) {
        current_line += ", " + description;
    }

    current_line.push_back(' ');

    if (isMulti) {
        current_line.push_back('[');
        current_line += "repeated, min args = ";
        current_line += std::to_string(min_args_count);
        current_line.push_back(']');
    }

    if (isDefault) {
        current_line.push_back('[');
        current_line += "default = ";
        if (isString) {
            current_line += string_arg.value;
        } else if (isInt) {
            current_line += std::to_string(int_arg.value);
        } else {
            if (bool_arg.value) {
                current_line += "true";
            } else {
                current_line += "false";
            }
        }
        current_line.push_back(']');
    }

    current_line.push_back('\n');

    return current_line;
}

ArgParser::Help::Help() = default;

void ArgParser::ArgumentValue::ValueArgument(const std::string& value) {
    if (isString) { ///STRING

        if (!isValued) {
            isValued = true;
        }

        if (isMulti) {
            if (isStored) {
                string_arg.values_pointer->push_back(value);
            } else {
                string_arg.values.push_back(value);
            }
        } else {
            if (isStored) {
                *string_arg.value_pointer = value;
            } else {
                string_arg.value = value;
            }
        }
    } else if (isInt) { ///INT

        if (!isValued) {
            isValued = true;
        }

        if (isMulti) {
            if (isStored) {
                int_arg.values_pointer->push_back(std::stoi(value));
            } else {
                int_arg.values.push_back(std::stoi(value));
            }
        } else {
            if (isStored) {
                *int_arg.value_pointer = std::stoi(value);
            } else {
                int_arg.value = std::stoi(value);
            }
        }
    } else if (isBool) { ///BOOL

        if (isStored) {
            *bool_arg.value_pointer = true;
        } else {
            bool_arg.value = true;
        }
    }
}

void ArgParser::ArgumentValue::ValuePositional(const std::string& value) {
    if (isString) { ///POS STRING
        if (!isValued) {
            isValued = true;
        }

        if (isMulti) {
            if (isStored) {
                string_arg.values_pointer->push_back(value);
            } else {
                string_arg.values.push_back(value);
            }
        } else {
            if (isStored) {
                *string_arg.value_pointer = value;
            } else {
                string_arg.value = value;
            }
        }
    } else if (isInt) { ///POS INT
        if (!isValued) {
            isValued = true;
        }

        if (isMulti) {
            if (isStored) {
                int_arg.values_pointer->push_back(std::stoi(value));
            } else {
                int_arg.values.push_back(std::stoi(value));
            }
        } else {
            if (isStored) {
                *int_arg.value_pointer = std::stoi(value);
            } else {
                int_arg.value = std::stoi(value);
            }
        }
    }
}

std::string ArgParser::Help::ConvertHelpToHelpLine() {
    std::string current_line;
    current_line.push_back('-');
    current_line.push_back(short_name);
    current_line += ", ";
    current_line += "--";
    current_line += name;
    current_line += ", ";
    current_line += "Display this help and exit\n";
    return current_line;
}
