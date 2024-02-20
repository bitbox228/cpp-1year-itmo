#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <sstream>
#include <vector>
#include <exception>


namespace omfl {

    class Variable {

    public:

        bool valid_ = true;
        std::string key_;

        Variable() = default;

        Variable(std::string_view key) {
            key_ = key;
        }

        virtual bool IsInt() const {
            return false;
        }

        virtual int32_t AsInt() const {
            throw std::invalid_argument("Invalid type of Variable");
        }

        virtual int32_t AsIntOrDefault(int32_t value) const {
            return value;
        }

        virtual bool IsFloat() const {
            return false;
        }

        virtual float AsFloat() const {
            throw std::invalid_argument("Invalid type of Variable");
        }

        virtual float AsFloatOrDefault(float value) const {
            return value;
        }

        virtual bool IsString() const {
            return false;
        }

        virtual std::string AsString() const {
            throw std::invalid_argument("Invalid type of Variable");
        }

        virtual std::string AsStringOrDefault(const std::string& value) const {
            return value;
        }

        virtual bool IsBool() const {
            return false;
        }

        virtual bool AsBool() const {
            throw std::invalid_argument("Invalid type of Variable");
        }

        virtual bool AsBoolOrDefault(bool value) const {
            return value;
        }

        virtual bool IsArray() const {
            return false;
        }

        virtual bool IsSection() const {
            return false;
        }

        virtual Variable& operator[](size_t index) const;

        virtual Variable& Get(std::string_view path) const;

        virtual void WriteXML(std::ofstream& file) const {
            return;
        }

        virtual void WriteYAML(std::ofstream& file, size_t margins) const {
            return;
        }

        virtual void WriteJSON(std::ofstream& file, size_t tabs) const {
            return;
        }

    };

    class IntVar : public Variable {
    private:

        int32_t value_;

    public:

        IntVar(std::string_view key, int32_t value) : Variable(key), value_(value) {
        }

        bool IsInt() const override {
            return true;
        }

        int32_t AsInt() const override {
            return value_;
        }

        int32_t AsIntOrDefault(int32_t value) const override {
            return value_;
        }

    private:

        void WriteXML(std::ofstream& file) const override {
            file << '<' << key_ << '>' << value_ << "</" << key_ << '>' << '\n';
        }

        void WriteYAML(std::ofstream& file, size_t margins) const override {
            for (size_t j = 0; j < margins; j++) {
                file << ' ';
            }
            if (key_.empty()) {
                file << "- " << value_ << '\n';
            } else {
                file << key_ << ": " << value_ << '\n';
            }
        }

        void WriteJSON(std::ofstream& file, size_t tabs) const override {
            for (size_t j = 0; j < tabs; j++) {
                file << "  ";
            }
            if (key_.empty()) {
                file << value_;
            } else {
                file << '\"' << key_ << "\": " << value_;
            }
        }

    };

    class FloatVar : public Variable {
    private:

        float value_;

    public:

        FloatVar(std::string_view key, float value) : Variable(key), value_(value) {
        }

        bool IsFloat() const override {
            return true;
        }

        float AsFloat() const override {
            return value_;
        }

        float AsFloatOrDefault(float value) const override {
            return value_;
        }

    private:

        void WriteXML(std::ofstream& file) const override {
            file << '<' << key_ << '>' << value_ << "</" << key_ << '>' << '\n';
        }

        void WriteYAML(std::ofstream& file, size_t margins) const override {
            for (size_t j = 0; j < margins; j++) {
                file << ' ';
            }
            if (key_.empty()) {
                file << "- " << value_ << '\n';
            } else {
                file << key_ << ": " << value_ << '\n';
            }
        }

        void WriteJSON(std::ofstream& file, size_t tabs) const override {
            for (size_t j = 0; j < tabs; j++) {
                file << "  ";
            }
            if (key_.empty()) {
                file << value_;
            } else {
                file << '\"' << key_ << "\": " << value_;
            }
        }

    };

    class StringVar : public Variable {
    private:

        std::string value_;

    public:

        StringVar(std::string_view key, std::string_view value) : Variable(key), value_(value) {
        }

        bool IsString() const override {
            return true;
        }

        std::string AsString() const override {
            return value_;
        }

        std::string AsStringOrDefault(const std::string& value) const override {
            return value_;
        }

    private:

        void WriteXML(std::ofstream& file) const override {
            file << '<' << key_ << '>' << value_ << "</" << key_ << '>' << '\n';
        }

        void WriteYAML(std::ofstream& file, size_t margins) const override {
            for (size_t j = 0; j < margins; j++) {
                file << ' ';
            }
            if (key_.empty()) {
                file << "- " << value_ << '\n';
            } else {
                file << key_ << ": " << value_ << '\n';
            }
        }

        void WriteJSON(std::ofstream& file, size_t tabs) const override {
            for (size_t j = 0; j < tabs; j++) {
                file << "  ";
            }
            if (key_.empty()) {
                file << value_;
            } else {
                file << '\"' << key_ << "\": \"" << value_ << '\"';
            }

        }

    };


