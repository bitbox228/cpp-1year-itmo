#include <lib/CCircularBuffer.h>
#include <gtest/gtest.h>
#include <algorithm>

template<typename T>
std::string get_output(const CCircularBuffer<T>& a) {
    std::string res;
    for (size_t i = 0; i < a.size(); i++) {
        res += a[i] + " ";
    }
    return res;
}

template<typename T>
std::string get_output_int(const CCircularBuffer<T>& a) {
    std::string res;
    for (size_t i = 0; i < a.size(); i++) {
        res += std::to_string(a[i]) + " ";
    }
    return res;
}

template<typename T>
std::string get_output(const CCircularBufferExt<T>& a) {
    std::string res;
    for (size_t i = 0; i < a.size(); i++) {
        res += a[i] + " ";
    }
    return res;
}

template<typename T>
std::string get_output_int(const CCircularBufferExt<T>& a) {
    std::string res;
    for (size_t i = 0; i < a.size(); i++) {
        res += std::to_string(a[i]) + " ";
    }
    return res;
}

/// STATIC BUFFER TESTS

TEST(CCircularBufferTestSuit, EmptyTest) {
    CCircularBuffer<int> buff;

    ASSERT_TRUE(buff.empty());
}

TEST(CCircularBufferTestSuit, BuffEqTest) {
    std::initializer_list<std::string> l{"ITMO", "239", "C++"};
    CCircularBuffer<std::string> buff1 = l;
    CCircularBuffer<std::string> buff2 = l;

    ASSERT_TRUE(buff1 == buff2);
    ASSERT_EQ(get_output(buff1), "ITMO 239 C++ ");
    ASSERT_EQ(get_output(buff2), "ITMO 239 C++ ");
}

TEST(CCircularBufferTestSuit, CopyConstructorTest) {
    CCircularBuffer<std::string> buff = {"ITMO", "239", "C++"};
    auto buff_copy(buff);

    ASSERT_TRUE(buff == buff_copy);
}

TEST(CCircularBufferTestSuit, CopyAssignTest) {
    CCircularBuffer<std::string> buff = {"beITMO", "beHEALTHY", "beFRIENDLY", "beOPEN", "bePRO", "beFIT", "beECO"};
    CCircularBuffer<std::string> new_buff = {"Vyazma", "Running Club"};
    new_buff = buff;

    ASSERT_TRUE(buff == new_buff);
}

TEST(CCircularBufferTestSuit, IteratorsTest) {
    CCircularBuffer<int64_t> buff = {21321321, 3213, 3233726713};
    auto it_beg = buff.begin();
    *(it_beg + 1) = 100;
    it_beg[2] = 200;
    auto it_end = buff.end();

    ASSERT_EQ(*it_beg, 21321321);
    ASSERT_EQ(*(it_end + 1), 21321321);
    ASSERT_EQ(it_beg[1], 100);
    ASSERT_EQ(*(it_beg + 2), 200);
}

TEST(CCircularBufferTestSuit, IteratorsArithmeticTest) {
    CCircularBuffer<int64_t> buff = {21321321, 3213, 3233726713};
    auto it1 = buff.begin();
    auto it2 = buff.begin() + 2;

    ASSERT_EQ(it1 + 2, it2);
    ASSERT_EQ(it2 - 2, it1);
    ASSERT_EQ(it2 - it1, 2);
    ASSERT_EQ(it1 - it2, -2);
    ASSERT_EQ(it1 - it1, 0);
}

TEST(CCircularBufferTestSuit, IteratorsComparisonTest) {
    CCircularBuffer<int64_t> buff = {21321321, 3213, 3233726713};
    auto it1 = buff.begin();
    auto it3 = buff.begin();
    auto it2 = buff.begin() + 2;

    ASSERT_TRUE(it1 < it2);
    ASSERT_FALSE(it1 > it2);
    ASSERT_TRUE(it1 == it3);
    ASSERT_FALSE(it1 == it2);
    ASSERT_TRUE(it1 != it2);
    ASSERT_TRUE(it1 <= it3);
    ASSERT_FALSE(it1 >= it2);
}

TEST(CCircularBufferTestSuit, IndexTest) {
    CCircularBuffer<std::string> buff = {"nikita", "ITMO", "C++"};

    ASSERT_EQ(buff[0], "nikita");
    ASSERT_EQ(buff[1], "ITMO");
    ASSERT_EQ(buff[2], "C++");
}

