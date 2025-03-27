#ifndef _PARSER_SQL_H
#define _PARSER_SQL_H

namespace sql {

class SelectStatement {
  public:
    SelectStatement() {}
};

class ParseResult {
  public:
    ParseResult(bool success) : success_(success) {}

    bool IsSuccess() const { return success_; }

  private:
    bool success_;
};

} // namespace sql

#endif // _PARSER_SQL_H_