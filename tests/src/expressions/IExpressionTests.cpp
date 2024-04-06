#include <gtest/gtest.h>

#include "fintamath/expressions/IExpression.hpp"

#include "fintamath/expressions/Expression.hpp"
#include "fintamath/expressions/ExpressionUtils.hpp"
#include "fintamath/functions/arithmetic/Add.hpp"
#include "fintamath/functions/other/Factorial.hpp"
#include "fintamath/functions/trigonometry/Sin.hpp"

using namespace fintamath;

namespace {

class TestExpression : public IExpressionCRTP<TestExpression> {
  FINTAMATH_CLASS_BODY(TestExpression, IExpression)

public:
  const std::shared_ptr<IFunction> &getFunction() const override {
    static const std::shared_ptr func = cast<IFunction>(Add().clone());
    return func;
  }

  const ArgumentPtrVector &getChildren() const override {
    static const ArgumentPtrVector children;
    return children;
  }

  void setChildren(const ArgumentPtrVector &childVect) override {
  }

protected:
  ArgumentPtr simplify() const override {
    if (auto res = preSimplify()) {
      return res;
    }

    if (auto res = postSimplify()) {
      return res;
    }

    return clone();
  }

  ArgumentPtr approximate() const override {
    return clone();
  }
};

[[maybe_unused]] const auto config = [] {
  IExpression::registerType<TestExpression>();
  return 0;
}();

}

TEST(IExpressionTests, parseTest) {
  EXPECT_TRUE(is<TestExpression>(*IExpression::parseFirst("TestExpression")));
}

TEST(IExpressionTests, getFunctionTest) {
  // TODO: implement
}

TEST(IExpressionTests, getFunctionToStringNotationTest) {
  // TODO: implement
}

TEST(IExpressionTests, getChildrenTest) {
  // TODO: implement
}

TEST(IExpressionTests, setChildrenTest) {
  const auto expr = cast<IExpression>(factorialExpr(Variable("a"))->clone());

  expr->setChildren({Variable("b").clone()});
  EXPECT_EQ(expr->toString(), "b!");

  EXPECT_THROW(expr->setChildren({Variable("b").clone(), Variable("c").clone()}), InvalidInputException);

  // TODO: implement more tests
}

TEST(IExpressionTests, getVariablesTest) {
  const auto expr = cast<IExpression>(Expression("x^2+y^2+a").clone());
  const auto vars = expr->getVariables();
  EXPECT_EQ(vars[0].toString(), "a");
  EXPECT_EQ(vars[1].toString(), "x");
  EXPECT_EQ(vars[2].toString(), "y");

  // TODO: implement more tests
}

TEST(IExpressionTests, setVariablesTest) {
  // TODO: implement
}

TEST(IExpressionTests, toMinimalObjectTest) {
  const TestExpression a;
  EXPECT_EQ(*a.toMinimalObject(), a);

  // TODO: implement more tests
}

TEST(IExpressionTests, getClassTest) {
  EXPECT_EQ(IExpression::getClassStatic()->getName(), "IExpression");
  EXPECT_EQ(IExpression::getClassStatic()->getParent(), IMathObject::getClassStatic());
}
