#pragma once

#include "fintamath/functions/IOperator.hpp"
#include "fintamath/numbers/Integer.hpp"

namespace fintamath {
  class DoubleFactorial : public IOperatorCRTP<DoubleFactorial, INumber> {
  public:
    DoubleFactorial();

    std::string toString() const override;

  protected:
    Expression call(const std::vector<std::reference_wrapper<const IMathObject>> &argsVect) const override;
  };
}
