#include "fintamath/literals/constants/NegInf.hpp"

#include <memory>

#include "fintamath/core/IMathObject.hpp"

namespace fintamath {

std::unique_ptr<IMathObject> NegInf::call() const {
  return clone();
}

}
