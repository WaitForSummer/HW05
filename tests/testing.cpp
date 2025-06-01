#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Account.h"
#include "Transaction.h"

using ::testing::NiveMock;
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

TEST(AccountTest, Locker) {
    NiceMock<MockAccount> acc(0, 1000);
    
    EXPECT_CALL(acc, Lock()).Times(2);
    EXPECT_CALL(acc, Unlock()).Times(1);
    
    acc.Lock();
    acc.Lock();
    acc.Unlock();
}

TEST(Account, balance_positive) {
    Account acc(1, 1000);
    acc.Lock();
    
    EXPECT_NO_THROW(acc.ChangeBalance(500));
    EXPECT_EQ(acc.GetBalance(), 1500);
    
    EXPECT_NO_THROW(acc.ChangeBalance(-200));
    EXPECT_EQ(acc.GetBalance(), 1300);
}

TEST(Accout, balance_negative) {
    NiceMock<MockAccount> mockAcc(2, 100);
    
    EXPECT_CALL(mockAcc, Lock());
    
    EXPECT_CALL(mockAcc, GetBalance())
        .WillOnce(Return(100))
        .WillOnce(Return(-50));
    
    EXPECT_CALL(mockAcc, ChangeBalance(-150))
        .Times(1);
    
    mockAcc.Lock();
    mockAcc.ChangeBalance(-150);

    EXPECT_EQ(mockAcc.GetBalance(), -50); 
}

TEST(Transaction, fee_mngr) {
    Transaction tr;
    EXPECT_EQ(tr.fee(), 1);
    
    tr.set_fee(50);
    EXPECT_EQ(tr.fee(), 50);
}

TEST(TransactionTest, successful_transctn) {

    NiceMock<MockAccount> from(1, 1000);
    NiceMock<MockAccount> to(2, 500);
    Transaction tr;
    tr.set_fee(10);
    
    EXPECT_CALL(from, Lock()).Times(1);
    EXPECT_CALL(to, Lock()).Times(1);
    EXPECT_CALL(from, Unlock()).Times(1);
    EXPECT_CALL(to, Unlock()).Times(1);
    
    EXPECT_CALL(from, GetBalance())
        .WillOnce(Return(1000));
    EXPECT_CALL(to, GetBalance())
        .WillOnce(Return(500));
    
    EXPECT_CALL(from, ChangeBalance(-210)).Times(1);
    EXPECT_CALL(to, ChangeBalance(200)).Times(1);
    
    EXPECT_TRUE(tr.Make(from, to, 200));
}

TEST(TransactionTest, negative_baalnce_after_trans) {
    NiceMock<MockAccount> from(3, 300);
    NiceMock<MockAccount> to(4, 100);
    Transaction tr;
    tr.set_fee(10);
    
    EXPECT_CALL(from, GetBalance())
        .WillOnce(Return(300))
        .WillOnce(Return(-10));
    
    EXPECT_CALL(to, GetBalance())
        .WillOnce(Return(100));
    
    EXPECT_CALL(from, ChangeBalance(-310)).Times(1);
    EXPECT_CALL(to, ChangeBalance(300)).Times(1);
    
    EXPECT_TRUE(tr.Make(from, to, 300));
    EXPECT_EQ(from.GetBalance(), -10);
}

TEST(TransactionTest, invalid_transaction) {
    Account acc1(5, 100);
    Account acc2(6, 200);
    Transaction tr;
    
    EXPECT_THROW(tr.Make(acc1, acc1, 50), std::logic_error);
    
    EXPECT_THROW(tr.Make(acc1, acc2, -50), std::invalid_argument);
    
    tr.set_fee(100);
    EXPECT_FALSE(tr.Make(acc1, acc2, 50));
}

