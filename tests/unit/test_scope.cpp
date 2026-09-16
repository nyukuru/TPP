#include <gtest/gtest.h>
#include <tpp/scope.h>

TEST(Scope, DefaultConstructedHasNoScopes) {
  tpp::scope scopes;
  EXPECT_EQ(scopes.to_string(), "");
  EXPECT_FALSE(scopes.has(tpp::scope::s_chat_read));
}

TEST(Scope, ConstructFromSingleScope) {
  tpp::scope scopes(tpp::scope::s_chat_read);
  EXPECT_TRUE(scopes.has(tpp::scope::s_chat_read));
  EXPECT_FALSE(scopes.has(tpp::scope::s_chat_edit));
  EXPECT_EQ(scopes.to_string(), "chat:read");
}

TEST(Scope, ConstructFromMultipleScopesAtOnce) {
  tpp::scope scopes(tpp::scope::s_chat_read, tpp::scope::s_chat_edit);
  EXPECT_TRUE(scopes.has(tpp::scope::s_chat_read));
  EXPECT_TRUE(scopes.has(tpp::scope::s_chat_edit));
  EXPECT_FALSE(scopes.has(tpp::scope::s_openid));
}

TEST(Scope, BitwiseOrCombinesTwoScopeValues) {
  /* std::bitset has no built-in operator| against a bare position value,
   * so tpp::scope overloads `|` itself (see tpp/scope.h) to restore the
   * DPP-style `s_a | s_b` combination syntax. */
  tpp::scope scopes = tpp::scope::s_chat_read | tpp::scope::s_chat_edit;
  EXPECT_TRUE(scopes.has(tpp::scope::s_chat_read));
  EXPECT_TRUE(scopes.has(tpp::scope::s_chat_edit));
  EXPECT_FALSE(scopes.has(tpp::scope::s_openid));
}

TEST(Scope, BitwiseOrChainsAcrossMoreThanTwoValues) {
  tpp::scope scopes = tpp::scope::s_chat_read | tpp::scope::s_chat_edit | tpp::scope::s_openid;
  EXPECT_TRUE(scopes.has(tpp::scope::s_chat_read));
  EXPECT_TRUE(scopes.has(tpp::scope::s_chat_edit));
  EXPECT_TRUE(scopes.has(tpp::scope::s_openid));
}

TEST(Scope, BitwiseOrCombinesTwoScopeObjects) {
  tpp::scope a(tpp::scope::s_chat_read);
  tpp::scope b(tpp::scope::s_chat_edit);
  tpp::scope combined = a | b;
  EXPECT_TRUE(combined.has(tpp::scope::s_chat_read));
  EXPECT_TRUE(combined.has(tpp::scope::s_chat_edit));
  /* a and b are untouched - operator| returns a new tpp::scope. */
  EXPECT_FALSE(a.has(tpp::scope::s_chat_edit));
  EXPECT_FALSE(b.has(tpp::scope::s_chat_read));
}

TEST(Scope, BitwiseOrAssignAddsAScopeInPlace) {
  tpp::scope scopes(tpp::scope::s_chat_read);
  scopes |= tpp::scope::s_chat_edit;
  EXPECT_TRUE(scopes.has(tpp::scope::s_chat_read));
  EXPECT_TRUE(scopes.has(tpp::scope::s_chat_edit));
}

TEST(Scope, BitwiseOrAssignAddsAnotherScopeObjectInPlace) {
  tpp::scope scopes(tpp::scope::s_chat_read);
  tpp::scope other(tpp::scope::s_chat_edit, tpp::scope::s_openid);
  scopes |= other;
  EXPECT_TRUE(scopes.has(tpp::scope::s_chat_read));
  EXPECT_TRUE(scopes.has(tpp::scope::s_chat_edit));
  EXPECT_TRUE(scopes.has(tpp::scope::s_openid));
}

TEST(Scope, BitwiseAndIntersectsTwoScopeObjects) {
  tpp::scope a(tpp::scope::s_chat_read, tpp::scope::s_chat_edit);
  tpp::scope b(tpp::scope::s_chat_edit, tpp::scope::s_openid);
  tpp::scope intersection = a & b;
  EXPECT_FALSE(intersection.has(tpp::scope::s_chat_read));
  EXPECT_TRUE(intersection.has(tpp::scope::s_chat_edit));
  EXPECT_FALSE(intersection.has(tpp::scope::s_openid));
}

TEST(Scope, BitwiseAndWithBareScopeValueKeepsOnlyThatScope) {
  tpp::scope scopes(tpp::scope::s_chat_read, tpp::scope::s_chat_edit);
  tpp::scope result = scopes & tpp::scope::s_chat_edit;
  EXPECT_FALSE(result.has(tpp::scope::s_chat_read));
  EXPECT_TRUE(result.has(tpp::scope::s_chat_edit));
}

TEST(Scope, BitwiseAndAssignNarrowsInPlace) {
  tpp::scope scopes(tpp::scope::s_chat_read, tpp::scope::s_chat_edit);
  tpp::scope filter(tpp::scope::s_chat_edit, tpp::scope::s_openid);
  scopes &= filter;
  EXPECT_FALSE(scopes.has(tpp::scope::s_chat_read));
  EXPECT_TRUE(scopes.has(tpp::scope::s_chat_edit));
  EXPECT_FALSE(scopes.has(tpp::scope::s_openid));
}

