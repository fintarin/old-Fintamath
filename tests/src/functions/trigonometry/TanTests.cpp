#include "gtest/gtest.h"

#include "fintamath/functions/trigonometry/Tan.hpp"

#include "fintamath/exceptions/UndefinedFunctionException.hpp"
#include "fintamath/literals/Variable.hpp"
#include "fintamath/numbers/NumberConstants.hpp"
#include "fintamath/numbers/Rational.hpp"
#include "fintamath/numbers/Real.hpp"
#include "fintamath/numbers/RealFunctions.hpp"

using namespace fintamath;

const Tan f;

TEST(TanTests, toStringTest) {
  EXPECT_EQ(f.toString(), "tan");
}

TEST(TanTests, getFunctionTypeTest) {
  EXPECT_EQ(f.getFunctionType(), IFunction::Type::Unary);
}

TEST(TanTests, callTest) {
  EXPECT_EQ(f(Integer(0))->toString(), "0");
  EXPECT_EQ(f(Integer(10))->toString(),
            "0.6483608274590866712591249330098086768168743429837249756336279673958556003746239");
  EXPECT_EQ(f(Integer(-10))->toString(),
            "-0.6483608274590866712591249330098086768168743429837249756336279673958556003746239");
  EXPECT_EQ(f(Integer(5))->toString(),
            "-3.3805150062465856369827058794473439087095692082854598771768209809183460102012061");
  EXPECT_EQ(f(Rational(1, 10))->toString(),
            "0.10033467208545054505808004578111153681900480457644204002220806579803211288567387");
  EXPECT_EQ(f(PI_NUM / 4)->toString(), "1");

  EXPECT_EQ(f(Variable("a"))->toString(), "tan(a)");

  EXPECT_THROW(f(PI_NUM / 2), UndefinedFunctionException);

  EXPECT_THROW(f(), InvalidInputFunctionException);
  EXPECT_THROW(f(Integer(1), Integer(1), Integer(1)), InvalidInputFunctionException);
}
