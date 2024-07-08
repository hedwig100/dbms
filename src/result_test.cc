#include "result.h"
#include <gtest/gtest.h>

TEST(ResultVE, IsOk) {
    const result::ResultVE<int, std::string> ok(4);
    EXPECT_TRUE(ok.IsOk());

    const result::ResultVE<int, std::string> err("aaaa");
    EXPECT_FALSE(err.IsOk());
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