#pragma once

#include "fintamath/literals/constants/IConstant.hpp"

namespace fintamath {
  class Pi : public IConstantCRTP<Pi> {
  public:
    std::string toString() const override;

  protected:
    Expression getValue() const override;
  };
}
