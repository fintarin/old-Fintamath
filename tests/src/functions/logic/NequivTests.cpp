#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "fintamath/functions/logic/Nequiv.hpp"

#include "fintamath/literals/Boolean.hpp"
#include "fintamath/literals/Variable.hpp"

using namespace fintamath;

const Nequiv f;

TEST(NequivTests, toStringTest) {
  EXPECT_EQ(f.toString(), "!<->");
}

TEST(NequivTests, getArgumentClassesTest) {
  EXPECT_THAT(f.getArgumentClasses(), testing::ElementsAre(Boolean::getClassStatic(), Boolean::getClassStatic()));
}

TEST(NequivTests, getReturnClassTest) {
  EXPECT_EQ(f.getReturnClass(), Boolean::getClassStatic());
}

TEST(NequivTests, isVariadicTest) {
  EXPECT_FALSE(f.isVariadic());
}

TEST(NequivTests, isEvaluatableTest) {
  EXPECT_TRUE(f.isEvaluatable());
}

TEST(NequivTests, getPriorityTest) {
  EXPECT_EQ(f.getPriority(), IOperator::Priority::Equivalence);
}

TEST(NequivTests, isAssociativeTest) {
  EXPECT_TRUE(f.isAssociative());
}

TEST(NequivTests, callTest) {
  EXPECT_EQ(f(Boolean(false), Boolean(false))->toString(), "False");
  EXPECT_EQ(f(Boolean(false), Boolean(true))->toString(), "True");
  EXPECT_EQ(f(Boolean(true), Boolean(false))->toString(), "True");
  EXPECT_EQ(f(Boolean(true), Boolean(true))->toString(), "False");

  EXPECT_EQ(f(Variable("a"), Variable("b"))->toString(), "(a & ~b) | (~a & b)");

  EXPECT_THROW(f(), InvalidInputFunctionException);
  EXPECT_THROW(f(Boolean(true)), InvalidInputFunctionException);
  EXPECT_THROW(f(Boolean(true), Boolean(true), Boolean(true)), InvalidInputFunctionException);
}

TEST(NequivTests, exprTest) {
  EXPECT_EQ(nequivExpr(Boolean(true), Boolean(false))->toString(), "(~True & False) | (True & ~False)");
}

TEST(NequivTests, getClassTest) {
  EXPECT_EQ(f.getClass(), MathObjectClass("Nequiv"));
  EXPECT_EQ(f.getClass().getParent(), IOperator::getClassStatic());
}
