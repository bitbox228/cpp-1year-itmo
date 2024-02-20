#include <lib/database.h>
#include <gtest/gtest.h>


using namespace MyCoolNamespace;

std::vector<std::string> SplitString(const std::string& args) {
    std::regex separator(R"(\s+)");
    std::vector<std::string> result;
    std::sregex_token_iterator iter(args.begin(), args.end(), separator, -1);
    std::sregex_token_iterator end;
    while (iter != end) {
        result.push_back(*iter);
        ++iter;
    }
    return result;
}

TEST(DataBaseTestSuit, CreateTableTest) {
    testing::internal::CaptureStdout();

    MyCoolDataBase db("Cool DB");
    db.Parse(R"( creAtE    tAblE  Students    (ISU int prImary Key,    age int  ,name  varchar(255),  );)");
    db.Parse(R"(   select * from Students ;)");
    auto output = testing::internal::GetCapturedStdout();
    std::string expected = "ISU age name";
    ASSERT_EQ(SplitString(output), SplitString(expected));
}

TEST(DataBaseTestSuit, CreateTableExceptTest) {
    MyCoolDataBase db("Cool DB");
    EXPECT_THROW(db.Parse(R"( CREATE TABLE Students(ISU int PRIMARY KEY,age int  ,name  varchar(255) ))"), QueryException);
    EXPECT_THROW(db.Parse(R"( CREATE TABLE Students(ISU int ,ISU int  ,name  varchar(255) );)"), QueryException);
    EXPECT_THROW(db.Parse(R"( CREATE TABLE Students(int ,int  , varchar(255) );)"), QueryException);
    EXPECT_THROW(db.Parse(R"( CREATE TABLE Students( );)"), QueryException);
}

TEST(DataBaseTestSuit, DropTableTest) {
    MyCoolDataBase db("Cool DB");
    db.Parse(R"( creAtE    tAblE  Students    (ISU int prImary Key,    age int  ,name  varchar(255) );)");
    db.Parse(R"(DROP TABLE   Students;)");
    EXPECT_THROW(db.Parse(R"( select * from Students; )"), LogicErrorException);
}

TEST(DataBaseTestSuit, InsertIntoTest) {
    testing::internal::CaptureStdout();

    MyCoolDataBase db("Cool DB");
    db.Parse(R"( creAtE    tAblE  Students    (ISU int prImary Key,    age int  ,name  varchar(255) );)");
    db.Parse(R"(Insert into Students   values  (368050, 18  , 'Nikita' );)");
    db.Parse(R"(Insert into Students (name , age, ISU)   values  (Alex, 19  , 35423 );)");
    db.Parse(R"(Insert into Students  (ISU) values  (238903);)");
    db.Parse(R"(select * from Students;)");
    auto output = testing::internal::GetCapturedStdout();
    std::string expected = "ISU age name 368050 18 Nikita 35423 19 Alex 238903 NULL NULL";
    ASSERT_EQ(SplitString(output), SplitString(expected));
}

TEST(DataBaseTestSuit, InsertExceptTest) {
    MyCoolDataBase db("Cool DB");
    db.Parse(R"( CREATE TABLE Students(ISU int PRIMARY KEY,age int  ,name  varchar(255) );)");
    EXPECT_THROW(db.Parse(R"(Insert into Students values (5, 5, "A"))"), QueryException);
    EXPECT_THROW(db.Parse(R"(Insert into Students values (5, 5);)"), QueryException);
    EXPECT_THROW(db.Parse(R"(Insert into Students values (ABC, 5, "A");)"), QueryException);
    EXPECT_THROW(db.Parse(R"(Insert into (ISU, name) Students values (Nikita, 5);)"), QueryException);
}

