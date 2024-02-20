#include "parser.h"

using namespace omfl;

Variable empty_var;
Section empty_section;

Array* current_array = nullptr;
Section* current_section = nullptr;

Variable& Variable::operator[](size_t index) const {
    return empty_var;
}

Variable& Array::operator[](size_t index) const {
    if (index < values_.size()) {
        return *values_[index];
    }
    return empty_var;
}

Variable& Variable::Get(std::string_view path) const {
    return empty_var;
}

Variable& Section::Get(std::string_view path) const {
    size_t dot = path.find('.');

    if (dot == std::string_view::npos) {
        return Find(path);
    } else {
        std::string_view current = path.substr(0, dot);
        return Find(current).Get(path.substr(current.size() + 1, path.size()));
    }
}

void DeleteSpaces(std::string_view& line) {
    int i;
    for (i = 0; i < line.size(); i++) {
        if (line[i] != ' ') {
            break;
        }
    }
    line.remove_prefix(i);

    for (i = line.size() - 1; i >= 0; i--) {
        if (line[i] != ' ') {
            break;
        }
    }
    line.remove_suffix(line.size() - i - 1);
}

void DeleteComment(std::string_view& line) {
    bool in_string = false;
    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == '#' && !in_string) {
            line.remove_suffix(line.size() - i);
            return;
        } else if (line[i] == '\"') {
            in_string = !in_string;
        }
    }
}

std::string_view DeleteNeedless(std::string_view line) {
    if (line.empty()) {
        return line;
    }
    std::string_view new_line = line;
    DeleteComment(new_line);
    DeleteSpaces(new_line);
    return new_line;
}

void Section::ParseLine(std::string_view line) {
    std::string_view new_line = DeleteNeedless(line);
    if (new_line.empty()) {
        return;
    }
    if (new_line.front() == '[' && new_line.back() == ']') {
        AddSection(new_line);
    } else {
        AddVariable(new_line);
    }
}

void Section::AddSection(std::string_view line) {
    if (line.size() == 2 || line[line.size() - 2] == '.' || line[1] == '.') {
        valid_ = false;
        return;
    }

    size_t start = 1;
    Section* section = this;
    for (size_t i = 1; i < line.size() - 1; i++) {
        if (line[i] == '.') {
            std::string_view section_name = line.substr(start, i - start);
            if (section->Find(section_name).IsSection()) {
                section = dynamic_cast<Section*>(&section->Get(section_name));
            } else {
                auto* new_section = new Section(section_name);
                section->values_.push_back(new_section);
                section = new_section;
            }
            start = i + 1;
        } else if (!(isdigit(line[i]) || isalpha(line[i]) || line[i] == '-' || line[i] == '_')) {
            valid_ = false;
            return;
        } else if (i == line.size() - 2) {
            std::string_view section_name = line.substr(start, i - start + 1);
            if (section->Find(section_name).IsSection()) {
                section = dynamic_cast<Section*>(&section->Get(section_name));
            } else {
                auto* new_section = new Section(section_name);
                section->values_.push_back(new_section);
                section = new_section;
            }
            start = i + 1;
        }
    }

    current_section = section;
}

void Section::AddVariable(std::string_view line) {
    std::string_view key;
    std::string_view value;
    bool key_end = false;
    bool equal = false;
    for (size_t i = 0; i < line.size(); i++) {
        if ((equal) && (line[i] != ' ')) {
            value = line.substr(i, line.size() - i);
            break;
        } else if (key_end) {
            if (line[i] == '=') {
                equal = true;
            } else if (line[i] != ' ') {
                valid_ = false;
                return;
            }
        } else {
            if (line[i] == ' ') {
                key_end = true;
                key = line.substr(0, i);
            } else if (line[i] == '=') {
                key_end = true;
                equal = true;
                key = line.substr(0, i);
            } else if (!(isdigit(line[i]) || isalpha(line[i]) || line[i] == '-' || line[i] == '_')) {
                valid_ = false;
                return;
            }
        }
    }

    if (current_section->Exists(key) || value.empty() || key.empty()) {
        valid_ = false;
    } else {
        current_section->ParseValue(key, value);
        if (!current_section->valid_) {
            valid_ = false;
        }
    }
}


bool IsValueFloat(const std::string_view& value) {
    size_t dots_count = 0;
    bool digits = false;
    for (size_t i = 0; i < value.size(); i++) {
        if (value[i] == '.') {
            dots_count++;
            if (dots_count > 1 || i == 0 || i == value.size() - 1 || (i == 1 && (value[0] == '-' || value[0] == '+'))) {
                return false;
            }
        } else if ((value[i] == '-' || value[i] == '+') && i == 0) {
            continue;
        } else if (!isdigit(value[i])) {
            return false;
        } else if (!digits && isdigit(value[i])) {
            digits = true;
        }
    }
    if (dots_count == 0 || !digits) {
        return false;
    }
    return true;
}

bool IsValueInt(const std::string_view& value) {
    bool digits = false;
    for (size_t i = 0; i < value.size(); i++) {
        if ((value[i] == '-' || value[i] == '+') && i == 0) {
            continue;
        } else if (!isdigit(value[i])) {
            return false;
        } else if (!digits && isdigit(value[i])) {
            digits = true;
        }
    }
    if (!digits) {
        return false;
    }
    return true;
}

bool IsValueString(const std::string_view& value) {
    for (size_t i = 1; i < value.size() - 1; i++) {
        if (value[i] == '\"') {
            return false;
        }
    }
    return true;
}

