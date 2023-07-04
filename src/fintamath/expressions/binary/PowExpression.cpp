#include "fintamath/expressions/binary/PowExpression.hpp"

#include "fintamath/exceptions/UndefinedException.hpp"
#include "fintamath/expressions/ExpressionUtils.hpp"
#include "fintamath/functions/arithmetic/Add.hpp"
#include "fintamath/functions/arithmetic/Div.hpp"
#include "fintamath/functions/arithmetic/Mul.hpp"
#include "fintamath/functions/arithmetic/Neg.hpp"
#include "fintamath/functions/powers/Pow.hpp"
#include "fintamath/functions/powers/Root.hpp"
#include "fintamath/functions/powers/Sqrt.hpp"
#include "fintamath/numbers/Integer.hpp"
#include "fintamath/numbers/IntegerFunctions.hpp"
#include "fintamath/numbers/Rational.hpp"

namespace fintamath {

PowExpression::PowExpression(const ArgumentPtr &inLhsChild, const ArgumentPtr &inRhsChild)
    : IBinaryExpressionCRTP(Pow(), inLhsChild, inRhsChild) {
}

std::string PowExpression::toString() const {
  if (auto val = cast<Rational>(rhsChild)) {
    const Integer &numerator = val->numerator();
    const Integer &denominator = val->denominator();

    if (numerator == 1) {
      if (denominator == 2) {
        return functionToString(Sqrt(), {lhsChild});
      }

      return functionToString(Root(), {lhsChild, denominator.clone()});
    }
  }

  return IBinaryExpression::toString();
}

std::shared_ptr<IFunction> PowExpression::getOutputFunction() const {
  if (*rhsChild == Rational(1, 2)) {
    return std::make_shared<Sqrt>();
  }

  return IBinaryExpression::getFunction();
}

ArgumentPtr PowExpression::preciseSimplify() const {
  static const int64_t maxPreciseRoot = 9;

  if (const auto ratRhsChild = cast<Rational>(rhsChild); ratRhsChild && ratRhsChild->denominator() <= maxPreciseRoot) {
    auto preciseExpr = cast<PowExpression>(clone());
    preciseSimplifyChild(preciseExpr->lhsChild);
    return preciseExpr;
  }

  return IBinaryExpression::preciseSimplify();
}

PowExpression::SimplifyFunctionsVector PowExpression::getFunctionsForPreSimplify() const {
  static const PowExpression::SimplifyFunctionsVector simplifyFunctions = {
      &PowExpression::negSimplify, //
      &PowExpression::powSimplify, //
  };
  return simplifyFunctions;
}

PowExpression::SimplifyFunctionsVector PowExpression::getFunctionsForPostSimplify() const {
  static const PowExpression::SimplifyFunctionsVector simplifyFunctions = {
      &PowExpression::numSimplify,     //
      &PowExpression::polynomSimplify, //
  };
  return simplifyFunctions;
}

// Use bites representation for generate all partitions of numbers, using stars and bars method
// https://en.wikipedia.org/wiki/Stars_and_bars_(combinatorics)
// https://en.wikipedia.org/wiki/Partition_(number_theory)
// This method generate first bit number in sequence of numbers for each partition
Integer PowExpression::generateFirstNum(const Integer &countOfOne) {
  Integer n = 0;

  for (int i = 0; i < countOfOne; i++) {
    n = n << 1 | 1;
  }

  return n;
}

// Use alghorithm for generating next number in sequence of partitions
// https://en.wikipedia.org/wiki/Partition_(number_theory)
// https://en.wikipedia.org/wiki/Combinatorial_number_system#Applications
Integer PowExpression::generateNextNumber(Integer n) {
  Integer u = n & -n;
  Integer v = u + n;
  n = v + (((v ^ n) / u) >> 2);
  return n;
}

std::vector<Integer> PowExpression::getPartition(Integer bitNumber, const Integer &variableCount) {
  std::vector<Integer> result;
  Integer counter = 0;

  while (int64_t(result.size()) < variableCount) {
    if (bitNumber % 2 == 1) {
      counter++;
    }

    if (bitNumber % 2 == 0) {
      result.emplace_back(counter);
      counter = 0;
    }

    bitNumber >>= 1;
  }

  return result;
}

// Use multinomial theorem for exponentiation of sum
//  https://en.wikipedia.org/wiki/Multinomial_theorem
ArgumentPtr PowExpression::sumPolynomSimplify(const ArgumentPtr &expr, const Integer &powValue) {
  auto sumExpr = cast<IExpression>(expr);
  ArgumentsPtrVector polynom;

  if (sumExpr && is<Add>(sumExpr->getFunction())) {
    polynom = sumExpr->getChildren();
  }
  else {
    return {};
  }

  ArgumentsPtrVector newPolynom;
  Integer variableCount = int64_t(polynom.size());

  Integer bitNumber = generateFirstNum(powValue);

  for (int i = 0; i < combinations(powValue + variableCount - 1, powValue); i++) {
    std::vector<Integer> vectOfPows = getPartition(bitNumber, variableCount);
    bitNumber = generateNextNumber(bitNumber);

    ArgumentsPtrVector mulExprPolynom;
    mulExprPolynom.emplace_back(std::make_shared<Integer>(multinomialCoefficient(powValue, vectOfPows)));

    for (size_t j = 0; j < size_t(variableCount); j++) {
      ArgumentPtr powExpr = makeExpr(Pow(), polynom[j], std::make_shared<Integer>(std::move(vectOfPows[j])));
      mulExprPolynom.emplace_back(powExpr);
    }

    ArgumentPtr mulExpr = makeExpr(Mul(), mulExprPolynom);
    newPolynom.emplace_back(mulExpr);
  }

  return makeExpr(Add(), newPolynom)->toMinimalObject();
}

ArgumentPtr PowExpression::negSimplify(const IFunction & /*func*/, const ArgumentPtr &lhs, const ArgumentPtr &rhs) {
  if (auto lhsExpr = cast<IExpression>(lhs); lhsExpr && is<Neg>(lhsExpr->getFunction())) {
    ArgumentPtr lhsMul = makeExpr(Pow(), std::make_shared<Integer>(-1), rhs);
    ArgumentPtr rhsMul = makeExpr(Pow(), lhsExpr->getChildren().front(), rhs);
    return makeExpr(Mul(), lhsMul, rhsMul)->toMinimalObject();
  }

  return {};
}

ArgumentPtr PowExpression::powSimplify(const IFunction & /*func*/, const ArgumentPtr &lhs, const ArgumentPtr &rhs) {
  if (auto lhsExpr = cast<IExpression>(lhs); lhsExpr && is<Pow>(lhsExpr->getFunction())) {
    ArgumentPtr lhsPow = lhsExpr->getChildren().front();
    ArgumentPtr rhsPow = makeExpr(Mul(), lhsExpr->getChildren().back(), rhs);
    return makeExpr(Pow(), lhsPow, rhsPow)->toMinimalObject();
  }

  return {};
}

ArgumentPtr PowExpression::numSimplify(const IFunction & /*func*/, const ArgumentPtr &lhs, const ArgumentPtr &rhs) {
  auto lhsInt = cast<Integer>(lhs);
  auto rhsInt = cast<Integer>(rhs);

  if (rhsInt) {
    if (*rhsInt == 0) {
      return std::make_shared<Integer>(1);
    }

    if (*rhsInt == 1 || (lhsInt && *lhsInt == 1)) {
      return lhs;
    }

    if (*rhsInt == -1) {
      return makeExpr(Div(), std::make_shared<Integer>(1), lhs)->toMinimalObject();
    }

    if (*rhsInt < 0) {
      return makeExpr(Div(), std::make_shared<Integer>(1), makeExpr(Pow(), lhs, makeExpr(Neg(), rhsInt)))
          ->toMinimalObject();
    }
  }

  if (lhsInt && *lhsInt == 0) {
    return lhs;
  }

  return {};
}

ArgumentPtr PowExpression::polynomSimplify(const IFunction & /*func*/, const ArgumentPtr &lhs, const ArgumentPtr &rhs) {
  if (auto res = mulSimplify(lhs, rhs)) {
    return res;
  }

  if (auto res = sumSimplify(lhs, rhs)) {
    return res;
  }

  return {};
}

ArgumentPtr PowExpression::mulSimplify(const ArgumentPtr &lhs, const ArgumentPtr &rhs) {
  if (auto mulExpr = cast<IExpression>(lhs); mulExpr && is<Mul>(mulExpr->getFunction())) {
    ArgumentsPtrVector args = mulExpr->getChildren();

    for (auto &arg : args) {
      arg = makeExpr(Pow(), arg, rhs->clone());
    }

    return makeExpr(Mul(), args)->toMinimalObject();
  }

  return {};
}

ArgumentPtr PowExpression::sumSimplify(const ArgumentPtr &lhs, const ArgumentPtr &rhs) {
  if (const auto rhsInt = cast<Integer>(rhs)) {
    if (auto sumExpr = sumPolynomSimplify(lhs, *rhsInt)) {
      return sumExpr;
    }
  }

  return {};
}
}
