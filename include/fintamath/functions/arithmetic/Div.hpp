#pragma once

#include <memory>
#include <string>

#include "fintamath/core/IArithmetic.hpp"
#include "fintamath/core/IMathObject.hpp"
#include "fintamath/core/MathObjectClass.hpp"
#include "fintamath/functions/FunctionArguments.hpp"
#include "fintamath/functions/FunctionUtils.hpp"
#include "fintamath/functions/IOperator.hpp"

namespace fintamath {

class Integer;
class INumber;

class Div final : public IOperatorCRTP<IArithmetic, Div, IArithmetic, IArithmetic> {
  FINTAMATH_CLASS_BODY(Div)

public:
  std::string toString() const noexcept override {
    return "/";
  }

  static constexpr Priority getPriorityStatic() {
    return Priority::Multiplication;
  }

protected:
  std::unique_ptr<IMathObject> call(const ArgumentRefVector &argVect) const override;
};

FINTAMATH_FUNCTION_EXPRESSION(Div, divExpr);

}
