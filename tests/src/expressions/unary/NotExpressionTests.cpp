#include <gtest/gtest.h>

#include "fintamath/expressions/Expression.hpp"
#include "fintamath/expressions/ExpressionUtils.hpp"
#include "fintamath/functions/logic/Not.hpp"

using namespace fintamath;

TEST(NotExpressionTests, getTypeTest) {
  EXPECT_EQ(notExpr(Integer(0).clone())->getType(), MathObjectType::NotExpression);
}
