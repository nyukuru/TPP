#include <gtest/gtest.h>
#include <tpp/user.h>

TEST(User, DefaultConstructedFieldsAreEmpty) {
  tpp::user user;
  EXPECT_EQ(user.id, "");
  EXPECT_EQ(user.login, "");
  EXPECT_EQ(user.name, "");
}

TEST(User, EqualityComparesAllFields) {
  tpp::user a {"123", "someuser", "SomeUser"};
  tpp::user b {"123", "someuser", "SomeUser"};
  EXPECT_EQ(a, b);
}

TEST(User, InequalityDetectsAnyDifferingField) {
  tpp::user base {"123", "someuser", "SomeUser"};
  EXPECT_NE(base, (tpp::user {"124", "someuser", "SomeUser"}));
  EXPECT_NE(base, (tpp::user {"123", "otheruser", "SomeUser"}));
  EXPECT_NE(base, (tpp::user {"123", "someuser", "OtherName"}));
}
