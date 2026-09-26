#include <gtest/gtest.h>
#include "../TimeParser.h"

// Test suite: TimeParserTest
TEST(TimeParserTest, TestCaseCorrectTime) {

    char time_test[] = "000005";
    ASSERT_EQ(time_parse(time_test),5);

    char time_test2[] = "000105";
    ASSERT_EQ(time_parse(time_test2),65);

}

TEST(TimeParserTest, TestCaseInCorrectTime) {

    char time_test[] = "000075";
    ASSERT_EQ(time_parse(time_test),TIME_VALUE_ERROR );

    char time_test2[] = "009000";
    ASSERT_EQ(time_parse(time_test2),TIME_VALUE_ERROR );

}

TEST(TimeParserTest, TestCaseCorrectBoundary) {

    char boundary_test[] = "000059";
    ASSERT_EQ(time_parse(boundary_test),59);

    char boundary_test2[] = "005900";
    ASSERT_EQ(time_parse(boundary_test2),3540);

    char boundary_test3[] = "230000";
    ASSERT_EQ(time_parse(boundary_test3),0);

}

TEST(TimeParserTest, TestCaseInCorrectBoundary) {

    char boundary_test[] = "000060";
    ASSERT_EQ(time_parse(boundary_test),TIME_VALUE_ERROR);

    char boundary_test2[] = "006000";
    ASSERT_EQ(time_parse(boundary_test2),TIME_VALUE_ERROR);

    char boundary_test3[] = "240000";
    ASSERT_EQ(time_parse(boundary_test3),TIME_VALUE_ERROR);

}

TEST(TimeParserTest, TestCaseInCorrectLen) {

    char len_test[] = "0005";
    ASSERT_EQ(time_parse(len_test),TIME_LEN_ERROR);

    char len_test2[] = "00105";
    ASSERT_EQ(time_parse(len_test2),TIME_LEN_ERROR);
}

TEST(TimeParserTest, TestCaseNull) {

    ASSERT_EQ(time_parse(NULL),TIME_ARRAY_ERROR);

}

TEST(TimeParserTest, TestCaseOnlyNumbers) {

    char numbers_test[] = "00aa11";
    ASSERT_EQ(time_parse(numbers_test),TIME_VALUE_ERROR);

    char numbers2_test[] = "00B!-1";
    ASSERT_EQ(time_parse(numbers2_test),TIME_VALUE_ERROR);

}



// https://google.github.io/googletest/reference/testing.html
// https://google.github.io/googletest/reference/assertions.html
