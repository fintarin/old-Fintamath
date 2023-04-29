#include "fintamath/expressions/binary/PowExpression.hpp"

#include "fintamath/exceptions/UndefinedBinaryOperatorException.hpp"
#include "fintamath/expressions/ExpressionUtils.hpp"
#include "fintamath/functions/arithmetic/Add.hpp"
#include "fintamath/functions/arithmetic/Div.hpp"
#include "fintamath/functions/arithmetic/Mul.hpp"
#include "fintamath/functions/arithmetic/Neg.hpp"
#include "fintamath/functions/powers/Pow.hpp"
#include "fintamath/numbers/Integer.hpp"
#include "fintamath/numbers/IntegerFunctions.hpp"
#include "fintamath/numbers/NumberConstants.hpp"

namespace fintamath {

const Pow POW;

PowExpression::PowExpression(const ArgumentPtr &inLhsChild, const ArgumentPtr &inRhsChild)
    : IBinaryExpressionCRTP(POW, inLhsChild, inRhsChild) {
}

ArgumentPtr PowExpression::mulSimplify() const {
  auto powExpr = cast<PowExpression>(clone());

  if (auto mulExpr = cast<IExpression>(powExpr->lhsChild); mulExpr && is<Mul>(mulExpr->getFunction())) {
    ArgumentsPtrVector args = mulExpr->getChildren();

    for (auto &arg : args) {
      arg = makeRawFunctionExpression(Pow(), {arg, powExpr->rhsChild->clone()});
    }

    return makeFunctionExpression(Mul(), args);
  }

  return {};
}

ArgumentPtr PowExpression::sumSimplify() const {
  auto powExpr = cast<PowExpression>(clone());

  if (const auto rhs = cast<Integer>(powExpr->rhsChild)) {
    if (auto sumExpr = sumPolynomSimplify(powExpr->lhsChild, *rhs)) {
      return sumExpr;
    }
  }

  return {};
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
ArgumentPtr PowExpression::sumPolynomSimplify(const ArgumentPtr &expr, Integer powValue) {
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
      auto powExpr =
          makeRawFunctionExpression(Pow(), {polynom[j], std::make_shared<Integer>(std::move(vectOfPows[j]))});
      mulExprPolynom.emplace_back(powExpr);
    }

    ArgumentPtr mulExpr = makeRawFunctionExpression(Mul(), mulExprPolynom);
    newPolynom.emplace_back(mulExpr);
  }

  ArgumentPtr newSumExpr = makeFunctionExpression(Add(), newPolynom);

  return newSumExpr;
}

ArgumentPtr PowExpression::polynomSimplify() const {
  if (auto res = mulSimplify()) {
    return res;
  }

  if (auto res = sumSimplify()) {
    return res;
  }

  return {};
}

ArgumentPtr PowExpression::invert() const {
  auto inv = std::make_shared<PowExpression>(*this);
  inv->rhsChild = makeFunctionExpression(Neg(), {inv->rhsChild});
  return inv;
}

ArgumentPtr PowExpression::preSimplify() const {
  auto simpl = IBinaryExpression::preSimplify();
  auto simplExpr = cast<PowExpression>(simpl);

  if (!simplExpr) {
    return simpl;
  }

  if (auto lhsExpr = cast<IExpression>(simplExpr->lhsChild); lhsExpr && is<Neg>(lhsExpr->getFunction())) {
    auto lhsMul = makeRawFunctionExpression(Pow(), {NEG_ONE.clone(), simplExpr->rhsChild});
    auto rhsMul = makeRawFunctionExpression(Pow(), {lhsExpr->getChildren()[0], simplExpr->rhsChild});
    return makeFunctionExpression(Mul(), {lhsMul, rhsMul});
  }

  if (auto lhsExpr = cast<IExpression>(simplExpr->lhsChild); lhsExpr && is<Pow>(lhsExpr->getFunction())) {
    auto lhsPow = lhsExpr->getChildren().front();
    auto rhsPow = makeRawFunctionExpression(Mul(), {lhsExpr->getChildren()[1], simplExpr->rhsChild});
    return makeFunctionExpression(Pow(), {lhsPow, rhsPow});
  }

  return simpl;
}

ArgumentPtr PowExpression::postSimplify() const {
  auto simpl = IBinaryExpression::postSimplify();
  auto simplExpr = cast<PowExpression>(simpl);

  if (!simplExpr) {
    return simpl;
  }

  auto lhsInt = cast<Integer>(simplExpr->lhsChild);
  auto rhsInt = cast<Integer>(simplExpr->rhsChild);

  if (rhsInt) {
    if (*rhsInt == ZERO) {
      if (lhsInt && *lhsInt == ZERO) {
        throw UndefinedBinaryOperatorException(POW.toString(), simplExpr->lhsChild->toString(),
                                               simplExpr->rhsChild->toString());
      }

      return ONE.clone();
    }

    if (*rhsInt == ONE || (lhsInt && *lhsInt == ONE)) {
      return simplExpr->lhsChild;
    }

    if (*rhsInt == NEG_ONE) {
      return makeFunctionExpression(Div(), {ONE.clone(), simplExpr->lhsChild});
    }

    if (*rhsInt < ZERO) {
      return makeFunctionExpression(
          Div(),
          {ONE.clone(), makeFunctionExpression(Pow(), {simplExpr->lhsChild, makeFunctionExpression(Neg(), {rhsInt})})});
    }
  }

  if (lhsInt && *lhsInt == ZERO) {
    return simplExpr->lhsChild;
  }

  return simplExpr->polynomSimplify();
}

}
