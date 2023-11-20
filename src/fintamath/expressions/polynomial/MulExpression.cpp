#include "fintamath/expressions/polynomial/MulExpression.hpp"

#include "fintamath/expressions/ExpressionUtils.hpp"
#include "fintamath/functions/arithmetic/Add.hpp"
#include "fintamath/functions/arithmetic/Div.hpp"
#include "fintamath/functions/arithmetic/Mul.hpp"
#include "fintamath/functions/arithmetic/Neg.hpp"
#include "fintamath/functions/powers/Pow.hpp"
#include "fintamath/literals/constants/ComplexInf.hpp"
#include "fintamath/literals/constants/IConstant.hpp"
#include "fintamath/literals/constants/Inf.hpp"
#include "fintamath/literals/constants/NegInf.hpp"
#include "fintamath/literals/constants/Undefined.hpp"
#include "fintamath/numbers/Integer.hpp"
#include "fintamath/numbers/IntegerFunctions.hpp"
#include "fintamath/numbers/Rational.hpp"

namespace fintamath {

MulExpression::MulExpression(ArgumentPtrVector inChildren)
    : IPolynomExpressionCRTP(Mul(), std::move(inChildren)) {
}

std::string MulExpression::toString() const {
  if (const auto firstChildRat = cast<Rational>(children.front())) {
    ArgumentPtrVector numeratorChildren = children;

    if (const Integer firstChildNumeratorAbs = abs(firstChildRat->numerator()); firstChildNumeratorAbs != 1) {
      numeratorChildren.front() = firstChildNumeratorAbs.clone();
    }
    else {
      numeratorChildren.erase(numeratorChildren.begin());
    }

    ArgumentPtr numerator = numeratorChildren.size() > 1 ? mulExpr(std::move(numeratorChildren)) : numeratorChildren.front();
    ArgumentPtr denominator = firstChildRat->denominator().clone();
    ArgumentPtr res = divExpr(numerator, denominator);

    std::string resStr = res->toString();
    if (firstChildRat->numerator() < Integer(0)) {
      resStr.insert(0, Neg().toString());
    }

    return resStr;
  }

  return IPolynomExpression::toString();
}

std::string MulExpression::childToString(const IOperator &oper, const ArgumentPtr &inChild, const ArgumentPtr &prevChild) const {
  if (!prevChild && *inChild == Integer(-1)) {
    return Neg().toString();
  }

  std::string operStr;
  if (prevChild && *prevChild != Integer(-1)) {
    if (is<INumber>(inChild) && is<INumber>(prevChild)) {
      operStr = oper.toString();
    }
    else {
      operStr = " ";
    }
  }

  return operStr + operatorChildToString(oper, inChild);
}

MulExpression::SimplifyFunctionVector MulExpression::getFunctionsForPreSimplify() const {
  static const MulExpression::SimplifyFunctionVector simplifyFunctions = {
      &MulExpression::constSimplify,
      &MulExpression::callFunctionSimplify,
      &MulExpression::rationalSimplify,
      &MulExpression::divSimplify,
      &MulExpression::powSimplify,
  };
  return simplifyFunctions;
}

MulExpression::SimplifyFunctionVector MulExpression::getFunctionsForPostSimplify() const {
  static const MulExpression::SimplifyFunctionVector simplifyFunctions = {
      &MulExpression::constSimplify,
      &MulExpression::polynomSimplify,
      &MulExpression::divSimplify,
      &MulExpression::powSimplify,
  };
  return simplifyFunctions;
}

bool MulExpression::isTermsOrderInversed() const {
  return true;
}

ArgumentPtr MulExpression::constSimplify(const IFunction & /*func*/, const ArgumentPtr &lhs, const ArgumentPtr &rhs) {
  if (*lhs == Integer(0) &&
      (is<Inf>(rhs) || is<NegInf>(rhs) || is<ComplexInf>(rhs))) {

    return Undefined().clone();
  }

  if (is<ComplexInf>(lhs) || is<ComplexInf>(rhs)) {
    return ComplexInf().clone();
  }

  if (is<NegInf>(lhs) && is<Inf>(rhs)) {
    return lhs;
  }

  if (const auto lhsNum = cast<INumber>(lhs)) {
    if (!lhsNum->isComplex()) {
      bool isNegated = *lhsNum < Integer(0);

      if (is<Inf>(rhs)) {
        return isNegated ? NegInf().clone() : rhs;
      }

      if (is<NegInf>(rhs)) {
        return isNegated ? Inf().clone() : rhs;
      }
    }
    else {
      if (is<NegInf>(rhs)) {
        return mulExpr(negExpr(lhsNum), Inf().clone());
      }
    }
  }

  if (*lhs == Integer(0)) {
    return lhs;
  }

  if (*lhs == Integer(1)) {
    return rhs;
  }

  return {};
}

ArgumentPtr MulExpression::rationalSimplify(const IFunction & /*func*/, const ArgumentPtr &lhs, const ArgumentPtr &rhs) {
  if (const auto lhsRat = cast<Rational>(lhs)) {
    ArgumentPtr numerator = mulExpr(lhsRat->numerator().clone(), rhs);
    ArgumentPtr denominator = lhsRat->denominator().clone();
    return divExpr(numerator, denominator);
  }

  return {};
}

ArgumentPtr MulExpression::callFunctionSimplify(const IFunction &func, const ArgumentPtr &lhs, const ArgumentPtr &rhs) {
  return callFunction(func, {lhs, rhs});
}

ArgumentPtr MulExpression::divSimplify(const IFunction & /*func*/, const ArgumentPtr &lhs, const ArgumentPtr &rhs) {
  const std::shared_ptr<const IExpression> lhsExpr = cast<IExpression>(lhs);
  const std::shared_ptr<const IExpression> rhsExpr = cast<IExpression>(rhs);

  bool isLhsDiv = false;
  bool isRhsDiv = false;

  if (lhsExpr && is<Div>(lhsExpr->getFunction())) {
    isLhsDiv = true;
  }

  if (rhsExpr && is<Div>(rhsExpr->getFunction())) {
    isRhsDiv = true;
  }

  ArgumentPtr numerator;
  ArgumentPtr denominator;

  if (isLhsDiv && isRhsDiv) {
    numerator = mulExpr(lhsExpr->getChildren().front(), rhsExpr->getChildren().front());
    denominator = mulExpr(lhsExpr->getChildren().back(), rhsExpr->getChildren().back());
  }
  else if (isLhsDiv) {
    numerator = mulExpr(lhsExpr->getChildren().front(), rhs);
    denominator = lhsExpr->getChildren().back();
  }
  else if (isRhsDiv) {
    numerator = mulExpr(lhs, rhsExpr->getChildren().front());
    denominator = rhsExpr->getChildren().back();
  }
  else {
    return {};
  }

  ArgumentPtr res = divExpr(numerator, denominator);
  return res;
}

ArgumentPtr MulExpression::polynomSimplify(const IFunction & /*func*/, const ArgumentPtr &lhs, const ArgumentPtr &rhs) {
  std::shared_ptr<const IExpression> lhsExpr = cast<IExpression>(lhs);
  std::shared_ptr<const IExpression> rhsExpr = cast<IExpression>(rhs);

  if (lhsExpr &&
      rhsExpr &&
      !is<Add>(lhsExpr->getFunction()) &&
      !is<Add>(rhsExpr->getFunction())) {

    return {};
  }

  ArgumentPtrVector lhsChildren;
  ArgumentPtrVector rhsChildren;

  if (lhsExpr && is<Add>(lhsExpr->getFunction())) {
    lhsChildren = lhsExpr->getChildren();
  }
  else {
    lhsChildren.emplace_back(lhs);
  }

  if (rhsExpr && is<Add>(rhsExpr->getFunction())) {
    rhsChildren = rhsExpr->getChildren();
  }
  else {
    if (!containsVariable(lhs) && containsVariable(rhs)) {
      return {};
    }

    rhsChildren.emplace_back(rhs);
  }

  if (lhsChildren.size() == 1 && rhsChildren.size() == 1) {
    return {};
  }

  ArgumentPtrVector resultVect;

  for (const auto &lhsSubChild : lhsChildren) {
    for (const auto &rhsSubChild : rhsChildren) {
      resultVect.emplace_back(mulExpr(lhsSubChild, rhsSubChild));
    }
  }

  ArgumentPtr res = addExpr(std::move(resultVect));
  return res;
}

ArgumentPtr MulExpression::powSimplify(const IFunction & /*func*/, const ArgumentPtr &lhs, const ArgumentPtr &rhs) {
  auto [lhsChildRate, lhsChildValue] = splitPowExpr(lhs);
  auto [rhsChildRate, rhsChildValue] = splitPowExpr(rhs);

  if (*lhsChildValue == *rhsChildValue) {
    ArgumentPtr ratesSum = addExpr(lhsChildRate, rhsChildRate);
    return powExpr(lhsChildValue, ratesSum);
  }

  if (is<INumber>(lhsChildValue) &&
      is<INumber>(rhsChildValue) &&
      *lhsChildRate == *rhsChildRate &&
      *rhsChildRate != Integer(1)) {

    ArgumentPtr valuesMul = mulExpr(lhsChildValue, rhsChildValue);
    return powExpr(valuesMul, lhsChildRate);
  }

  return {};
}

}
