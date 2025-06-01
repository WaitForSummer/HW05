#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Account.h"
#include "Transaction.h"

using ::testing::Return;
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

    ON_CALL(acc, Lock()).WillByDefault([&acc]() { acc.Account::Lock(); });
    ON_CALL(acc, Unlock()).WillByDefault([&acc]() { acc.Account::Unlock(); });
    ON_CALL(acc, ChangeBalance(::testing::_)).WillByDefault([&acc](int diff) { acc.Account::ChangeBalance(diff); });

    EXPECT_CALL(acc, Lock()).Times(2);
    EXPECT_CALL(acc, Unlock()).Times(1);

    acc.Lock();
    EXPECT_NO_THROW(acc.ChangeBalance(100));

    EXPECT_THROW(acc.Lock(), std::runtime_error);

    acc.Unlock();

    EXPECT_THROW(acc.ChangeBalance(100), std::runtime_error);
}

TEST(AccountTest, BalanceOperations) {
    Account acc(0, 1000);
    acc.Lock();
    acc.ChangeBalance(100);
    EXPECT_EQ(acc.GetBalance(), 1100);
}

TEST(AccountTest, NegativeBalanceScenario) {
    MockAccount acc(1, 100);
    
    EXPECT_CALL(acc, Lock()).Times(1);
    EXPECT_CALL(acc, ChangeBalance(-150)).Times(1);
    
    acc.Lock();
    acc.ChangeBalance(-150);
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
    
    EXPECT_CALL(from, GetBalance())
        .WillOnce(Return(1000))
        .WillOnce(Return(790));
    EXPECT_CALL(to, GetBalance())
        .WillOnce(Return(500));
    
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
    EXPECT_THROW(tr.Make(acc1, acc2, 50), std::logic_error);
}

TEST(AccountTest, UnlockDirectly) {
    Account acc(42, 1000);
    acc.Lock();
    acc.Unlock();
    EXPECT_THROW(acc.ChangeBalance(0), std::runtime_error);
}

TEST(TransactionTest, NotEnoughMoneyToDebit) {
    Account from(1, 100);
    Account to(2, 300);
    Transaction tr;
    tr.set_fee(50);
    EXPECT_FALSE(tr.Make(from, to, 100));
}

TEST(AccountTest, DebCallsGBlncNgtvSum) {
    MockAccount acc(1, 1000);
    Transaction tr;
    int sum = 500;

    ON_CALL(acc, Lock()).WillByDefault([&acc]() { acc.Account::Lock(); });
    ON_CALL(acc, Unlock()).WillByDefault([&acc]() { acc.Account::Unlock(); });
    ON_CALL(acc, ChangeBalance(::testing::_)).WillByDefault([&acc](int diff) { acc.Account::ChangeBalance(diff); });
    
    acc.Lock();

    EXPECT_CALL(acc, ChangeBalance(-sum)).Times(1);

    EXPECT_CALL(acc, GetBalance()).WillOnce(Return(-sum));

    acc.Lock();
    bool result = tr.Debit(acc, sum);
    acc.Unlock();

    EXPECT_TRUE(result);
    
    acc.ChangeBalance(-sum);

    int balance = acc.GetBalance();
    EXPECT_EQ(balance, -sum);

    acc.Unlock();
}