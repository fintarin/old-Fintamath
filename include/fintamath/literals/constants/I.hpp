#pragma once

#include <memory>
#include <string>

#include "fintamath/core/IMathObject.hpp"
#include "fintamath/core/MathObjectClass.hpp"
#include "fintamath/literals/constants/IConstant.hpp"
#include "fintamath/numbers/Complex.hpp"

namespace fintamath {

class I final : public IConstantCRTP<Complex, I> {
  FINTAMATH_CLASS_BODY(I)

public:
  std::string toString() const noexcept override {
    return "I";
  }

protected:
  std::unique_ptr<IMathObject> call() const override;
};

}
