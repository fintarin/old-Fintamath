#include "gtest/gtest.h"

#include "fintamath/functions/comparison/MoreEqv.hpp"

#include "fintamath/literals/Variable.hpp"
#include "fintamath/numbers/Rational.hpp"

using namespace fintamath;

const MoreEqv f;

TEST(MoreEqvTests, toStringTest) {
  EXPECT_EQ(f.toString(), ">=");
}

TEST(MoreEqvTests, getFunctionTypeTest) {
  EXPECT_EQ(f.getFunctionType(), IFunction::Type::Binary);
}

TEST(MoreEqvTests, getOperatorPriorityTest) {
  EXPECT_EQ(f.getOperatorPriority(), IOperator::Priority::Comparison);
}

TEST(MoreEqvTests, callTest) {
  EXPECT_EQ(f(Integer(5), Integer(3))->toString(), "true");
  EXPECT_EQ(f(Integer(3), Integer(5))->toString(), "false");
  EXPECT_EQ(f(Integer(3), Integer(3))->toString(), "true");
  EXPECT_EQ(f(Integer(3), Rational(3, 1))->toString(), "true");
  EXPECT_EQ(f(Rational(5, 2), Integer(2))->toString(), "true");

  EXPECT_EQ(f(Integer(3), Variable("a"))->toString(), "-a+3>=0");
  EXPECT_EQ(f(Variable("a"), Variable("a"))->toString(), "true");

  EXPECT_THROW(f(Integer(1)), InvalidInputFunctionException);
  EXPECT_THROW(f(Rational(2, 3)), InvalidInputFunctionException);
  EXPECT_THROW(f(), InvalidInputFunctionException);
  EXPECT_THROW(f(Integer(1), Integer(1), Integer(1)), InvalidInputFunctionException);
}