    class BoolVar : public Variable {
    private:

        bool value_;

    public:

        BoolVar(std::string_view key, bool value) : Variable(key), value_(value) {
        }

        bool IsBool() const override {
            return true;
        }

        bool AsBool() const override {
            return value_;
        }

        bool AsBoolOrDefault(bool value) const override {
            return value_;
        }

    private:

        void WriteXML(std::ofstream& file) const override {
            file << '<' << key_ << '>' << (value_ ? "true" : "false") << "</" << key_ << '>' << '\n';
        }

        void WriteYAML(std::ofstream& file, size_t margins) const override {
            for (size_t j = 0; j < margins; j++) {
                file << ' ';
            }
            if (key_.empty()) {
                file << "- " << (value_ ? "true" : "false") << '\n';
            } else {
                file << key_ << ": " << (value_ ? "true" : "false") << '\n';
            }
        }

        void WriteJSON(std::ofstream& file, size_t tabs) const override {
            for (size_t j = 0; j < tabs; j++) {
                file << "  ";
            }
            if (key_.empty()) {
                file << value_;
            } else {
                file << '\"' << key_ << "\": " << (value_ ? "true" : "false");
            }
        }

    };

    class Array : public Variable {
    private:

        std::vector<Variable*> values_;

        void PushVar(Variable& var) {
            values_.push_back(&var);
        }

    public:

        Array(std::string_view key) : Variable(key) {
        }

        ~Array() {
            for (size_t i = 0; i < values_.size(); i++) {
                delete values_[i];
            }
        }

        bool IsArray() const override {
            return true;
        }

        Variable& operator[](size_t index) const override;

        void ParseValue(std::string_view key, std::string_view value);

    private:

        void WriteYAML(std::ofstream& file, size_t margins) const override {
            for (size_t j = 0; j < margins; j++) {
                file << ' ';
            }
            if (key_.empty()) {
                file << "- " << '\n';
            } else {
                file << key_ << ": " << '\n';
            }
            for (size_t i = 0; i < values_.size(); i++) {
                values_[i]->WriteYAML(file, margins + 1);
            }
        }

        void WriteJSON(std::ofstream& file, size_t tabs) const override {
            for (size_t j = 0; j < tabs; j++) {
                file << "  ";
            }
            if (!key_.empty()) {
                file << '\"' << key_ << "\": ";
            }
            file << '[' << '\n';
            for (size_t i = 0; i < values_.size(); i++) {
                values_[i]->WriteJSON(file, tabs + 1);
                if (i != values_.size() - 1) {
                    file << ',';
                }
                file << '\n';
            }
            for (size_t j = 0; j < tabs; j++) {
                file << "  ";
            }
            file << ']';
        }
    };

    class Section : public Variable {
    private:

        std::vector<Variable*> values_;

        void AddSection(std::string_view line);

        void AddVariable(std::string_view line);

        Variable& Find(std::string_view key) const;

        bool Exists(std::string_view key) const;

        void ParseValue(std::string_view key, std::string_view value);

    public:

        Section() = default;

        Section(std::string_view key) : Variable(key) {
        }

        ~Section() {
            for (size_t i = 0; i < values_.size(); i++) {
                delete values_[i];
            }
        }

        bool IsSection() const override {
            return true;
        }

        Variable& Get(std::string_view path) const override;

        void ParseLine(std::string_view line);

        bool valid() const {
            return valid_;
        }

        void CreateXML(const std::filesystem::path& path) const;

        void CreateYAML(const std::filesystem::path& path) const;

        void CreateJSON(const std::filesystem::path& path) const;

    private:

        void WriteXML(std::ofstream& file) const override {
            file << '<' << key_ << '>' << '\n';
            for (size_t i = 0; i < values_.size(); i++) {
                values_[i]->WriteXML(file);
            }
            file << "</" << key_ << '>' << '\n';
        }

        void WriteYAML(std::ofstream& file, size_t margins) const override {
            for (size_t j = 0; j < margins; j++) {
                file << ' ';
            }
            file << key_ << ": " << '\n';
            for (size_t i = 0; i < values_.size(); i++) {
                values_[i]->WriteYAML(file, margins + 1);
            }
        }

        void WriteJSON(std::ofstream& file, size_t tabs) const override {
            for (size_t j = 0; j < tabs; j++) {
                file << "  ";
            }
            file << '\"' << key_ << "\": {" << '\n';
            for (size_t i = 0; i < values_.size(); i++) {
                values_[i]->WriteJSON(file, tabs + 1);
                if (i != values_.size() - 1) {
                    file << ',';
                }
                file << '\n';
            }
            for (size_t j = 0; j < tabs; j++) {
                file << "  ";
            }
            file << '}';
        }

    };

    Section& parse(const std::string& code);

    Section& parse(const std::filesystem::path& path);
}