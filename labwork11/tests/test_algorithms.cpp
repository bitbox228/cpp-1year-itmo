#include "lib/stl-algorithms.h"
#include "lib/xrange.h"
#include "lib/zip.h"

#include <gtest/gtest.h>

bool is_positive(int a) {
    return a > 0;
}

bool is_negative(int a) {
    return a < 0;
}

bool is_zero(int a) {
    return a == 0;
}

bool is_string_4_letters(const std::string& a) {
    return a.size() == 4;
}

bool comp_int(int a, int b) {
    return a < b;
}

bool comp_str(const std::string& a, const std::string& b) {
    return a.size() < b.size();
}

bool is_eq_int(int a, int b) {
    return a == b;
}

bool is_eq_str(const std::string& a, const std::string& b) {
    return a == b;
}

TEST(STLAlgorithmsTestSuit, all_of) {
    std::vector<int> vector = {1, 2, 3, 4, 5};

    ASSERT_TRUE(MySTL::all_of(vector.begin(), vector.end(), is_positive));
    ASSERT_FALSE(MySTL::all_of(vector.begin(), vector.end(), is_negative));
}

TEST(STLAlgorithmsTestSuit, any_of) {
    std::vector<int> vector = {1, 2, -3, 4, 5};
    std::vector<std::string> string_vector = {"239", "ITMO", "C++"};

    ASSERT_TRUE(MySTL::any_of(vector.begin(), vector.end(), is_positive));
    ASSERT_TRUE(MySTL::any_of(vector.begin(), vector.end(), is_negative));

    ASSERT_TRUE(MySTL::any_of(string_vector.begin(), string_vector.end(), is_string_4_letters));
}

TEST(STLAlgorithmsTestSuit, none_of) {
    std::vector<int> vector = {1, 2, -3, 4, 5};
    std::vector<std::string> string_vector = {"239", "University", "C++"};

    ASSERT_TRUE(MySTL::none_of(vector.begin(), vector.end(), is_zero));
    ASSERT_FALSE(MySTL::none_of(vector.begin(), vector.end(), is_positive));

    ASSERT_TRUE(MySTL::none_of(string_vector.begin(), string_vector.end(), is_string_4_letters));
}

TEST(STLAlgorithmsTestSuit, one_of) {
    std::vector<int> vector = {1, 2, -3, 4, 5};
    std::vector<std::string> string_vector = {"239", "ITMO", "C++"};

    ASSERT_TRUE(MySTL::one_of(vector.begin(), vector.end(), is_negative));
    ASSERT_FALSE(MySTL::one_of(vector.begin(), vector.end(), is_positive));

    ASSERT_TRUE(MySTL::one_of(string_vector.begin(), string_vector.end(), is_string_4_letters));
}

TEST(STLAlgorithmsTestSuit, is_sorted) {
    std::vector<int> vector = {1, 2, 3, 4, 5};
    std::vector<std::string> string_vector = {"239", "ITMO", "C+++++++"};

    ASSERT_TRUE(MySTL::is_sorted(vector.begin(), vector.end(), comp_int));
    ASSERT_FALSE(MySTL::is_sorted(vector.rend(), vector.rbegin(), comp_int));

    ASSERT_TRUE(MySTL::is_sorted(string_vector.begin(), string_vector.end(), comp_str));
}

TEST(STLAlgorithmsTestSuit, is_partitioned) {
    std::vector<int> vector = {1, 2, -1, -4, -5};
    std::vector<std::string> string_vector = {"239", "ITMO", "C++"};

    ASSERT_TRUE(MySTL::is_partitioned(vector.begin(), vector.end(), is_positive));
    ASSERT_FALSE(MySTL::is_partitioned(vector.rend(), vector.rbegin(), is_negative));

    ASSERT_FALSE(MySTL::is_partitioned(string_vector.begin(), string_vector.end(), is_string_4_letters));
}

TEST(STLAlgorithmsTestSuit, find_not) {
    std::vector<int> vector = {1, 1, 2, 1, 1};
    std::vector<std::string> string_vector = {"239", "ITMO", "C++"};

    ASSERT_EQ(*MySTL::find_not(vector.begin(), vector.end(), 1), 2);

    ASSERT_EQ(*MySTL::find_not(string_vector.begin(), string_vector.end(), "239"), "ITMO");
}

TEST(STLAlgorithmsTestSuit, find_backward) {
    std::vector<int> vector = {1, 1, 1, -3, 3};
    std::vector<std::string> string_vector = {"239", "ITMO", "C++"};

    ASSERT_EQ(MySTL::find_backward(vector.begin(), vector.end(), 1), vector.begin() + 2);

    ASSERT_EQ(MySTL::find_backward(string_vector.begin(), string_vector.end(), "C++"), string_vector.begin() + 2);
}

TEST(STLAlgorithmsTestSuit, is_palindrome) {
    std::vector<int> vector = {1, 2, 3, 2, 1};
    std::vector<std::string> string_vector = {"239", "ITMO", "239"};

    ASSERT_TRUE(MySTL::is_palindrome(vector.begin(), vector.end(), is_eq_int));

    ASSERT_TRUE(MySTL::is_palindrome(string_vector.begin(), string_vector.end(), is_eq_str));
}

TEST(xrangeTestSuite, IntTest) {
    auto x = xrange(-4, 2);
    std::vector<int> a(x.begin(), x.end());
    std::vector<int> result = {-4, -3, -2, -1, 0, 1};

    ASSERT_EQ(a, result);
}

bool is_eq_double(double a, double b) {
    return (abs(a - b) / b < 0.0001);
}

TEST(xrangeTestSuite, DoubleTest) {
    auto x = xrange(1.5, 5.5);
    std::vector<double> a(x.begin(), x.end());
    std::vector<double> result_a = {1.5, 2.5, 3.5, 4.5};

    x = xrange(1.5, 2.3, 0.1);
    std::vector<double> b(x.begin(), x.end());
    std::vector<double> result_b = {1.5, 1.6, 1.7, 1.8, 1.9, 2, 2.1, 2.2};


    ASSERT_TRUE(std::equal(b.begin(), b.end(), result_b.begin(), is_eq_double));
}

TEST(xrangeTestSuite, RangeTest) {
    auto x = xrange(2, 8, 2);
    std::vector<int> a;
    for (auto number: x) {
        a.push_back(number);
    }

    std::vector<double> result_a = {2, 4, 6};

    ASSERT_TRUE(std::equal(a.begin(), a.end(), result_a.begin()));
}

TEST(zipTestSuite, SimpleTest) {
    std::vector<int> a = {1, 2, 3, 4, 5};
    std::vector<char> b = {'a', 'b', 'c', 'd'};
    std::vector<std::pair<int,char>> result;
    std::vector<std::pair<int,char>> answer = {{1,'a'}, {2, 'b'}, {3, 'c'}, {4, 'd'}};
    for (auto pair: zip(a, b)) {
        result.push_back(pair);
    }

    ASSERT_TRUE(std::equal(result.begin(), result .end(), answer.begin()));
}