TEST(CCircularBufferTestSuit, EqualTest) {
    CCircularBuffer<size_t> buff = {1, 2, 3, 4, 5};
    auto buff_copy(buff);
    CCircularBuffer<size_t> new_buff = {1, 2, 3, 4, 5};

    ASSERT_TRUE(buff == buff_copy);
    ASSERT_EQ(buff == buff_copy, std::equal(buff.begin(), buff.end(), buff_copy.begin(), buff_copy.end()));
    ASSERT_TRUE(buff == new_buff);
    ASSERT_EQ(buff == new_buff, std::equal(buff.begin(), buff.end(), new_buff.begin(), new_buff.end()));
}

TEST(CCircularBufferTestSuit, SwapTest) {
    CCircularBuffer<size_t> a = {1, 2, 3, 4, 5};
    CCircularBuffer<size_t> b = {1, 2, 3};
    auto a_copy(a);
    auto b_copy(b);
    swap(a_copy, b_copy);
    auto a_another_copy(a);
    auto b_another_copy(b);
    a_another_copy.swap(b_another_copy);

    ASSERT_TRUE(b_copy == a);
    ASSERT_TRUE(a_copy == b);
    ASSERT_TRUE(a_copy != b_copy);
    ASSERT_TRUE(b_another_copy == a);
    ASSERT_TRUE(a_another_copy == b);
    ASSERT_TRUE(a_another_copy != b_another_copy);
}

TEST(CCircularBufferTestSuit, SizeTest) {
    CCircularBuffer<size_t> a = {1, 2, 3, 4, 5, 6, 7};
    auto b(a);
    b.resize(100);

    ASSERT_EQ(a.size(), 7);
    ASSERT_EQ(b.size(), 100);
}

TEST(CCircularBufferTestSuit, CapacityTest) {
    CCircularBuffer<size_t> a = {1, 2, 3, 4, 5, 6, 7};
    auto b(a);
    b.reserve(100);

    ASSERT_EQ(a.capacity(), 7);
    ASSERT_EQ(b.capacity(), 100);
}

TEST(CCircularBufferTestSuit, NCopiesConstructorTest) {
    CCircularBuffer<size_t> a(20, 128);

    ASSERT_EQ(std::distance(a.begin(), a.end()), 20);
}

TEST(CCircularBufferTestSuit, RangeConstructorTest) {
    std::vector<int> a = {1, 2, 3, 4, 5};
    CCircularBuffer<int> b(a.begin(), a.end());

    ASSERT_EQ(std::distance(b.begin(), b.end()), std::distance(a.begin(), a.end()));
    ASSERT_EQ(b.size(), 5);
    ASSERT_EQ(b[4], 5);
}

TEST(CCircularBufferTestSuit, InitListConstructorTest) {
    std::initializer_list<int> l = {1, 2, 3, 4, 5};
    CCircularBuffer<int> a(l);
    CCircularBuffer<int> b(l.begin(), l.end());

    ASSERT_EQ(std::distance(b.begin(), b.end()), std::distance(a.begin(), a.end()));
    ASSERT_EQ(a.size(), 5);
    ASSERT_EQ(a[3], 4);
}

TEST(CCircularBufferTestSuit, InitListAssignTest) {
    std::initializer_list<int> l = {1, 2, 3, 4, 5};
    CCircularBuffer<int> a{1, 2, 3};
    a = l;

    ASSERT_EQ(std::distance(a.begin(), a.end()), std::distance(l.begin(), l.end()));
    ASSERT_EQ(a.size(), 5);
    ASSERT_EQ(a[4], 5);
}

TEST(CCircularBufferTestSuit, FrontBackTest) {
    CCircularBuffer<size_t> l = {111, 222, 333};

    ASSERT_EQ(l.front(), 111);
    ASSERT_EQ(l.back(), 333);
}

TEST(CCircularBufferTestSuit, PushBackTest) {
    CCircularBuffer<size_t> a = {111, 222, 333};
    a.push_back(322);

    ASSERT_EQ(a.back(), 322);
    ASSERT_EQ(get_output_int(a), "222 333 322 ");
}

TEST(CCircularBufferTestSuit, PushFrontTest) {
    CCircularBuffer<size_t> a = {111, 222, 333};
    a.push_front(322);

    ASSERT_EQ(a.front(), 322);
    ASSERT_EQ(get_output_int(a), "322 111 222 ");
}

