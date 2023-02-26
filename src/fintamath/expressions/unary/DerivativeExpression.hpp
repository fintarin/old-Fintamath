#pragma once

#include <cstdint>

#include "fintamath/expressions/IUnaryExpression.hpp"

namespace fintamath {

class DerivativeExpression : public IUnaryExpressionCRTP<DerivativeExpression> {
public:
  explicit DerivativeExpression(const IMathObject &obj);

  explicit DerivativeExpression(MathObjectPtr &&obj);

  const IFunction *getFunction() const override;

  MathObjectPtr toMinimalObject() const override;

  MathObjectPtr simplify(bool isPrecise) const override;

  void compress() override;
};

}