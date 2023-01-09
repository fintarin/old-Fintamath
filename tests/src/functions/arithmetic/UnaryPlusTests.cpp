#include "gtest/gtest.h"

#include "fintamath/functions/arithmetic/UnaryPlus.hpp"

#include "fintamath/literals/Variable.hpp"
#include "fintamath/numbers/Rational.hpp"

using namespace fintamath;

const UnaryPlus f;

TEST(UnaryPlusTests, toStringTest) {
  EXPECT_EQ(f.toString(), "+");
}

TEST(UnaryPlusTests, getFunctionTypeTest) {
  EXPECT_EQ(f.getFunctionType(), IFunction::Type::Unary);
}

TEST(UnaryPlusTests, getOperatorPriorityTest) {
  EXPECT_EQ(f.getOperatorPriority(), IOperator::Priority::PrefixUnary);
}

TEST(UnaryPlusTests, callTest) {
  EXPECT_EQ(f(Integer(3))->toString(), "3");
  EXPECT_EQ(f(Rational(5, 2))->toString(), "5/2");
  EXPECT_EQ(f(Rational(-5, 2))->toString(), "-5/2");

  EXPECT_EQ(f(Variable("a"))->toString(), "a");

  EXPECT_THROW(f(Integer(1), Rational(2, 3)), InvalidInputFunctionException);
  EXPECT_THROW(f(), InvalidInputFunctionException);
  EXPECT_THROW(f(Integer(1), Integer(1), Integer(1)), InvalidInputFunctionException);
}