TEST(CCircularBufferTestSuit, EraseOneTest) {
    CCircularBuffer<size_t> a = {111, 222, 333, 4, 5};
    a.erase(a.cbegin() + 2);

    ASSERT_EQ(a.size(), 4);
    ASSERT_EQ(get_output_int(a), "111 222 4 5 ");
}

TEST(CCircularBufferTestSuit, EraseManyTest) {
    CCircularBuffer<size_t> a = {111, 222, 333, 4, 5};
    a.erase(a.cbegin() + 1, a.cbegin() + 4);

    ASSERT_EQ(a.size(), 2);
    ASSERT_EQ(get_output_int(a), "111 5 ");
}

TEST(CCircularBufferTestSuit, ClearTest) {
    CCircularBuffer<size_t> a = {111, 222, 333, 4, 5};
    a.clear();

    ASSERT_TRUE(a.empty());
}

TEST(CCircularBufferTestSuit, AssignTest) {
    std::vector<size_t> a{1, 2, 3, 4};
    CCircularBuffer<size_t> b{111, 222, 333, 4, 5};
    b.assign(a.begin(), a.end());

    ASSERT_EQ(a.size(), b.size());
    ASSERT_EQ(get_output_int(b), "1 2 3 4 ");
}

TEST(CCircularBufferTestSuit, AssignNTest) {
    CCircularBuffer<size_t> b{111, 222, 333, 4, 5};
    b.assign(10, 10);

    ASSERT_EQ(10, b.size());
    ASSERT_EQ(get_output_int(b), "10 10 10 10 10 10 10 10 10 10 ");
}

TEST(CCircularBufferTestSuit, SortTest) {
    CCircularBuffer<int> a{100, -1, -2, 25, 5};
    std::sort(a.begin(), a.end());

    ASSERT_EQ(get_output_int(a), "-2 -1 5 25 100 ");
}

/// EXTENDED BUFFER TESTS

TEST(CCircularBufferExtTestSuit, EmptyTest) {
    CCircularBufferExt<int> buff;

    ASSERT_TRUE(buff.empty());
}

TEST(CCircularBufferExtTestSuit, BuffEqTest) {
    std::initializer_list<std::string> l{"ITMO", "239", "C++"};
    CCircularBufferExt<std::string> buff1 = l;
    CCircularBufferExt<std::string> buff2 = l;

    ASSERT_TRUE(buff1 == buff2);
    ASSERT_EQ(get_output(buff1), "ITMO 239 C++ ");
    ASSERT_EQ(get_output(buff2), "ITMO 239 C++ ");
}

TEST(CCircularBufferExtTestSuit, CopyConstructorTest) {
    CCircularBufferExt<std::string> buff = {"ITMO", "239", "C++"};
    auto buff_copy(buff);

    ASSERT_TRUE(buff == buff_copy);
}

TEST(CCircularBufferExtTestSuit, CopyAssignTest) {
    CCircularBufferExt<std::string> buff = {"beITMO", "beHEALTHY", "beFRIENDLY", "beOPEN", "bePRO", "beFIT", "beECO"};
    CCircularBufferExt<std::string> new_buff = {"Vyazma", "Running Club"};
    new_buff = buff;

    ASSERT_TRUE(buff == new_buff);
}

TEST(CCircularBufferExtTestSuit, IteratorsTest) {
    CCircularBufferExt<int64_t> buff = {21321321, 3213, 3233726713};
    auto it_beg = buff.begin();
    *(it_beg + 1) = 100;
    it_beg[2] = 200;
    auto it_end = buff.end();

    ASSERT_EQ(*it_beg, 21321321);
    ASSERT_EQ(*(it_end + 1), 21321321);
    ASSERT_EQ(it_beg[1], 100);
    ASSERT_EQ(*(it_beg + 2), 200);
}

TEST(CCircularBufferExtTestSuit, IteratorsArithmeticTest) {
    CCircularBufferExt<int64_t> buff = {21321321, 3213, 3233726713};
    auto it1 = buff.begin();
    auto it2 = buff.begin() + 2;

    ASSERT_EQ(it1 + 2, it2);
    ASSERT_EQ(it2 - 2, it1);
    ASSERT_EQ(it2 - it1, 2);
    ASSERT_EQ(it1 - it2, -2);
    ASSERT_EQ(it1 - it1, 0);
}

