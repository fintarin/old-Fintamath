#include "fintamath/expressions/interfaces/IUnaryExpression.hpp"

#include "fintamath/core/IComparable.hpp"
#include "fintamath/expressions/ExpressionUtils.hpp"
#include "fintamath/literals/constants/Undefined.hpp"

namespace fintamath {

std::string IUnaryExpression::toString() const {
  if (const auto oper = cast<IOperator>(func)) {
    if (oper->getOperatorPriority() == IOperator::Priority::PostfixUnary) {
      return postfixUnaryOperatorToString(*oper, child);
    }

    return prefixUnaryOperatorToString(*oper, child);
  }

  return functionToString(*func, {child});
}

std::shared_ptr<IFunction> IUnaryExpression::getFunction() const {
  return func;
}

ArgumentPtrVector IUnaryExpression::getChildren() const {
  return {child};
}

IUnaryExpression::SimplifyFunctionVector IUnaryExpression::getFunctionsForPreSimplify() const {
  return {};
}

IUnaryExpression::SimplifyFunctionVector IUnaryExpression::getFunctionsForPostSimplify() const {
  return {};
}

ArgumentPtr IUnaryExpression::preSimplify() const {
  auto simpl = cast<IUnaryExpression>(clone());
  preSimplifyChild(simpl->child);

  if (auto res = simplifyUndefined(*simpl->func, simpl->child)) {
    return res;
  }

  ArgumentPtr res = simpl->useSimplifyFunctions(getFunctionsForPreSimplify());

  if (res && *res != *simpl) {
    preSimplifyChild(res);
    return res;
  }

  return simpl;
}

ArgumentPtr IUnaryExpression::postSimplify() const {
  auto simpl = cast<IUnaryExpression>(clone());
  postSimplifyChild(simpl->child);

  if (auto res = simplifyUndefined(*simpl->func, simpl->child)) {
    return res;
  }

  if (ArgumentPtr res = callFunction(*simpl->func, {simpl->child})) {
    return res;
  }

  ArgumentPtr res = simpl->useSimplifyFunctions(getFunctionsForPostSimplify());

  if (res && *res != *simpl) {
    postSimplifyChild(res);
    return res;
  }

  return simpl;
}

ArgumentPtr IUnaryExpression::preciseSimplify() const {
  auto preciseExpr = cast<IUnaryExpression>(clone());
  preciseSimplifyChild(preciseExpr->child);
  return preciseExpr;
}

ArgumentPtr IUnaryExpression::useSimplifyFunctions(const SimplifyFunctionVector &simplFuncs) const {
  for (const auto &simplFunc : simplFuncs) {
    if (auto res = simplFunc(*func, child)) {
      return res;
    }
  }

  return {};
}

void IUnaryExpression::setChildren(const ArgumentPtrVector &childVect) {
  if (childVect.size() != 1) {
    throw InvalidInputFunctionException(toString(), argumentVectorToStringVector(childVect));
  }

  child = childVect.front();
}

}