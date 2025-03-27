#ifndef _PARSER_SQL_H
#define _PARSER_SQL_H

#include <string>

namespace sql {

class ParseResult {
  public:
    ParseResult() {}

    bool IsError() const { return error_; }

    void SetError(const char *error_message) {
        error_         = true;
        error_message_ = error_message;
    }

    std::string Error() const { return error_message_; }

  private:
    bool error_ = false;
    std::string error_message_;
};

class SelectStatement {
  public:
    SelectStatement() {}
};

} // namespace sql

#endif // _PARSER_SQL_H_