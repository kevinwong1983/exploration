
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <boost/asio.hpp>
#include <time.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/system_error.hpp>
#include <functional>
#include <unordered_map>
#include <boost/variant.hpp>

using namespace std;
using ::testing::AtLeast;
using ::testing::Return;
using ::testing::_;

class DataBaseConnect {
public:
    virtual bool login(string username, string password) {return true;}
    virtual bool logout(string username) {return true;}
    virtual int fetchRecord() {return -1;}
};

class DataBaseConnectMock : public DataBaseConnect {
public:
    MOCK_METHOD0(fetchRecord, int());
    MOCK_METHOD1(logout, bool(string username));
    MOCK_METHOD2(login, bool(string username, string password));
};

class MyDataBase {
public:
    MyDataBase(DataBaseConnect& dbC) : dbC_(dbC) {
    }

    int Init(string username, string password) {
        if (dbC_.login(username, password) != true) {
            // ...
            if (dbC_.login(username, password) != true) {
                cout << "DB FAILURE" << endl;
                return -1;
            }
        }
        cout << "DB SUCCESS" << endl;
        return 1;
    }

private:
    DataBaseConnect &dbC_;
};

TEST(gmock, EXPECT_CALL_LoginTest_DataBaseConnectReturnsTrue) {
    // given
    DataBaseConnectMock mdb;
    MyDataBase db(mdb);

    EXPECT_CALL(mdb, login("Terminator", "I'm Back")) // login(_, _) dont care about parameters
    .Times(1)
    .WillOnce(Return(true));

    // when
    int rv = db.Init("Terminator", "I'm Back");

    // then
    EXPECT_EQ(rv, 1);
}

TEST(gmock, ON_CALL_LoginTest_DataBaseConnectReturnsTrue) {
    // given
    DataBaseConnectMock mdb;
    MyDataBase db(mdb);

    // difference between ON_CALL and EXPECT_CALL is that with
    // ON CALL just discribes what need to happen, when function is
    // called. It does not mandate the the function to be called.
    // However for EXPECT_CALL it does.
    // Only use when there is e.g. randomness in your code. And you
    // do not know if the function will get called.
    ON_CALL(mdb, login(_ , _))
            .WillByDefault(Return(true));

    // when
    int rv = db.Init("Terminator", "I'm Back");

    // then
    EXPECT_EQ(rv, 1);
}

TEST(gmock, EXPECT_CALL_oginTest_DataBaseConnectReturnsFalse) {
    // given
    DataBaseConnectMock mdb;
    MyDataBase db(mdb);
    EXPECT_CALL(mdb, login("Terminator", "I'm Back"))
            .Times(2)
            .WillRepeatedly(Return(false));

    // when
    int rv = db.Init("Terminator", "I'm Back");

    // then
    EXPECT_EQ(rv, -1);
}

// Google Mock can't mock a non-virtual function. To mock this class,
// we have to modify not just our test code, but the code under test
// too. We have to implement dependency injection via a template that
// accepts duck typing.

class DataBaseConnectMock2 {
public:
    MOCK_METHOD0(fetchRecord, int());
    MOCK_METHOD1(logout, bool(string username));
    MOCK_METHOD2(login, bool(string username, string password));
};

template <class Connect>
class MyDataBase2 {
public:
    MyDataBase2(Connect& dbC) : dbC_(dbC) {
    }

    int Init(string username, string password) {
        if (dbC_.login(username, password) != true) {
            // ...
            if (dbC_.login(username, password) != true) {
                cout << "DB FAILURE" << endl;
                return -1;
            }
        }
        cout << "DB SUCCESS" << endl;
        return 1;
    }

private:
    Connect &dbC_;
};

TEST(gmock, mocking_of_non_virtual_class) {
    // given
    DataBaseConnectMock2 mdb;
    MyDataBase2<DataBaseConnectMock2> db(mdb);

    EXPECT_CALL(mdb, login("Terminator", "I'm Back")) // login(_, _) dont care about parameters
            .Times(1)
            .WillOnce(Return(true));

    // when
    int rv = db.Init("Terminator", "I'm Back");

    // then
    EXPECT_EQ(rv, 1);
}

/////////////////////////////////////////////////////
// test fixtures
/////////////////////////////////////////////////////

class Stack {
public:
    void push(int value) {
        stack_.push_back(value);
    }
    int pop() {
        if (stack_.size() > 0) {
            int value = stack_.back();
            stack_.pop_back();
            return value;
        }
        return 0;
    }
private:
    vector<int> stack_ = {};
};

class StackFixture : public testing::Test {
public:
    Stack s;
    void SetUp(){
        int value[] = {1,2,3,4,5,6,7,8,9};
        for (auto&& val : value) {
            s.push(val);
        }
    }

    void TearDown(){}
};

TEST_F(StackFixture, PopTest) {
    int n = 9;
    while (n >= 1){
        EXPECT_EQ(s.pop(), n);
        n --;
    }
}