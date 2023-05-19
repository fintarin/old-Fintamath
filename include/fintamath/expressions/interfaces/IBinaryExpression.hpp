#pragma once

#include "fintamath/expressions/IExpression.hpp"

namespace fintamath {

class IBinaryExpression : public IExpression {
public:
  std::string toString() const override;

  std::shared_ptr<IFunction> getFunction() const final;

  ArgumentsPtrVector getChildren() const final;

  void setChildren(const ArgumentsPtrVector &childVect) final;

  static MathObjectTypeId getTypeIdStatic() {
    return MathObjectTypeId(MathObjectType::IBinaryExpression);
  }

protected:
  using SimplifyFunction = std::function<ArgumentPtr(const IFunction &, const ArgumentPtr &, const ArgumentPtr &)>;

  using SimplifyFunctionsVector = std::vector<SimplifyFunction>;

  virtual SimplifyFunctionsVector getFunctionsForSimplify() const;

  virtual SimplifyFunctionsVector getFunctionsForPreSimplify() const;

  virtual SimplifyFunctionsVector getFunctionsForPostSimplify() const;

  ArgumentPtr preSimplify() const override;

  ArgumentPtr postSimplify() const override;

  ArgumentPtr simplify() const final;

private:
  ArgumentPtr useSimplifyFunctions(const SimplifyFunctionsVector &simplFuncs) const;

protected:
  std::shared_ptr<IFunction> func;

  ArgumentPtr lhsChild;

  ArgumentPtr rhsChild;
};

template <typename Derived, bool isMultiFunction = false>
class IBinaryExpressionBaseCRTP : public IBinaryExpression {
#define FINTAMATH_I_EXPRESSION_BASE_CRTP IBinaryExpressionBaseCRTP<Derived, isMultiFunction>
#include "fintamath/expressions/IExpressionBaseCRTP.hpp"
#undef FINTAMATH_I_EXPRESSION_BASE_CRTP
};

template <typename Derived, bool isMultiFunction = false>
class IBinaryExpressionCRTP : public IBinaryExpressionBaseCRTP<Derived, isMultiFunction> {
#define FINTAMATH_I_EXPRESSION_CRTP IBinaryExpressionCRTP<Derived, isMultiFunction>
#include "fintamath/expressions/IExpressionCRTP.hpp"
#undef FINTAMATH_I_EXPRESSION_CRTP

public:
  explicit IBinaryExpressionCRTP(const IFunction &inFunc, const ArgumentPtr &inLhsChild,
                                 const ArgumentPtr &inRhsChild) {
    this->func = cast<IFunction>(inFunc.clone());

    this->lhsChild = inLhsChild;
    this->compressChild(this->lhsChild);

    this->rhsChild = inRhsChild;
    this->compressChild(this->rhsChild);
  }
};

}