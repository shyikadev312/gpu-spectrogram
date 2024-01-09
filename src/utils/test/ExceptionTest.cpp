#include <spectr/utils/Exception.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace spectr::utils::test
{
TEST(Exception, Empty)
{
    Exception ex{};
    EXPECT_STREQ(ex.what(), "");
}

TEST(Exception, CStringWithoutParameters)
{
    Exception ex("my exception");
    EXPECT_STREQ(ex.what(), "my exception");
}

TEST(Exception, StdStringWithoutParameters)
{
    const std::string s = "my exception";
    Exception ex(s);
    EXPECT_STREQ(ex.what(), "my exception");
}

TEST(Exception, StringWithOneParameter)
{
    Exception ex("my exception: {}", 42);
    EXPECT_STREQ(ex.what(), "my exception: 42");
}

TEST(Exception, StringWithSeveralParameters)
{
    Exception ex("my exception: {}, {}, {}", 42, "hello", 'c');
    EXPECT_STREQ(ex.what(), "my exception: 42, hello, c");
}
}
