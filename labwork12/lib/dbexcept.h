#include <exception>
#include <string>

#pragma once

namespace MyCoolNamespace {

    class DataBaseException : public std::exception {
    public:
        DataBaseException(const std::string &err_message) : err_message_(err_message) {}

        const char *what() const noexcept override {
            return err_message_.c_str();
        }

    private:
        std::string err_message_;
    };

    class QueryException : public DataBaseException { /// all errors with queries
    public:
        QueryException(const std::string &err_message) : DataBaseException(err_message) {}
    };

    class LogicErrorException : public QueryException { /// mainly errors with multiple keys
    public:
        LogicErrorException(const std::string& err_message) : QueryException(err_message) {}
    };

    class SyntaxErrorException : public QueryException { /// errors in syntax of queries
    public:
        SyntaxErrorException(const std::string& err_message) : QueryException(err_message) {}
    };

    class WriteFileException : public DataBaseException { /// errors in writing db to file
    public:
        WriteFileException(const std::string &err_message) : DataBaseException(err_message) {}
    };
}