TEST(Scope, AddAndRemove) {
  tpp::scope scopes;
  scopes.add(tpp::scope::s_chat_read);
  EXPECT_TRUE(scopes.has(tpp::scope::s_chat_read));
  scopes.add(tpp::scope::s_chat_edit);
  EXPECT_TRUE(scopes.has(tpp::scope::s_chat_edit));
  scopes.remove(tpp::scope::s_chat_read);
  EXPECT_FALSE(scopes.has(tpp::scope::s_chat_read));
  EXPECT_TRUE(scopes.has(tpp::scope::s_chat_edit));
}

TEST(Scope, AddAcceptsMultipleScopesAtOnce) {
  tpp::scope scopes;
  scopes.add(tpp::scope::s_chat_read, tpp::scope::s_chat_edit, tpp::scope::s_openid);
  EXPECT_TRUE(scopes.has(tpp::scope::s_chat_read));
  EXPECT_TRUE(scopes.has(tpp::scope::s_chat_edit));
  EXPECT_TRUE(scopes.has(tpp::scope::s_openid));
}

TEST(Scope, SetReplacesRatherThanAccumulates) {
  tpp::scope scopes;
  scopes.add(tpp::scope::s_chat_read);
  scopes.set(tpp::scope::s_chat_edit);
  EXPECT_FALSE(scopes.has(tpp::scope::s_chat_read));
  EXPECT_TRUE(scopes.has(tpp::scope::s_chat_edit));
}

TEST(Scope, HasRequiresAllGivenBitsSet) {
  tpp::scope scopes(tpp::scope::s_chat_read);
  /* Only chat_read is set, so requiring both must fail. */
  EXPECT_FALSE(scopes.has(tpp::scope::s_chat_read, tpp::scope::s_chat_edit));
  scopes.add(tpp::scope::s_chat_edit);
  EXPECT_TRUE(scopes.has(tpp::scope::s_chat_read, tpp::scope::s_chat_edit));
}

TEST(Scope, HasAnyRequiresOnlyOneGivenBitSet) {
  tpp::scope scopes(tpp::scope::s_chat_read);
  EXPECT_TRUE(scopes.has_any(tpp::scope::s_chat_read, tpp::scope::s_chat_edit));
  EXPECT_FALSE(scopes.has_any(tpp::scope::s_chat_edit, tpp::scope::s_openid));
}

TEST(Scope, RemoveAcceptsMultipleScopesAtOnce) {
  tpp::scope scopes(tpp::scope::s_chat_read, tpp::scope::s_chat_edit, tpp::scope::s_openid);
  scopes.remove(tpp::scope::s_chat_read, tpp::scope::s_chat_edit);
  EXPECT_FALSE(scopes.has(tpp::scope::s_chat_read));
  EXPECT_FALSE(scopes.has(tpp::scope::s_chat_edit));
  EXPECT_TRUE(scopes.has(tpp::scope::s_openid));
}

TEST(Scope, BitsBeyond64AreDistinctFromLowBits) {
  tpp::scope scopes(tpp::scope::s_user_bot, tpp::scope::s_openid);
  EXPECT_TRUE(scopes.has(tpp::scope::s_user_bot));
  EXPECT_TRUE(scopes.has(tpp::scope::s_openid));
  EXPECT_FALSE(scopes.has(tpp::scope::s_chat_read));
  EXPECT_FALSE(scopes.has(tpp::scope::s_whispers_read));
}

TEST(Scope, HighestBitScopeRoundTripsThroughToString) {
  tpp::scope scopes(tpp::scope::s_whispers_read, tpp::scope::s_openid);
  EXPECT_EQ(scopes.to_string(), "openid whispers:read");
}

TEST(Scope, ToStringIsSpaceDelimitedAndStable) {
  tpp::scope scopes(tpp::scope::s_chat_read, tpp::scope::s_chat_edit);
  std::string first = scopes.to_string();
  std::string second = scopes.to_string();
  EXPECT_EQ(first, second);
  /* Both scope names must appear, space separated, in some fixed order. */
  EXPECT_NE(first.find("chat:read"), std::string::npos);
  EXPECT_NE(first.find("chat:edit"), std::string::npos);
  EXPECT_EQ(first.find(' '), first.rfind(' '));
}

TEST(Scope, OpenidScopeSerializesToLiteralOpenid) {
  tpp::scope scopes(tpp::scope::s_openid);
  EXPECT_EQ(scopes.to_string(), "openid");
}

TEST(Scope, ManyScopesAllAppearInOutput) {
  tpp::scope scopes(tpp::scope::s_chat_read, tpp::scope::s_chat_edit, tpp::scope::s_openid, tpp::scope::s_user_read_email,
                    tpp::scope::s_moderator_read_followers);
  std::string out = scopes.to_string();
  EXPECT_NE(out.find("chat:read"), std::string::npos);
  EXPECT_NE(out.find("chat:edit"), std::string::npos);
  EXPECT_NE(out.find("openid"), std::string::npos);
  EXPECT_NE(out.find("user:read:email"), std::string::npos);
  EXPECT_NE(out.find("moderator:read:followers"), std::string::npos);
}
