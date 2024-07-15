#ifndef RESULT_H
#define RESULT_H

#include <string>

// The code in this file is partially from the following blog.
// https://agtn.hatenablog.com/entry/2016/07/01/233009

namespace result {

// Has either the expected value or an error.
// When T and E are different types, this implementation is used. Otherwise, the
// implementation below (template specialization) will be used.
// Usually, you should use result::Ok() or result::Error() below to instantiate
// this class. One of the typical use case is below;
//
// result::ResultVE<int, std::string> Div(int a, int b) {
//   if (b == 0) return result::Error("b should not be zero.");
//   return result::Ok(a/b);
// }
//
template <typename T, typename E> class ResultVE {
  public:
    explicit ResultVE(T const &ok) : tag_(Tag::Ok) { ok_ = ok; }
    explicit ResultVE(E const &error) : tag_(Tag::Error) { error_ = error; }
    ResultVE(ResultVE const &result) : tag_(result.tag_) {
        switch (tag_) {
        case Tag::Ok:
            ok_ = result.ok_;
            break;
        case Tag::Error:
            error_ = result.error_;
            break;
        }
    }

    // Returns true when this result represents ok.
    bool IsOk() const { return tag_ == Tag::Ok; }

    // Returns true when this result represents error value.
    bool IsError() const { return tag_ == Tag::Error; }

    // Returns the meaningful value.
    T const &Get() const {
        if (tag_ != Tag::Ok) { throw "Invalid Get operation"; }
        return ok_;
    }

    // Returns error value.
    E const &Error() const {
        if (tag_ != Tag::Error) { throw "Invalid Error operation"; }
        return error_;
    }

  private:
    enum class Tag { Ok, Error };
    Tag tag_;
    T ok_;
    E error_;
};

// Has either the expected value or an error.
// When T and E are the same type, this implementation is used.
// Usually, you should use result::Ok() or result::Error() below to instantiate
// this class. One of the typical use case is below;
//
// result::ResultVE<std::string, std::string> FetchUsername(int id) {
//   if (id < 0) return result::Error("id should not be zero.");
//   return result::Ok(GetUsername(id));
// }
//
template <typename T> class ResultVE<T, T> {
  public:
    ResultVE(T const &ok_or_error, bool is_ok) {
        if (is_ok) {
            tag_ = Tag::Ok;
            ok_  = ok_or_error;
        } else {
            tag_   = Tag::Error;
            error_ = ok_or_error;
        }
    }
    ResultVE(ResultVE const &result) : tag_(result.tag_) {
        switch (tag_) {
        case Tag::Ok:
            ok_ = result.ok_;
            break;
        case Tag::Error:
            error_ = result.error_;
            break;
        }
    }

    // Returns true when this result represents ok.
    bool IsOk() const { return tag_ == Tag::Ok; }

    // Returns true when this result represents error value.
    bool IsError() const { return tag_ == Tag::Error; }

    // Returns the meaningful value.
    T const &Get() const {
        if (tag_ != Tag::Ok) { throw "Invalid Get operation"; }
        return ok_;
    }

    // Returns error value.
    T const &Error() const {
        if (tag_ != Tag::Error) { throw "Invalid Error operation"; }
        return error_;
    }

  private:
    enum class Tag { Ok, Error };
    Tag tag_;
    T ok_;
    T error_;
};

// Has either the expected value or an error. The error is represented as
// a std::string.
// Usually, you should use result::Ok() or result::Error() below to instantiate
// this class. One of the typical use case is below;
//
// result::ResultV<std::string> FetchUsername(int id) {
//   if (id < 0) return result::Error("id should not be zero.");
//   return result::Ok(GetUsername(id));
// }
//
template <typename T> using ResultV = ResultVE<T, std::string>;

// Has an error or nothing. You should use this class when you don't have to
// return values when the function succeeds.
// Usually, you should use result::Ok() or result::Error() below to instantiate
// this class. One of the typical use case is below;
//
// result::Result Flush(int fd) {
//   if (fsync(fd) == -1) return result::Error("failed to flush").
//   return result::Ok();
// }
//
using Result = ResultV<int>;

// Has the expected value
template <typename T> class OkValue {
  public:
    explicit OkValue(T t) : t_(t) {}

    // Returns ResultVE<V, E>, which represents ok value
    template <typename V, typename E> operator ResultVE<V, E>() const {
        return ResultVE<V, E>(t_);
    }

    // Returns ResultVE<V, V>, which represents ok value
    template <typename V> operator ResultVE<V, V>() const {
        return ResultVE<V, V>(t_, true);
    }

  private:
    T t_;
};

// Returns OkValue<T>, which can be transformed to ResultVE<T, E>.
template <typename T> inline OkValue<T> Ok(T t) { return OkValue<T>(t); }

// Returns OkValue<T>, which can be transformed to ResultVE<T, E>.
OkValue<int> inline Ok() { return OkValue<int>(0); }

// Has an error
template <typename E> class ErrorValue {
  public:
    explicit ErrorValue(E error) : error_(error) {}

    // Returns ResultVE<V, E>, which represents error value
    template <typename T, typename F> operator ResultVE<T, F>() const {
        return ResultVE<T, F>(error_);
    }

    // Returns ResultVE<V, V>, which represents error value
    template <typename F> operator ResultVE<F, F>() const {
        return ResultVE<F, F>(error_, false);
    }

  private:
    E error_;
};

// Returns ErrorValue<E>, which can be transformed to ResultVE<T, E>.
template <typename E> inline ErrorValue<E> Error(E error) {
    return ErrorValue<E>(error);
}

} // namespace result

#endif // RESULT_H
