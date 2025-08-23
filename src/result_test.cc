#include "result.h"
#include <gtest/gtest.h>
#include <memory>

TEST(ResultVE, IsOk) {
    const result::ResultVE<int, std::string> ok(4);
    EXPECT_TRUE(ok.IsOk());

    const result::ResultVE<int, std::string> err("aaaa");
    EXPECT_FALSE(err.IsOk());
}

result::Result fail() { return result::Error("fail"); }

result::Result test() {
    TRY_VALUE(value, fail());
    return result::Ok();
}

TEST(ResultVE, MacroTryReturn) {
    const auto result = test();
    EXPECT_TRUE(result.IsError()) << result.Error();
}

TEST(ResultVE, IsOkWhenValueAndErrorTypeIsSame) {
    const result::ResultVE<int, int> ok(4, true);
    EXPECT_TRUE(ok.IsOk());

    const result::ResultVE<int, int> err(0, false);
    EXPECT_FALSE(err.IsOk());
}

TEST(ResultVE, IsError) {
    const result::ResultVE<bool, int> ok(true);
    EXPECT_FALSE(ok.IsError());

    const result::ResultVE<bool, int> err(6);
    EXPECT_TRUE(err.IsError());
}

TEST(ResultVE, IsErrorWhenValueAndErrorTypeIsSame) {
    const result::ResultVE<int, int> ok(4, true);
    EXPECT_TRUE(ok.IsOk());

    const result::ResultVE<int, int> err(0, false);
    EXPECT_FALSE(err.IsOk());
}

TEST(ResultVE, Get) {
    const result::ResultVE<int, std::string> ok(3);
    EXPECT_EQ(ok.Get(), 3);
}

TEST(ResultVE, GetWhenValueAndErrorTypeIsSame) {
    const result::ResultVE<int, int> ok(3, true);
    EXPECT_EQ(ok.Get(), 3);
}

TEST(ResultVE, Error) {
    const result::ResultVE<int, std::string> err("error message");
    EXPECT_EQ(err.Error(), "error message");
}

TEST(ResultVE, ErrorWhenValueAndErrorTypeIsSame) {
    const result::ResultVE<int, int> ok(3, false);
    EXPECT_EQ(ok.Error(), 3);
}

TEST(ResultVE, CopyOkValueToErrorValue) {
    result::ResultVE<int, std::string> ok(3), error("aiueo");
    ok = error;
}

class WithReference {
  public:
    WithReference(const int &integer) : integer_(integer) {}

    const int &integer_;
};

TEST(ResultVE, HoldClassWithReferenceValue) {
    int four = 4;
    WithReference x(four);

    result::ResultV<WithReference> ok(x), err(std::string("error"));

    EXPECT_EQ(ok.Get().integer_, 4);
    EXPECT_EQ(err.Error(), "error");
}

TEST(ResultVE, UniquePtrInstance) {
    std::unique_ptr<int> x = std::make_unique<int>(5);
    result::ResultV<std::unique_ptr<int>> result(std::move(x));
    std::unique_ptr<int> moved_x = result.MoveValue();
    EXPECT_EQ(*moved_x, 5);
}

TEST(Ok, CorrectlyReturnsOk) {
    const result::ResultVE<int, std::string> ok = result::Ok(3);
    EXPECT_TRUE(ok.IsOk());
    EXPECT_EQ(ok.Get(), 3);
}

TEST(Ok, CorrectlyReturnsOkWhenValueAndErrorTypeIsSame) {
    const result::ResultVE<int, int> ok = result::Ok(3);
    EXPECT_TRUE(ok.IsOk());
    EXPECT_EQ(ok.Get(), 3);
}

TEST(Ok, ReturnsOkWithNoArgument) {
    const result::Result ok = result::Ok();
    EXPECT_TRUE(ok.IsOk());
}

TEST(Error, CorrectlyReturnsError) {
    const result::ResultVE<int, std::string> err =
        result::Error("error message");
    EXPECT_TRUE(err.IsError());
    EXPECT_EQ(err.Error(), "error message");
}

TEST(Error, CorrectlyReturnsErrorWhenValueAndErrorTypeIsSame) {
    const result::ResultVE<int, int> err = result::Error(3);
    EXPECT_TRUE(err.IsError());
    EXPECT_EQ(err.Error(), 3);
}

TEST(Error, CorrectlyMergeErrors) {
    result::ResultV<int> err1 = result::Error("aaa"),
                         err2 = result::Error("bbb");

    err1 = err1 + err2;
    EXPECT_TRUE(err1.IsError());
    EXPECT_EQ(err1.Error(), "aaa\nbbb");
}

TEST(Error, CorrectlyMergeErrorsOfString) {
    result::ResultV<std::string> err1 = result::Error("aaa"),
                                 err2 = result::Error("bbb");

    err1 = err1 + err2;
    EXPECT_TRUE(err1.IsError());
    EXPECT_EQ(err1.Error(), "aaa\nbbb");
}

TEST(Error, CorrectlyMergeErrorsWithErrorValue) {
    result::ResultV<int> err1 = result::Error("aaa");

    err1 = err1 + result::Error("bbb");
    EXPECT_TRUE(err1.IsError());
    EXPECT_EQ(err1.Error(), "aaa\nbbb");
}

result::ResultVE<int, std::string> test0(int a) {
    if (a == 0) return result::Error("a should not be zero");
    return result::Ok(a);
}

TEST(FunctionReturningResult, CorrectlyReturnsResult) {
    const auto ok_result  = test0(3);
    const auto err_result = test0(0);

    EXPECT_TRUE(ok_result.IsOk());
    EXPECT_EQ(ok_result.Get(), 3);
    EXPECT_TRUE(err_result.IsError());
    EXPECT_EQ(err_result.Error(), "a should not be zero");
}

result::ResultVE<std::string, std::string> test1(const std::string &a) {
    if (a.empty()) return result::Error("a should not be empty");
    return result::Ok(a);
}

TEST(FunctionReturningResult, WhenValueAndErrorTypeIsSame) {
    const auto ok_result  = test1("hello");
    const auto err_result = test1("");

    EXPECT_TRUE(ok_result.IsOk());
    EXPECT_EQ(ok_result.Get(), "hello");
    EXPECT_TRUE(err_result.IsError());
    EXPECT_EQ(err_result.Error(), "a should not be empty");
}

result::Result test2(int fd) {
    if (fd <= 0) return result::Error("fd should be plus.");
    return result::Ok();
}

TEST(FunctionReturningResult, UseOkWithNoArgument) {
    const auto ok_result  = test2(2);
    const auto err_result = test2(-1);

    EXPECT_TRUE(ok_result.IsOk());
    EXPECT_TRUE(err_result.IsError());
    EXPECT_EQ(err_result.Error(), "fd should be plus.");
}

result::ResultV<bool> test3(int a, int b) {
    if (a < 0 || b < 0) return result::Error("a and b should not be negative.");
    return result::Ok(a == b);
}

TEST(FunctionReturningResult, ResultVWithBoolSuccess) {
    const auto ok_result  = test3(2, 2);
    const auto ng_result  = test3(2, 3);
    const auto err_result = test3(-1, 2);

    EXPECT_TRUE(ok_result.IsOk());
    EXPECT_TRUE(ok_result.Get());
    EXPECT_TRUE(ng_result.IsOk());
    EXPECT_FALSE(ng_result.Get());
    EXPECT_TRUE(err_result.IsError());
}