TEST(CCircularBufferExtTestSuit, IteratorsComparisonTest) {
    CCircularBufferExt<int64_t> buff = {21321321, 3213, 3233726713};
    auto it1 = buff.begin();
    auto it3 = buff.begin();
    auto it2 = buff.begin() + 2;

    ASSERT_TRUE(it1 < it2);
    ASSERT_FALSE(it1 > it2);
    ASSERT_TRUE(it1 == it3);
    ASSERT_FALSE(it1 == it2);
    ASSERT_TRUE(it1 != it2);
    ASSERT_TRUE(it1 <= it3);
    ASSERT_FALSE(it1 >= it2);
}

TEST(CCircularBufferExtTestSuit, IndexTest) {
    CCircularBufferExt<std::string> buff = {"nikita", "ITMO", "C++"};

    ASSERT_EQ(buff[0], "nikita");
    ASSERT_EQ(buff[1], "ITMO");
    ASSERT_EQ(buff[2], "C++");
}

TEST(CCircularBufferExtTestSuit, EqualTest) {
    CCircularBufferExt<size_t> buff = {1, 2, 3, 4, 5};
    auto buff_copy(buff);
    CCircularBufferExt<size_t> new_buff = {1, 2, 3, 4, 5};

    ASSERT_TRUE(buff == buff_copy);
    ASSERT_EQ(buff == buff_copy, std::equal(buff.begin(), buff.end(), buff_copy.begin(), buff_copy.end()));
    ASSERT_TRUE(buff == new_buff);
    ASSERT_EQ(buff == new_buff, std::equal(buff.begin(), buff.end(), new_buff.begin(), new_buff.end()));
}

TEST(CCircularBufferExtTestSuit, SwapTest) {
    CCircularBufferExt<size_t> a = {1, 2, 3, 4, 5};
    CCircularBufferExt<size_t> b = {1, 2, 3};
    auto a_copy(a);
    auto b_copy(b);
    swap(a_copy, b_copy);
    auto a_another_copy(a);
    auto b_another_copy(b);
    a_another_copy.swap(b_another_copy);

    ASSERT_TRUE(b_copy == a);
    ASSERT_TRUE(a_copy == b);
    ASSERT_TRUE(a_copy != b_copy);
    ASSERT_TRUE(b_another_copy == a);
    ASSERT_TRUE(a_another_copy == b);
    ASSERT_TRUE(a_another_copy != b_another_copy);
}

TEST(CCircularBufferExtTestSuit, SizeTest) {
    CCircularBufferExt<size_t> a = {1, 2, 3, 4, 5, 6, 7};
    auto b(a);
    b.resize(100);

    ASSERT_EQ(a.size(), 7);
    ASSERT_EQ(b.size(), 100);
}

TEST(CCircularBufferExtTestSuit, CapacityTest) {
    CCircularBufferExt<size_t> a = {1, 2, 3, 4, 5, 6, 7};
    auto b(a);
    b.reserve(100);

    ASSERT_EQ(a.capacity(), 7);
    ASSERT_EQ(b.capacity(), 100);
}

TEST(CCircularBufferExtTestSuit, NCopiesConstructorTest) {
    CCircularBufferExt<size_t> a(20, 128);

    ASSERT_EQ(std::distance(a.begin(), a.end()), 20);
}

TEST(CCircularBufferExtTestSuit, RangeConstructorTest) {
    std::vector<int> a = {1, 2, 3, 4, 5};
    CCircularBufferExt<int> b(a.begin(), a.end());

    ASSERT_EQ(std::distance(b.begin(), b.end()), std::distance(a.begin(), a.end()));
    ASSERT_EQ(b.size(), 5);
    ASSERT_EQ(b[4], 5);
}

TEST(CCircularBufferExtTestSuit, InitListConstructorTest) {
    std::initializer_list<int> l = {1, 2, 3, 4, 5};
    CCircularBufferExt<int> a(l);
    CCircularBufferExt<int> b(l.begin(), l.end());

    ASSERT_EQ(std::distance(b.begin(), b.end()), std::distance(a.begin(), a.end()));
    ASSERT_EQ(a.size(), 5);
    ASSERT_EQ(a[3], 4);
}

TEST(CCircularBufferExtTestSuit, InitListAssignTest) {
    std::initializer_list<int> l = {1, 2, 3, 4, 5};
    CCircularBufferExt<int> a{1, 2, 3};
    a = l;

    ASSERT_EQ(std::distance(a.begin(), a.end()), std::distance(l.begin(), l.end()));
    ASSERT_EQ(a.size(), 5);
    ASSERT_EQ(a[4], 5);
}

