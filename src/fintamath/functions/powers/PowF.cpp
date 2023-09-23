#include "fintamath/functions/powers/PowF.hpp"

#include "fintamath/functions/powers/Pow.hpp"

namespace fintamath {

std::unique_ptr<IMathObject> PowF::call(const ArgumentRefVector &argsVect) const {
  return Pow()(argsVect);
}

}