TEST(DataBaseTestSuit, DeleteFromTest) {
    testing::internal::CaptureStdout();

    MyCoolDataBase db("Cool DB");
    db.Parse(R"( creAtE    tAblE  Students    (ISU int prImary Key,    age int  ,name  varchar(255) );)");
    db.Parse(R"(Insert into Students   values  (368050, 27  , 'Nikita' );)");
    db.Parse(R"(Insert into Students   values  (370213, 31678  , 'Matvey' );)");
    db.Parse(R"(Insert into Students   values  (111111, 18  , 'Alex' );)");
    db.Parse(R"(Insert into Students   values  (789213, -100  , 'Stas' );)");
    db.Parse(R"(Delete from Students where age < 20;)");
    db.Parse(R"(Select * from Students;)");
    auto output = testing::internal::GetCapturedStdout();
    std::string expected = "ISU age name 368050 27 Nikita 370213 31678 Matvey";
    ASSERT_EQ(SplitString(output), SplitString(expected));
}

TEST(DataBaseTestSuit, DeleteFromWoutWhereTest) {
    testing::internal::CaptureStdout();

    MyCoolDataBase db("Cool DB");
    db.Parse(R"( creAtE    tAblE  Students    (ISU int prImary Key,    age int  ,name  varchar(255) );)");
    db.Parse(R"(Insert into Students   values  (368050, 27  , 'Nikita' );)");
    db.Parse(R"(Insert into Students   values  (370213, 31678  , 'Matvey' );)");
    db.Parse(R"(Insert into Students   values  (111111, 18  , 'Alex' );)");
    db.Parse(R"(Insert into Students   values  (789213, -100  , 'Stas' );)");
    db.Parse(R"(Delete from Students;)");
    db.Parse(R"(Select * from Students;)");
    auto output = testing::internal::GetCapturedStdout();
    std::string expected = "ISU age name";
    ASSERT_EQ(SplitString(output), SplitString(expected));
}

TEST(DataBaseTestSuit, UpdateTest) {
    testing::internal::CaptureStdout();

    MyCoolDataBase db("Cool DB");
    db.Parse(R"( creAtE    tAblE  Students    (ISU int prImary Key,    age int  ,name  varchar(255) );)");
    db.Parse(R"(Insert into Students   values  (368050, 27  , 'Nikita' );)");
    db.Parse(R"(Insert into Students   values  (370213, 31678  , 'Matvey' );)");
    db.Parse(R"(Insert into Students   values  (111111, 18  , 'Alex' );)");
    db.Parse(R"(Insert into Students   values  (789213, -100  , 'Stas' );)");
    db.Parse(R"(Update Students set ISU = 5, age = 5 where name = Nikita;)");
    db.Parse(R"(Select * from Students;)");
    auto output = testing::internal::GetCapturedStdout();
    std::string expected = "ISU age name 5 5 Nikita 370213 31678 Matvey 111111 18 Alex 789213 -100 Stas";
    ASSERT_EQ(SplitString(output), SplitString(expected));
}

TEST(DataBaseTestSuit, UpdateWoutWhereTest) {
    testing::internal::CaptureStdout();

    MyCoolDataBase db("Cool DB");
    db.Parse(R"( creAtE    tAblE  Students    (ISU int prImary Key,    age int  ,name  varchar(255) );)");
    db.Parse(R"(Insert into Students   values  (368050, 27  , 'Nikita' );)");
    db.Parse(R"(Insert into Students   values  (370213, 31678  , 'Matvey' );)");
    db.Parse(R"(Insert into Students   values  (111111, 18  , 'Alex' );)");
    db.Parse(R"(Insert into Students   values  (789213, -100  , 'Stas' );)");
    db.Parse(R"(Update Students set ISU = 5, age = 5 ;)");
    db.Parse(R"(Select * from Students;)");
    auto output = testing::internal::GetCapturedStdout();
    std::string expected = "ISU age name 5 5 Nikita 5 5 Matvey 5 5 Alex 5 5 Stas";
    ASSERT_EQ(SplitString(output), SplitString(expected));
}