TEST(CCircularBufferExtTestSuit, FrontBackTest) {
    CCircularBufferExt<size_t> l = {111, 222, 333};

    ASSERT_EQ(l.front(), 111);
    ASSERT_EQ(l.back(), 333);
}

TEST(CCircularBufferExtTestSuit, PushBackTest) {
    CCircularBufferExt<size_t> a = {111, 222, 333};
    a.push_back(322);

    ASSERT_EQ(a.back(), 322);
    ASSERT_EQ(get_output_int(a), "111 222 333 322 ");
}

TEST(CCircularBufferExtTestSuit, PushFrontTest) {
    CCircularBufferExt<size_t> a = {111, 222, 333};
    a.push_front(322);

    ASSERT_EQ(a.front(), 322);
    ASSERT_EQ(get_output_int(a), "322 111 222 333 ");
}

TEST(CCircularBufferExtTestSuit, EraseOneTest) {
    CCircularBufferExt<size_t> a = {111, 222, 333, 4, 5};
    a.erase(a.cbegin() + 2);

    ASSERT_EQ(a.size(), 4);
    ASSERT_EQ(get_output_int(a), "111 222 4 5 ");
}

TEST(CCircularBufferExtTestSuit, EraseManyTest) {
    CCircularBufferExt<size_t> a = {111, 222, 333, 4, 5};
    a.erase(a.cbegin() + 1, a.cbegin() + 4);

    ASSERT_EQ(a.size(), 2);
    ASSERT_EQ(get_output_int(a), "111 5 ");
}

TEST(CCircularBufferExtTestSuit, InsertOneTest) {
    CCircularBufferExt<std::string> a = {"It's", "more", "university"};
    auto it = a.insert(a.cbegin() + 2, "than");

    ASSERT_EQ(*it, "than");
    ASSERT_EQ(get_output(a), "It's more than university ");
}

TEST(CCircularBufferExtTestSuit, InsertManyTest) {
    CCircularBufferExt<std::string> a = {"It's", "more", "than", "university"};
    auto it = a.insert(a.cbegin() + 2, 5, "and more");

    ASSERT_EQ(*it, "and more");
    ASSERT_EQ(get_output(a), "It's more and more and more and more and more and more than university ");
}

TEST(CCircularBufferExtTestSuit, InsertIterTest) {
    std::vector<std::string> words = {"more", "than"};
    CCircularBufferExt<std::string> a = {"It's", "university"};
    auto it = a.insert(a.cbegin() + 1, words.begin(), words.end());

    ASSERT_EQ(*it, "more");
    ASSERT_EQ(get_output(a), "It's more than university ");
}

TEST(CCircularBufferExtTestSuit, ClearTest) {
    CCircularBufferExt<size_t> a = {111, 222, 333, 4, 5};
    a.clear();

    ASSERT_TRUE(a.empty());
}

TEST(CCircularBufferExtTestSuit, AssignTest) {
    std::vector<size_t> a{1, 2, 3, 4};
    CCircularBufferExt<size_t> b{111, 222, 333, 4, 5};
    b.assign(a.begin(), a.end());

    ASSERT_EQ(a.size(), b.size());
    ASSERT_EQ(get_output_int(b), "1 2 3 4 ");
}

TEST(CCircularBufferExtTestSuit, AssignNTest) {
    CCircularBufferExt<size_t> b{111, 222, 333, 4, 5};
    b.assign(10, 10);

    ASSERT_EQ(10, b.size());
    ASSERT_EQ(get_output_int(b), "10 10 10 10 10 10 10 10 10 10 ");
}

TEST(CCircularBufferExtTestSuit, CapacityCheckTest) {
    CCircularBufferExt<int> a{111, 222, 333, 4, 5};
    ASSERT_EQ(a.size(), 5);
    ASSERT_EQ(a.capacity(), 5);

    a.push_back(10);
    ASSERT_EQ(a.size(), 6);
    ASSERT_EQ(a.capacity(), 10);

    for (size_t i = 0; i < 5; i++) {
        a.push_back(10);
    }
    ASSERT_EQ(a.size(), 11);
    ASSERT_EQ(a.capacity(), 20);
}

TEST(CCircularBufferExtTestSuit, SortTest) {
    CCircularBufferExt<int> a{100, -1, -2, 25, 5};
    std::sort(a.begin(), a.end());

    ASSERT_EQ(get_output_int(a), "-2 -1 5 25 100 ");
}