#include <gtest/gtest.h>
#include <tpp/intents.h>

TEST(Intent, DefaultConstructedHasNoIntents) {
  tpp::intent intents;
  EXPECT_FALSE(intents.has(tpp::i_chat_messages));
}

TEST(Intent, ConstructFromSingleIntent) {
  tpp::intent intents(tpp::i_chat_messages);
  EXPECT_TRUE(intents.has(tpp::i_chat_messages));
  EXPECT_FALSE(intents.has(tpp::i_follows));
}

TEST(Intent, ConstructFromMultipleIntentsAtOnce) {
  tpp::intent intents(tpp::i_chat_messages, tpp::i_follows);
  EXPECT_TRUE(intents.has(tpp::i_chat_messages));
  EXPECT_TRUE(intents.has(tpp::i_follows));
  EXPECT_FALSE(intents.has(tpp::i_raids));
}

TEST(Intent, BitwiseOrCombinesTwoIntentValues) {
  tpp::intent intents = tpp::i_chat_messages | tpp::i_follows;
  EXPECT_TRUE(intents.has(tpp::i_chat_messages));
  EXPECT_TRUE(intents.has(tpp::i_follows));
  EXPECT_FALSE(intents.has(tpp::i_raids));
}

TEST(Intent, BitwiseOrChainsAcrossMoreThanTwoValues) {
  tpp::intent intents = tpp::i_chat_messages | tpp::i_channel_points | tpp::i_raids;
  EXPECT_TRUE(intents.has(tpp::i_chat_messages));
  EXPECT_TRUE(intents.has(tpp::i_channel_points));
  EXPECT_TRUE(intents.has(tpp::i_raids));
}

TEST(Intent, BitwiseOrCombinesTwoIntentObjects) {
  tpp::intent a(tpp::i_chat_messages);
  tpp::intent b(tpp::i_follows);
  tpp::intent combined = a | b;
  EXPECT_TRUE(combined.has(tpp::i_chat_messages));
  EXPECT_TRUE(combined.has(tpp::i_follows));
  EXPECT_FALSE(a.has(tpp::i_follows));
  EXPECT_FALSE(b.has(tpp::i_chat_messages));
}

TEST(Intent, BitwiseOrAssignAddsAnIntentInPlace) {
  tpp::intent intents(tpp::i_chat_messages);
  intents |= tpp::i_follows;
  EXPECT_TRUE(intents.has(tpp::i_chat_messages));
  EXPECT_TRUE(intents.has(tpp::i_follows));
}

TEST(Intent, BitwiseAndIntersectsTwoIntentObjects) {
  tpp::intent a(tpp::i_chat_messages, tpp::i_follows);
  tpp::intent b(tpp::i_follows, tpp::i_raids);
  tpp::intent intersection = a & b;
  EXPECT_FALSE(intersection.has(tpp::i_chat_messages));
  EXPECT_TRUE(intersection.has(tpp::i_follows));
  EXPECT_FALSE(intersection.has(tpp::i_raids));
}

TEST(Intent, BitwiseAndAssignNarrowsInPlace) {
  tpp::intent intents(tpp::i_chat_messages, tpp::i_follows);
  tpp::intent filter(tpp::i_follows, tpp::i_raids);
  intents &= filter;
  EXPECT_FALSE(intents.has(tpp::i_chat_messages));
  EXPECT_TRUE(intents.has(tpp::i_follows));
  EXPECT_FALSE(intents.has(tpp::i_raids));
}

TEST(Intent, AddAndRemove) {
  tpp::intent intents;
  intents.add(tpp::i_chat_messages);
  EXPECT_TRUE(intents.has(tpp::i_chat_messages));
  intents.add(tpp::i_follows);
  EXPECT_TRUE(intents.has(tpp::i_follows));
  intents.remove(tpp::i_chat_messages);
  EXPECT_FALSE(intents.has(tpp::i_chat_messages));
  EXPECT_TRUE(intents.has(tpp::i_follows));
}

TEST(Intent, SetReplacesRatherThanAccumulates) {
  tpp::intent intents;
  intents.add(tpp::i_chat_messages);
  intents.set(tpp::i_follows);
  EXPECT_FALSE(intents.has(tpp::i_chat_messages));
  EXPECT_TRUE(intents.has(tpp::i_follows));
}

TEST(Intent, HasRequiresAllGivenBitsSet) {
  tpp::intent intents(tpp::i_chat_messages);
  EXPECT_FALSE(intents.has(tpp::i_chat_messages, tpp::i_follows));
  intents.add(tpp::i_follows);
  EXPECT_TRUE(intents.has(tpp::i_chat_messages, tpp::i_follows));
}

TEST(Intent, HasAnyRequiresOnlyOneGivenBitSet) {
  tpp::intent intents(tpp::i_chat_messages);
  EXPECT_TRUE(intents.has_any(tpp::i_chat_messages, tpp::i_follows));
  EXPECT_FALSE(intents.has_any(tpp::i_follows, tpp::i_raids));
}