TEST(DataBaseTestSuit, WhereTest1) {
    testing::internal::CaptureStdout();

    MyCoolDataBase db("Cool DB");
    db.Parse(R"( creAtE    tAblE  Students    (ISU int prImary Key,    age int  ,name  varchar(255) );)");
    db.Parse(R"(Insert into Students   values  (368050, 27  , 'Nikita' );)");
    db.Parse(R"(Insert into Students   values  (370213, 31678  , 'Matvey' );)");
    db.Parse(R"(Insert into Students   values  (111111, 18  , 'Alex' );)");
    db.Parse(R"(Insert into Students   values  (789213, -100  , 'Stas' );)");
    db.Parse(R"(Select * from Students where name = Nikita OR age < 0;)");
    auto output = testing::internal::GetCapturedStdout();
    std::string expected = "ISU age name 368050 27 Nikita 789213 -100 Stas";
    ASSERT_EQ(SplitString(output), SplitString(expected));
}

TEST(DataBaseTestSuit, WhereTest2) {
    testing::internal::CaptureStdout();

    MyCoolDataBase db("Cool DB");
    db.Parse(R"( creAtE    tAblE  Students    (ISU int prImary Key,    age int  ,name  varchar(255) );)");
    db.Parse(R"(Insert into Students   values  (368050, 27  , 'Nikita' );)");
    db.Parse(R"(Insert into Students   values  (370213, 27  , 'Matvey' );)");
    db.Parse(R"(Insert into Students   values  (111111, 18  , 'Alex' );)");
    db.Parse(R"(Insert into Students   values  (789213, -100  , 'Stas' );)");
    db.Parse(R"(Select * from Students where age = 27 and ISU = 368050 OR age < 0;)");
    auto output = testing::internal::GetCapturedStdout();
    std::string expected = "ISU age name 368050 27 Nikita 789213 -100 Stas";
    ASSERT_EQ(SplitString(output), SplitString(expected));
}

TEST(DataBaseTestSuit, SelectTest) {
    testing::internal::CaptureStdout();

    MyCoolDataBase db("Cool DB");
    db.Parse(R"( creAtE    tAblE  Students    (ISU int prImary Key,    age int  ,name  varchar(255) );)");
    db.Parse(R"(Insert into Students   values  (368050, 27  , 'Nikita' );)");
    db.Parse(R"(Insert into Students   values  (370213, 27  , 'Matvey' );)");
    db.Parse(R"(Insert into Students   values  (111111, 18  , 'Alex' );)");
    db.Parse(R"(Insert into Students   values  (789213, -100  , 'Stas' );)");
    db.Parse(R"(Select name, ISU from Students;)");
    auto output = testing::internal::GetCapturedStdout();
    std::string expected = "name ISU Nikita 368050 Matvey 370213 Alex 111111 Stas 789213";
    ASSERT_EQ(SplitString(output), SplitString(expected));
}

TEST(DataBaseTestSuit, InnerJoinTest) {
    testing::internal::CaptureStdout();

    MyCoolDataBase db("Cool DB");
    db.Parse(R"( creAtE    tAblE  Students    (ISU int prImary Key,    age int  ,name  varchar(255) );)");
    db.Parse(R"(Insert into Students   values  (368050, 27  , 'Nikita' );)");
    db.Parse(R"(Insert into Students   values  (370213, 27  , 'Matvey' );)");
    db.Parse(R"(Insert into Students   values  (111111, 18  , 'Alex' );)");
    db.Parse(R"(Insert into Students   values  (789213, -100  , 'Stas' );)");
    db.Parse(R"(create table mark (ISU int primary key, mark int );)");
    db.Parse(R"(Insert into mark   values  (368050, 5 );)");
    db.Parse(R"(Insert into mark   values  (111111, 4 );)");
    db.Parse(R"(Insert into mark   values  (48321, 3 );)");
    db.Parse(R"(select Students.ISU, mark.mark from Students inner join mark on Students.ISU = mark.ISU;)");
    auto output = testing::internal::GetCapturedStdout();
    std::string expected = "ISU mark 368050 5 111111 4";
    ASSERT_EQ(SplitString(output), SplitString(expected));
    testing::internal::CaptureStdout();
    db.Parse(R"(select Students.ISU, mark.mark from Students inner join mark on Students.ISU = mark.ISU where mark = 5;)");
    output = testing::internal::GetCapturedStdout();
    expected = "ISU mark 368050 5";
    ASSERT_EQ(SplitString(output), SplitString(expected));
}

