//
// status.hpp
//
// The status of a DB operation.
//
// If the status is not OK, then an error message
// is provided.
//

#ifndef skrillex_status_hpp
#define skrillex_status_hpp

#include <string>

namespace skrillex {
    class Status {
    private:
        enum class Code {
            OK = 0,
            NotFound = 1,
            Error = 2
        };

    public:
        Status()
        : code_(Code::OK)
        {
        }

        static Status OK() {
            return Status();
        }

        static Status NotFound(std::string message) {
            return Status(Code::NotFound, message);
        }
        static Status Error(std::string message) {
            return Status(Code::Error, message);
        }

        bool ok() const       { return code_ == Code::OK; }
        bool notFound() const { return code_ == Code::NotFound; }
        bool error() const    { return code_ == Code::Error; }

        std::string message() const {
            return message_;
        }

        std::string string() const {
            switch (code_) {
                case Code::OK:       return "Status: OK";
                case Code::NotFound: return "Status: NotFound - " + message_;
                case Code::Error:    return "Status: Error - " + message_;
                default:             return message_;
            }
        }

    private:
        Status(Code code, std::string message)
        : code_(code)
        , message_(std::move(message))
        {
        }

    private:
        Code        code_;
        std::string message_;

    };
}

#endif

