#pragma once

#include "fintamath/core/IComparable.hpp"
#include "fintamath/functions/IOperator.hpp"
#include "fintamath/literals/Boolean.hpp"

namespace fintamath {

class Neqv final : public IOperatorCRTP<Boolean, Neqv, IComparable, IComparable> {
public:
  Neqv() : IOperatorCRTP(IOperator::Priority::Comparison, true) {
  }

  std::string toString() const override {
    return "!=";
  }

  static MathObjectType getTypeStatic() {
    return MathObjectType::Neqv;
  }

protected:
  std::unique_ptr<IMathObject> call(const ArgumentRefVector &argVect) const override;
};

FINTAMATH_FUNCTION_EXPRESSION(Neqv, neqvExpr);

}