void Section::ParseValue(std::string_view key, std::string_view value) {
    if (value.empty()) {
        return;
    }
    if (value == "true") {
        auto* var = new BoolVar(key, true);
        values_.push_back(var);
    } else if (value == "false") {
        auto* var = new BoolVar(key, false);
        values_.push_back(var);
    } else if (value.front() == '[' && value.back() == ']') {
        auto* var = new Array(key);
        values_.push_back(var);

        int32_t qoute = 0;
        int32_t bracket = 0;
        int32_t var_start = 1;

        for (size_t i = 1; i < value.size() - 1; i++) {
            if (value[i] == '[' && qoute % 2 == 0) {
                bracket++;
            } else if (value[i] == '\"' && bracket == 0) {
                qoute++;
            } else if (value[i] == ']' && qoute % 2 == 0) {
                bracket--;
            } else if (value[i] == ',' && qoute % 2 == 0) {
                if (bracket == 0) {
                    var->ParseValue("", DeleteNeedless(value.substr(var_start, i - var_start)));
                    var_start = i + 1;
                }
            }
        }
        var->ParseValue("", DeleteNeedless(value.substr(var_start, value.size() - 1 - var_start)));
        if (!var->valid_) {
            valid_ = false;
            return;
        }

    } else if (value.front() == '\"' && value.back() == '\"' && IsValueString(value)) {
        auto* var = new StringVar(key, value.substr(1, value.size() - 2));
        values_.push_back(var);
    } else if (IsValueInt(value)) {
        std::string str_value{value};
        int32_t casted = std::stoi(str_value);
        auto* var = new IntVar(key, casted);
        values_.push_back(var);
    } else if (IsValueFloat(value)) {
        std::string str_value{value};
        float casted = std::atof(str_value.c_str());
        auto* var = new FloatVar(key, casted);
        values_.push_back(var);
    } else {
        valid_ = false;
        return;
    }
}

void Array::ParseValue(std::string_view key, std::string_view value) {
    if (value.empty()) {
        return;
    }
    if (value == "true") {
        auto* var = new BoolVar(key, true);
        values_.push_back(var);
    } else if (value == "false") {
        auto* var = new BoolVar(key, false);
        values_.push_back(var);
    } else if (value.front() == '[' && value.back() == ']') {
        auto* var = new Array(key);
        values_.push_back(var);

        int32_t qoute = 0;
        int32_t bracket = 0;
        int32_t var_start = 1;

        for (size_t i = 1; i < value.size() - 1; i++) {
            if (value[i] == '[' && qoute % 2 == 0) {
                bracket++;
            } else if (value[i] == '\"' && bracket == 0) {
                qoute++;
            } else if (value[i] == ']' && qoute % 2 == 0) {
                bracket--;
            } else if (value[i] == ',' && qoute % 2 == 0) {
                if (bracket == 0) {
                    var->ParseValue("", DeleteNeedless(value.substr(var_start, i - var_start)));
                    var_start = i + 1;
                }
            }
        }
        var->ParseValue("", DeleteNeedless(value.substr(var_start, value.size() - 1 - var_start)));
        if (!var->valid_) {
            valid_ = false;
            return;
        }

    } else if (value.front() == '\"' && value.back() == '\"' && IsValueString(value)) {
        auto* var = new StringVar(key, value.substr(1, value.size() - 2));
        values_.push_back(var);
    } else if (IsValueInt(value)) {
        std::string str_value{value};
        int32_t casted = std::stoi(str_value);
        auto* var = new IntVar(key, casted);
        values_.push_back(var);
    } else if (IsValueFloat(value)) {
        std::string str_value{value};
        float casted = std::atof(str_value.c_str());
        auto* var = new FloatVar(key, casted);
        values_.push_back(var);
    } else {
        valid_ = false;
        return;
    }
}

bool Section::Exists(std::string_view key) const {
    for (size_t i = 0; i < values_.size(); i++) {
        if (values_[i]->key_ == key) {
            return true;
        }
    }
    return false;
}

Variable& Section::Find(std::string_view key) const {
    for (size_t i = 0; i < values_.size(); i++) {
        if (values_[i]->key_ == key) {
            return *values_[i];
        }
    }
    return empty_var;
}

Section& omfl::parse(const std::string& code) {
    auto* root = new Section;
    current_section = root;
    std::stringstream buffer(code);
    std::string line;
    while (!buffer.eof()) {
        getline(buffer, line);
        root->ParseLine(line);
    }
    return *root;
}

Section& omfl::parse(const std::filesystem::path& path) {
    auto* root = new Section;
    if (!std::filesystem::exists(path)) {
        return *root;
    }
    current_section = root;
    std::ifstream file(path.c_str());
    std::string line;
    while (!file.eof()) {
        getline(file, line);
        root->ParseLine(line);
    }
    return *root;
}

void Section::CreateXML(const std::filesystem::path& path) const {
    std::ofstream file(path.c_str());
    file << '<' << "root" << '>' << '\n';
    for (size_t i = 0; i < values_.size(); i++) {
        values_[i]->WriteXML(file);
    }
    file << "</" << "root" << '>' << '\n';
}

void Section::CreateYAML(const std::filesystem::path& path) const {
    std::ofstream file(path.c_str());
    file << "---" << '\n';
    for (size_t i = 0; i < values_.size(); i++) {
        values_[i]->WriteYAML(file, 0);
    }
    file << "...";
}

void Section::CreateJSON(const std::filesystem::path& path) const {
    std::ofstream file(path.c_str());
    file << '{' << '\n';
    for (size_t i = 0; i < values_.size(); i++) {
        values_[i]->WriteJSON(file, 1);
        if (i != values_.size() - 1) {
            file << ',';
        }
        file << '\n';
    }
    file << '}';
}
