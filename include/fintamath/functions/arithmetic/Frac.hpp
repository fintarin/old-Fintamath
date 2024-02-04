#pragma once

#include <memory>
#include <string>

#include "fintamath/core/IArithmetic.hpp"
#include "fintamath/core/IMathObject.hpp"
#include "fintamath/core/MathObjectTypes.hpp"
#include "fintamath/functions/FunctionArguments.hpp"
#include "fintamath/functions/IFunction.hpp"

namespace fintamath {

class Frac final : public IFunctionCRTP<IArithmetic, Frac, IArithmetic, IArithmetic> {
public:
  std::string toString() const override {
    return "frac";
  }

  static MathObjectType getTypeStatic() {
    return MathObjectType::Frac;
  }

protected:
  std::unique_ptr<IMathObject> call(const ArgumentRefVector &argVect) const override;
};

}
