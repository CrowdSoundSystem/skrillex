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

#include <ostream>
#include <string>

namespace skrillex {
    class Status {
    private:
        enum class Code {
            OK             = 0,
            NotFound       = 1,
            NotImplemented = 2,
            Error          = 3
        };

    public:
        Status()
        : code_(Code::OK)
        {
        }

        // We make it a friend operator so comparing the static
        // functions (i.e. Status::OK()) to a status doesn't cause
        // problems (looking at you, gtest)
        friend bool operator==(const Status& a, const Status& b);
        friend bool operator!=(const Status& a, const Status& b);

        friend std::ostream& operator<<(std::ostream& os, const Status& status);

        operator bool() const {
            return !ok();
        }

        static Status OK() {
            return Status();
        }

        static Status NotFound(std::string message) {
            return Status(Code::NotFound, message);
        }

        static Status NotImplemented(std::string message) {
            return Status(Code::NotImplemented, message);
        }

        static Status Error(std::string message) {
            return Status(Code::Error, message);
        }

        bool ok() const             { return code_ == Code::OK; }
        bool notFound() const       { return code_ == Code::NotFound; }
        bool NotImplemented() const { return code_ == Code::NotImplemented; }
        bool error() const          { return code_ == Code::Error; }

        std::string message() const {
            return message_;
        }

        std::string string() const {
            switch (code_) {
                case Code::OK:             return "Status: OK";
                case Code::NotFound:       return "Status: NotFound - " + message_;
                case Code::NotImplemented: return "Status: NotImplemented - " + message_;
                case Code::Error:          return "Status: Error - " + message_;
                default:                   return message_;
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

