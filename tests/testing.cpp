#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Account.h"
#include "Transaction.h"

using ::testing::Return;
using ::testing::Throw;
using ::testing::_;

class MockAccount : public Account {
public:
    MockAccount(int id, int balance) : Account(id, balance) {}
    MOCK_METHOD(int, GetBalance, (), (const, override));
    MOCK_METHOD(void, ChangeBalance, (int diff), (override));
    MOCK_METHOD(void, Lock, (), (override));
    MOCK_METHOD(void, Unlock, (), (override));
};

TEST(AccountTest, LockUnlockBehavior) {
    MockAccount acc(0, 1111);
    EXPECT_CALL(acc, Lock()).Times(2);
    EXPECT_CALL(acc, Unlock()).Times(1);
    acc.Lock();
    acc.Lock();
    acc.Unlock();
}

TEST(AccountTest, BalanceOperations) {
    Account acc(0, 1000);
    EXPECT_EQ(acc.GetBalance(), 1000);

    acc.Lock();
    EXPECT_NO_THROW(acc.ChangeBalance(100));
    EXPECT_EQ(acc.GetBalance(), 1100);
}

TEST(AccountTest, NegativeBalanceScenario) {
    MockAccount acc(1, 100);
    
    EXPECT_CALL(acc, Lock()).Times(1);
    
    EXPECT_CALL(acc, GetBalance())
        .WillOnce(Return(100))
        .WillOnce(Return(-50));
    
    EXPECT_CALL(acc, ChangeBalance(-150)).Times(1);
    
    acc.Lock();
    acc.ChangeBalance(-150); 
    
    EXPECT_EQ(acc.GetBalance(), -50);
}

TEST(TransactionTest, FeeManagement) {
    Transaction tr;
    EXPECT_EQ(tr.fee(), 1);

    tr.set_fee(50);
    EXPECT_EQ(tr.fee(), 50);
}

TEST(TransactionTest, SuccessfulTransaction) {
    MockAccount from(1, 1000);
    MockAccount to(2, 500);
    Transaction tr;
    tr.set_fee(10);
    
    EXPECT_CALL(from, Lock()).Times(1);
    EXPECT_CALL(to, Lock()).Times(1);
    EXPECT_CALL(from, Unlock()).Times(1);
    EXPECT_CALL(to, Unlock()).Times(1);
    
    EXPECT_CALL(from, GetBalance()).WillOnce(Return(1000));
    EXPECT_CALL(to, GetBalance()).WillOnce(Return(500));
    
    EXPECT_CALL(from, ChangeBalance(-210)).Times(1);
    EXPECT_CALL(to, ChangeBalance(200)).Times(1);
    
    EXPECT_TRUE(tr.Make(from, to, 200));
}

TEST(TransactionTest, InvalidTransactions) {
    Account acc1(1, 100);
    Account acc2(2, 200);
    Transaction tr;
    
    EXPECT_THROW(tr.Make(acc1, acc1, 50), std::logic_error);
    
    EXPECT_THROW(tr.Make(acc1, acc2, -50), std::invalid_argument);
    
    tr.set_fee(100);
    EXPECT_FALSE(tr.Make(acc1, acc2, 50));
}