TEST(DataBaseTestSuit, LeftJoinTest) {
    testing::internal::CaptureStdout();

    MyCoolDataBase db("Cool DB");
    db.Parse(R"( creAtE    tAblE  Students    (ISU int prImary Key,    age int  ,name  varchar(255) );)");
    db.Parse(R"(Insert into Students   values  (368050, 27  , 'Nikita' );)");
    db.Parse(R"(Insert into Students   values  (370213, 27  , 'Matvey' );)");
    db.Parse(R"(Insert into Students   values  (111111, 18  , 'Alex' );)");
    db.Parse(R"(Insert into Students   values  (789213, -100  , 'Stas' );)");
    db.Parse(R"(create table mark (ISU int primary key, mark int );)");
    db.Parse(R"(Insert into mark   values  (368050, 5 );)");
    db.Parse(R"(Insert into mark   values  (111111, 4 );)");
    db.Parse(R"(Insert into mark   values  (48321, 3 );)");
    db.Parse(R"(select Students.ISU, mark.mark from Students left join mark on Students.ISU = mark.ISU;)");
    auto output = testing::internal::GetCapturedStdout();
    std::string expected = "ISU mark 368050 5 370213 NULL 111111 4 789213 NULL";
    ASSERT_EQ(SplitString(output), SplitString(expected));
    testing::internal::CaptureStdout();
    db.Parse(R"(select Students.ISU, mark.mark from Students inner join mark on Students.ISU = mark.ISU where mark = 5;)");
    output = testing::internal::GetCapturedStdout();
    expected = "ISU mark 368050 5";
    ASSERT_EQ(SplitString(output), SplitString(expected));
}

TEST(DataBaseTestSuit, RightJoinTest) {
    testing::internal::CaptureStdout();

    MyCoolDataBase db("Cool DB");
    db.Parse(R"( creAtE    tAblE  Students    (ISU int prImary Key,    age int  ,name  varchar(255) );)");
    db.Parse(R"(Insert into Students   values  (368050, 27  , 'Nikita' );)");
    db.Parse(R"(Insert into Students   values  (370213, 27  , 'Matvey' );)");
    db.Parse(R"(Insert into Students   values  (111111, 18  , 'Alex' );)");
    db.Parse(R"(Insert into Students   values  (789213, -100  , 'Stas' );)");
    db.Parse(R"(create table mark (ISU int primary key, mark int );)");
    db.Parse(R"(Insert into mark   values  (368050, 5 );)");
    db.Parse(R"(Insert into mark   values  (111111, 4 );)");
    db.Parse(R"(Insert into mark   values  (48321, 3 );)");
    db.Parse(R"(select Students.ISU, mark.mark from Students right join mark on Students.ISU = mark.ISU;)");
    auto output = testing::internal::GetCapturedStdout();
    std::string expected = "ISU mark 368050 5 111111 4 48321 3";
    ASSERT_EQ(SplitString(output), SplitString(expected));
    testing::internal::CaptureStdout();
    db.Parse(R"(select Students.ISU, mark.mark from Students inner join mark on Students.ISU = mark.ISU where mark = 5;)");
    output = testing::internal::GetCapturedStdout();
    expected = "ISU mark 368050 5";
    ASSERT_EQ(SplitString(output), SplitString(expected));
}

TEST(DataBaseTestSuit, PrimaryKeyErrorTest) {
    MyCoolDataBase db("Cool DB");
    db.Parse("create table students(ISU int primary key, name varchar(255));");
    db.Parse("insert into students values (1, 'Nikita');");
    EXPECT_THROW(db.Parse("insert into students values (1, 'Sasha');");, QueryException);
}

TEST(DataBaseTestSuit, NotNullTest) {
    MyCoolDataBase db("Cool DB");
    db.Parse("create table students(ISU int primary key, name varchar(255) NOT NULL);");
    EXPECT_THROW(db.Parse("insert into students values (1, null);");, QueryException);
}