#include "fintamath/expressions/binary/DivExpression.hpp"

#include "fintamath/exceptions/UndefinedException.hpp"
#include "fintamath/expressions/ExpressionUtils.hpp"
#include "fintamath/functions/arithmetic/Add.hpp"
#include "fintamath/functions/arithmetic/Div.hpp"
#include "fintamath/functions/arithmetic/Mul.hpp"
#include "fintamath/functions/arithmetic/Neg.hpp"
#include "fintamath/functions/arithmetic/Sub.hpp"
#include "fintamath/functions/powers/Pow.hpp"
#include "fintamath/numbers/IntegerFunctions.hpp"
#include "fintamath/numbers/Rational.hpp"

namespace fintamath {

DivExpression::DivExpression(const ArgumentPtr &inLhsChild, const ArgumentPtr &inRhsChild)
    : IBinaryExpressionCRTP(Div(), inLhsChild, inRhsChild) {
}

std::string DivExpression::toString() const {
  if (auto lhsChildExpr = cast<IExpression>(lhsChild); lhsChildExpr && is<Neg>(lhsChildExpr->getFunction())) {
    ArgumentPtr innerDiv = makeExpr(Div(), lhsChildExpr->getChildren().front(), rhsChild);
    return makeExpr(Neg(), innerDiv)->toString();
  }

  return IBinaryExpression::toString();
}

DivExpression::SimplifyFunctionsVector DivExpression::getFunctionsForPreSimplify() const {
  static const DivExpression::SimplifyFunctionsVector simplifyFunctions = {
      &DivExpression::zeroSimplify, //
      &DivExpression::divSimplify,  //
      &DivExpression::mulSimplify,  //
  };
  return simplifyFunctions;
}

DivExpression::SimplifyFunctionsVector DivExpression::getFunctionsForPostSimplify() const {
  static const DivExpression::SimplifyFunctionsVector simplifyFunctions = {
      &DivExpression::zeroSimplify,            //
      &DivExpression::negSimplify,             //
      &DivExpression::numSimplify,             //
      &DivExpression::divSimplify,             //
      &DivExpression::mulSimplify,             //
      &DivExpression::nestedRationalsSimplify, //
      &DivExpression::gcdSimplify,             //
      &DivExpression::sumSimplify,             //
  };
  return simplifyFunctions;
}

ArgumentPtr DivExpression::zeroSimplify(const IFunction & /*func*/, const ArgumentPtr &lhs, const ArgumentPtr &rhs) {
  if (*rhs == Integer(0)) {
    throw UndefinedBinaryOperatorException(Div().toString(), lhs->toString(), rhs->toString());
  }

  return {};
}

ArgumentPtr DivExpression::numSimplify(const IFunction & /*func*/, const ArgumentPtr &lhs, const ArgumentPtr &rhs) {
  if (*lhs == Integer(0)) {
    return lhs;
  }

  static const Integer one = 1;

  if (Div().doArgsMatch({one, *rhs})) {
    ArgumentPtr res = makeExpr(Mul(), lhs, Div()(one, *rhs));
    return res;
  }

  return {};
}

ArgumentPtr DivExpression::divSimplify(const IFunction & /*func*/, const ArgumentPtr &lhs, const ArgumentPtr &rhs) {
  ArgumentsPtrVector numeratorChildren;
  ArgumentsPtrVector denominatorChildren;

  bool containsDivExpression = false;

  if (auto lhsDivExpr = cast<DivExpression>(lhs)) {
    numeratorChildren.emplace_back(lhsDivExpr->getChildren().front());
    denominatorChildren.emplace_back(lhsDivExpr->getChildren().back());
    containsDivExpression = true;
  }
  else {
    numeratorChildren.emplace_back(lhs);
  }

  if (auto rhsDivExpr = cast<DivExpression>(rhs)) {
    denominatorChildren.emplace_back(rhsDivExpr->getChildren().front());
    numeratorChildren.emplace_back(rhsDivExpr->getChildren().back());
    containsDivExpression = true;
  }
  else {
    denominatorChildren.emplace_back(rhs);
  }

  if (!containsDivExpression) {
    return {};
  }

  ArgumentPtr numerator;
  if (numeratorChildren.size() == 1) {
    numerator = numeratorChildren.front();
  }
  else {
    numerator = makeExpr(Mul(), numeratorChildren);
  }

  ArgumentPtr denominator;
  if (denominatorChildren.size() == 1) {
    denominator = denominatorChildren.front();
  }
  else {
    denominator = makeExpr(Mul(), denominatorChildren);
  }

  ArgumentPtr res = makeExpr(Div(), numerator, denominator);
  return res;
}

ArgumentPtr DivExpression::mulSimplify(const IFunction & /*func*/, const ArgumentPtr &lhs, const ArgumentPtr &rhs) {
  ArgumentsPtrVector lhsChildren;
  if (const auto lhsExpr = cast<IExpression>(lhs); lhsExpr && is<Mul>(lhsExpr->getFunction())) {
    lhsChildren = lhsExpr->getChildren();
  }
  else {
    lhsChildren = {lhs};
  }

  ArgumentsPtrVector rhsChildren;
  if (const auto rhsExpr = cast<IExpression>(rhs); rhsExpr && is<Mul>(rhsExpr->getFunction())) {
    rhsChildren = rhsExpr->getChildren();
  }
  else {
    rhsChildren = {rhs};
  }

  size_t lhsChildrenSizeInitial = lhsChildren.size();
  size_t rhsChildrenSizeInitial = rhsChildren.size();

  for (auto &lhsChild : lhsChildren) {
    for (size_t j = 0; j < rhsChildren.size(); j++) {
      bool isResFound = false;

      if (auto divPowRes = powSimplify(lhsChild, rhsChildren[j])) {
        lhsChild = divPowRes;
        rhsChildren.erase(rhsChildren.begin() + ArgumentsPtrVector::difference_type(j));
        isResFound = true;
      }
      else if (auto callFuncRes = callFunction(Div(), {lhsChild, rhsChildren[j]});
               callFuncRes && !is<Rational>(callFuncRes)) {
        lhsChild = Div()(*lhsChild, *rhsChildren[j]);
        rhsChildren.erase(rhsChildren.begin() + ArgumentsPtrVector::difference_type(j));
        isResFound = true;
      }

      if (isResFound) {
        break;
      }
    }

    if (rhsChildren.empty()) {
      break;
    }
  }

  ArgumentPtr numerator;
  if (lhsChildren.size() > 1) {
    numerator = makeExpr(Mul(), lhsChildren);
  }
  else {
    numerator = lhsChildren.front();
  }

  if (rhsChildren.empty()) {
    return numerator;
  }

  ArgumentPtr denominator;
  if (rhsChildren.size() > 1) {
    denominator = makeExpr(Mul(), rhsChildren);
  }
  else {
    denominator = rhsChildren.front();
  }

  if (lhsChildren.size() != lhsChildrenSizeInitial || rhsChildren.size() != rhsChildrenSizeInitial) {
    ArgumentPtr res = makeExpr(Div(), numerator, denominator);
    return res;
  }

  return {};
}

ArgumentPtr DivExpression::negSimplify(const IFunction & /*func*/, const ArgumentPtr &lhs, const ArgumentPtr &rhs) {
  if (isNeg(rhs)) {
    ArgumentPtr res = makeExpr(Div(), makeExpr(Neg(), lhs), makeExpr(Neg(), rhs));
    return res;
  }

  return {};
}

bool DivExpression::isNeg(const ArgumentPtr &expr) {
  ArgumentPtr checkValue;
  if (auto exprChild = cast<IExpression>(expr); exprChild && is<Add>(exprChild->getFunction())) {
    checkValue = exprChild->getChildren().front();
  }
  else {
    checkValue = expr;
  }

  if (auto exprValue = cast<IExpression>(checkValue); exprValue && is<Neg>(exprValue->getFunction())) {
    return true;
  }

  return false;
}

ArgumentPtr DivExpression::sumSimplify(const IFunction & /*func*/, const ArgumentPtr &lhs, const ArgumentPtr &rhs) {
  if (auto [result, remainder] = mulSumSimplify(lhs, rhs); result) {
    return remainder ? makeExpr(Add(), result, remainder) : result;
  }

  if (auto [result, remainder] = sumMulSimplify(lhs, rhs); result) {
    return remainder ? makeExpr(Add(), result, remainder) : result;
  }

  if (auto [result, remainder] = sumSumSimplify(lhs, rhs); result) {
    return remainder ? makeExpr(Add(), result, remainder) : result;
  }

  if (auto [result, remainder] = sumSumSimplify(rhs, lhs); result && !is<IExpression>(remainder)) {
    ArgumentPtr denominator = remainder ? makeExpr(Add(), result, remainder) : result;
    return makeExpr(Div(), Integer(1).clone(), denominator);
  }

  return {};
}

std::pair<ArgumentPtr, ArgumentPtr> DivExpression::sumSumSimplify(const ArgumentPtr &lhs, const ArgumentPtr &rhs) {
  ArgumentsPtrVector lhsChildren;
  ArgumentsPtrVector rhsChildren;

  if (const auto lhsExpr = cast<IExpression>(lhs); lhsExpr && is<Add>(lhsExpr->getFunction())) {
    lhsChildren = lhsExpr->getChildren();
  }

  if (const auto rhsExpr = cast<IExpression>(rhs); rhsExpr && is<Add>(rhsExpr->getFunction())) {
    rhsChildren = rhsExpr->getChildren();
  }

  if (lhsChildren.empty() || rhsChildren.empty()) {
    return {};
  }

  ArgumentsPtrVector resultVect;
  ArgumentsPtrVector remainderVect;

  for (const auto &child : lhsChildren) {
    auto [result, remainder] = mulSumSimplify(child, rhs);

    if (result) {
      resultVect.emplace_back(result);

      if (remainder) {
        auto remainderDiv = cast<DivExpression>(remainder);
        remainderVect.emplace_back(remainderDiv->getChildren().front());
      }
    }
    else {
      remainderVect.emplace_back(child);
    }
  }

  if (resultVect.empty()) {
    return {};
  }

  ArgumentPtr result = makeExpr(Add(), resultVect);

  ArgumentPtr remainderAdd = makeExpr(Add(), remainderVect);
  ArgumentPtr remainder = makeExpr(Div(), remainderAdd, rhs);
  simplifyChild(remainder);

  return {result, remainder};
}

std::pair<ArgumentPtr, ArgumentPtr> DivExpression::sumMulSimplify(const ArgumentPtr &lhs, const ArgumentPtr &rhs) {
  ArgumentsPtrVector lhsChildren;

  if (const auto lhsExpr = cast<IExpression>(lhs); lhsExpr && is<Add>(lhsExpr->getFunction())) {
    lhsChildren = lhsExpr->getChildren();
  }

  if (const auto rhsChildExpr = cast<IExpression>(rhs);
      (rhsChildExpr && is<Add>(rhsChildExpr->getFunction())) || lhsChildren.empty()) {
    return {};
  }

  ArgumentsPtrVector resultChildren;
  ArgumentsPtrVector remainderChildren;

  for (const auto &child : lhsChildren) {
    ArgumentPtr divResult = makeExpr(Div(), child, rhs);
    simplifyChild(divResult);

    if (const auto divResultExpr = cast<IExpression>(divResult);
        divResultExpr && is<Div>(divResultExpr->getFunction()) && *divResultExpr->getChildren().back() == *rhs) {
      remainderChildren.emplace_back(child);
    }
    else {
      resultChildren.emplace_back(divResult);
    }
  }

  if (remainderChildren.size() == lhsChildren.size()) {
    return {};
  }

  ArgumentPtr result = makeExpr(Add(), resultChildren);

  ArgumentPtr remainder;
  if (!remainderChildren.empty()) {
    ArgumentPtr remainderAdd = makeExpr(Add(), remainderChildren);
    remainder = makeExpr(Div(), remainderAdd, rhs);
    simplifyChild(remainder);
  }

  return {result, remainder};
}

std::pair<ArgumentPtr, ArgumentPtr> DivExpression::mulSumSimplify(const ArgumentPtr &lhs, const ArgumentPtr &rhs) {
  ArgumentsPtrVector rhsChildren;

  if (const auto rhsExpr = cast<IExpression>(rhs); rhsExpr && is<Add>(rhsExpr->getFunction())) {
    rhsChildren = rhsExpr->getChildren();
  }

  if (const auto lhsChildExpr = cast<IExpression>(lhs);
      (lhsChildExpr && is<Add>(lhsChildExpr->getFunction())) || rhsChildren.empty()) {
    return {};
  }

  ArgumentPtr result = makeExpr(Div(), lhs, rhsChildren.front());
  simplifyChild(result);

  if (const auto divExpr = cast<IExpression>(result); divExpr && is<Div>(divExpr->getFunction())) {
    return {};
  }

  ArgumentsPtrVector multiplicator;

  for (size_t i = 1; i < rhsChildren.size(); i++) {
    multiplicator.emplace_back(makeExpr(Mul(), rhsChildren[i], result));
  }

  ArgumentPtr remainderNegAdd = makeExpr(Neg(), makeExpr(Add(), multiplicator));
  simplifyChild(remainderNegAdd);
  ArgumentPtr remainder = makeExpr(Div(), remainderNegAdd, rhs);

  return {result, remainder};
}

ArgumentPtr DivExpression::powSimplify(const ArgumentPtr &lhs, const ArgumentPtr &rhs) {
  if (*lhs == *rhs) {
    return std::make_shared<Integer>(1);
  }

  bool isResultNegated = false;

  ArgumentPtr lhsChild;
  ArgumentPtr rhsChild;

  if (const auto lhsExpr = cast<IExpression>(lhs); lhsExpr && is<Neg>(lhsExpr->getFunction())) {
    isResultNegated = !isResultNegated;
    lhsChild = lhsExpr->getChildren().front();
  }
  else {
    lhsChild = lhs;
  }

  if (const auto rhsExpr = cast<IExpression>(rhs); rhsExpr && is<Neg>(rhsExpr->getFunction())) {
    isResultNegated = !isResultNegated;
    rhsChild = rhsExpr->getChildren().front();
  }
  else {
    rhsChild = rhs;
  }

  auto [lhsChildRate, lhsChildValue] = getRateValuePair(lhsChild);
  auto [rhsChildRate, rhsChildValue] = getRateValuePair(rhsChild);

  ArgumentPtr result;

  if (*lhsChildValue == *rhsChildValue) {
    result = addRatesToValue({lhsChildRate, makeExpr(Neg(), rhsChildRate)}, lhsChildValue);

    if (isResultNegated) {
      result = makeExpr(Neg(), result);
    }
  }

  if (*lhsChildRate == *rhsChildRate && *rhsChildRate != Integer(1) && is<INumber>(lhsChildValue) &&
      is<INumber>(rhsChildValue)) {

    ArgumentPtr valuesDiv = makeExpr(Div(), lhsChildValue, rhsChildValue);
    return makeExpr(Pow(), valuesDiv, lhsChildRate);
  }

  return result;
}

std::pair<ArgumentPtr, ArgumentPtr> DivExpression::getRateValuePair(const ArgumentPtr &rhs) {
  if (const auto &powExpr = cast<IExpression>(rhs); powExpr && is<Pow>(powExpr->getFunction())) {
    ArgumentsPtrVector powExprChildren = powExpr->getChildren();
    return {powExprChildren[1], powExprChildren[0]};
  }

  return {std::make_shared<Integer>(1), rhs};
}

ArgumentPtr DivExpression::addRatesToValue(const ArgumentsPtrVector &rates, const ArgumentPtr &value) {
  ArgumentPtr ratesSum = makeExpr(Add(), rates);
  return makeExpr(Pow(), value, ratesSum);
}

ArgumentPtr DivExpression::nestedRationalsSimplify(const IFunction & /*func*/, const ArgumentPtr &lhs,
                                                   const ArgumentPtr &rhs) {

  if (const auto &lhsExpr = cast<IExpression>(lhs)) {
    if (is<Mul>(lhsExpr->getFunction())) {
      if (auto res = nestedRationalsInNumeratorSimplify(lhsExpr->getChildren(), rhs)) {
        return res;
      }
    }
  }

  return {};
}

ArgumentPtr DivExpression::gcdSimplify(const IFunction & /*func*/, const ArgumentPtr &lhs, const ArgumentPtr &rhs) {
  ArgumentsPtrVector lhsChildren;
  ArgumentsPtrVector rhsChildren;

  if (const auto lhsExpr = cast<IExpression>(lhs); lhsExpr && is<Add>(lhsExpr->getFunction())) {
    lhsChildren = lhsExpr->getChildren();
  }
  else {
    lhsChildren = {lhs};
  }

  if (const auto rhsExpr = cast<IExpression>(rhs); rhsExpr && is<Add>(rhsExpr->getFunction())) {
    rhsChildren = rhsExpr->getChildren();
  }
  else {
    rhsChildren = {rhs};
  }

  Integer lhsGcdNum = getGcd(lhsChildren);
  Integer rhsGcdNum = getGcd(rhsChildren);

  if (lhsGcdNum <= 1 || rhsGcdNum <= 1) {
    return {};
  }

  Integer gcdNum = gcd(lhsGcdNum, rhsGcdNum);

  if (gcdNum <= 1) {
    return {};
  }

  ArgumentPtr numerator = makeExpr(Div(), lhs, gcdNum.clone());
  simplifyChild(numerator);

  ArgumentPtr denominator = makeExpr(Div(), rhs, gcdNum.clone());
  simplifyChild(denominator);

  return makeExpr(Div(), numerator, denominator);
}

Integer DivExpression::getGcd(ArgumentsPtrVector &lhsChildren) {
  Integer gcdNum;

  for (auto child : lhsChildren) {
    if (const auto childExpr = cast<IExpression>(child); childExpr && is<Mul>(childExpr->getFunction())) {
      child = childExpr->getChildren().front();
    }

    if (const auto childInt = cast<Integer>(child)) {
      Integer childIntAbs = abs(*childInt);
      gcdNum = gcdNum != 0 ? gcd(gcdNum, childIntAbs) : childIntAbs;
    }
    else {
      return 1;
    }
  }

  return gcdNum;
}

ArgumentPtr DivExpression::nestedRationalsInNumeratorSimplify(const ArgumentsPtrVector &lhsChildren,
                                                              const ArgumentPtr &rhs) {

  ArgumentsPtrVector numeratorChildren;
  ArgumentsPtrVector denominatorChildren;

  for (const auto &child : lhsChildren) {
    if (const auto &rationalChild = cast<Rational>(child)) {
      numeratorChildren.emplace_back(rationalChild->numerator().clone());
      denominatorChildren.emplace_back(rationalChild->denominator().clone());
      continue;
    }

    numeratorChildren.emplace_back(child);
  }

  if (!denominatorChildren.empty()) {
    denominatorChildren.emplace_back(rhs);

    ArgumentPtr numerator = makeExpr(Mul(), numeratorChildren);
    ArgumentPtr denominator = makeExpr(Mul(), denominatorChildren);
    return makeExpr(Div(), numerator, denominator);
  }

  return {};
}

}
