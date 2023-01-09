#pragma once

#include "fintamath/core/CoreUtils.hpp"
#include "fintamath/core/IMathObject.hpp"
#include "fintamath/exceptions/InvalidInputBinaryOpearatorException.hpp"

namespace fintamath {

class IArithmetic;
using ArithmeticPtr = std::unique_ptr<IArithmetic>;

class IArithmetic : virtual public IMathObject {
public:
  friend ArithmeticPtr operator+(const IArithmetic &lhs, const IArithmetic &rhs);

  friend ArithmeticPtr operator-(const IArithmetic &lhs, const IArithmetic &rhs);

  friend ArithmeticPtr operator*(const IArithmetic &lhs, const IArithmetic &rhs);

  friend ArithmeticPtr operator/(const IArithmetic &lhs, const IArithmetic &rhs);

  friend ArithmeticPtr operator+(const IArithmetic &rhs);

  friend ArithmeticPtr operator-(const IArithmetic &rhs);

  template <typename Lhs, typename Rhs, typename Func>
  static void addMultiAddFunction(const Func &func) {
    multiAdd.add<Lhs, Rhs>(func);
  }

  template <typename Lhs, typename Rhs, typename Func>
  static void addMultiSubFunction(const Func &func) {
    multiSub.add<Lhs, Rhs>(func);
  }

  template <typename Lhs, typename Rhs, typename Func>
  static void addMultiMulFunction(const Func &func) {
    multiMul.add<Lhs, Rhs>(func);
  }

  template <typename Lhs, typename Rhs, typename Func>
  static void addMultiDivFunction(const Func &func) {
    multiDiv.add<Lhs, Rhs>(func);
  }

protected:
  virtual ArithmeticPtr addAbstract(const IArithmetic &rhs) const = 0;

  virtual ArithmeticPtr substractAbstract(const IArithmetic &rhs) const = 0;

  virtual ArithmeticPtr multiplyAbstract(const IArithmetic &rhs) const = 0;

  virtual ArithmeticPtr divideAbstract(const IArithmetic &rhs) const = 0;

  virtual ArithmeticPtr convertAbstract() const = 0;

  virtual ArithmeticPtr negateAbstract() const = 0;

  static MultiMethod<ArithmeticPtr(const IArithmetic &, const IArithmetic &)> multiAdd;

  static MultiMethod<ArithmeticPtr(const IArithmetic &, const IArithmetic &)> multiSub;

  static MultiMethod<ArithmeticPtr(const IArithmetic &, const IArithmetic &)> multiMul;

  static MultiMethod<ArithmeticPtr(const IArithmetic &, const IArithmetic &)> multiDiv;
};

inline ArithmeticPtr operator+(const IArithmetic &lhs, const IArithmetic &rhs) {
  if (auto res = IArithmetic::multiAdd(lhs, rhs)) {
    return res;
  }
  return lhs.addAbstract(rhs);
}

inline ArithmeticPtr operator-(const IArithmetic &lhs, const IArithmetic &rhs) {
  if (auto res = IArithmetic::multiSub(lhs, rhs)) {
    return res;
  }
  return lhs.substractAbstract(rhs);
}

inline ArithmeticPtr operator*(const IArithmetic &lhs, const IArithmetic &rhs) {
  if (auto res = IArithmetic::multiMul(lhs, rhs)) {
    return res;
  }
  return lhs.multiplyAbstract(rhs);
}

inline ArithmeticPtr operator/(const IArithmetic &lhs, const IArithmetic &rhs) {
  if (auto res = IArithmetic::multiDiv(lhs, rhs)) {
    return res;
  }
  return lhs.divideAbstract(rhs);
}

inline ArithmeticPtr operator+(const IArithmetic &rhs) {
  return rhs.convertAbstract();
}

inline ArithmeticPtr operator-(const IArithmetic &rhs) {
  return rhs.negateAbstract();
}

template <typename Derived>
class IArithmeticCRTP : virtual public IArithmetic, virtual public IMathObjectCRTP<Derived> {
public:
  Derived &operator+=(const Derived &rhs) {
    return add(rhs);
  }

  Derived &operator-=(const Derived &rhs) {
    return substract(rhs);
  }

  Derived &operator*=(const Derived &rhs) {
    return multiply(rhs);
  }

  Derived &operator/=(const Derived &rhs) {
    return divide(rhs);
  }

  Derived operator+(const Derived &rhs) const {
    return Derived(static_cast<const Derived &>(*this)) += rhs;
  }

  Derived operator-(const Derived &rhs) const {
    return Derived(static_cast<const Derived &>(*this)) -= rhs;
  }

  Derived operator*(const Derived &rhs) const {
    return Derived(static_cast<const Derived &>(*this)) *= rhs;
  }

  Derived operator/(const Derived &rhs) const {
    return Derived(static_cast<const Derived &>(*this)) /= rhs;
  }

  Derived operator+() const {
    return Derived(static_cast<const Derived &>(*this));
  }

  Derived operator-() const {
    Derived tmp = Derived(static_cast<const Derived &>(*this));
    return static_cast<IArithmeticCRTP<Derived> &>(tmp).negate();
  }

protected:
  virtual Derived &add(const Derived &rhs) = 0;

  virtual Derived &substract(const Derived &rhs) = 0;

  virtual Derived &multiply(const Derived &rhs) = 0;

  virtual Derived &divide(const Derived &rhs) = 0;

  virtual Derived &negate() = 0;

  ArithmeticPtr addAbstract(const IArithmetic &rhs) const final {
    return executeAbstract(
        rhs, "+", [this](IArithmeticCRTP<Derived> &lhs, const Derived &rhs) { return lhs.add(rhs); },
        [](const IArithmetic &lhs, const IArithmetic &rhs) { return lhs + rhs; });
  }

  ArithmeticPtr substractAbstract(const IArithmetic &rhs) const final {
    return executeAbstract(
        rhs, "-", [this](IArithmeticCRTP<Derived> &lhs, const Derived &rhs) { return lhs.substract(rhs); },
        [](const IArithmetic &lhs, const IArithmetic &rhs) { return lhs - rhs; });
  }

  ArithmeticPtr multiplyAbstract(const IArithmetic &rhs) const final {
    return executeAbstract(
        rhs, "*", [this](IArithmeticCRTP<Derived> &lhs, const Derived &rhs) { return lhs.multiply(rhs); },
        [](const IArithmetic &lhs, const IArithmetic &rhs) { return lhs * rhs; });
  }

  ArithmeticPtr divideAbstract(const IArithmetic &rhs) const final {
    return executeAbstract(
        rhs, "/", [this](IArithmeticCRTP<Derived> &lhs, const Derived &rhs) { return lhs.divide(rhs); },
        [](const IArithmetic &lhs, const IArithmetic &rhs) { return lhs / rhs; });
  }

  ArithmeticPtr convertAbstract() const final {
    return std::make_unique<Derived>(+(*this));
  }

  ArithmeticPtr negateAbstract() const final {
    return std::make_unique<Derived>(-(*this));
  }

private:
  ArithmeticPtr executeAbstract(const IArithmetic &rhs, const std::string &oper,
                                std::function<Derived(IArithmeticCRTP<Derived> &lhs, const Derived &rhs)> &&f1,
                                std::function<ArithmeticPtr(const IArithmetic &, const IArithmetic &)> &&f2) const {
    if (rhs.is<Derived>()) {
      auto tmpLhs = castPtr<IArithmeticCRTP<Derived>>(clone());
      return castPtr<IArithmetic>(f1(*tmpLhs, rhs.to<Derived>()).simplify());
    }
    if (MathObjectPtr tmpRhs = Converter::convert(rhs, *this); tmpRhs != nullptr) {
      auto tmpLhs = castPtr<IArithmeticCRTP<Derived>>(clone());
      return castPtr<IArithmetic>(f1(*tmpLhs, tmpRhs->to<Derived>()).simplify());
    }
    if (MathObjectPtr tmpLhs = Converter::convert(*this, rhs); tmpLhs != nullptr) {
      return castPtr<IArithmetic>(f2(tmpLhs->to<IArithmetic>(), rhs)->simplify());
    }
    throw InvalidInputBinaryOpearatorException(oper, toString(), rhs.toString());
  }
};

template <typename LhsType, typename RhsType,
          typename = std::enable_if_t<std::is_base_of_v<IArithmetic, LhsType> &&
                                      std::is_convertible_v<RhsType, LhsType> && !std::is_same_v<LhsType, RhsType>>>
LhsType &operator+=(LhsType &lhs, const RhsType &rhs) {
  return lhs += LhsType(rhs);
}

template <typename LhsType, typename RhsType,
          typename = std::enable_if_t<std::is_base_of_v<IArithmetic, LhsType> &&
                                      std::is_convertible_v<RhsType, LhsType> && !std::is_same_v<LhsType, RhsType>>>
LhsType &operator-=(LhsType &lhs, const RhsType &rhs) {
  return lhs -= LhsType(rhs);
}

template <typename LhsType, typename RhsType,
          typename = std::enable_if_t<std::is_base_of_v<IArithmetic, LhsType> &&
                                      std::is_convertible_v<RhsType, LhsType> && !std::is_same_v<LhsType, RhsType>>>
LhsType &operator*=(LhsType &lhs, const RhsType &rhs) {
  return lhs *= LhsType(rhs);
}

template <typename LhsType, typename RhsType,
          typename = std::enable_if_t<std::is_base_of_v<IArithmetic, LhsType> &&
                                      std::is_convertible_v<RhsType, LhsType> && !std::is_same_v<LhsType, RhsType>>>
LhsType &operator/=(LhsType &lhs, const RhsType &rhs) {
  return lhs /= LhsType(rhs);
}

template <typename LhsType, typename RhsType,
          typename = std::enable_if_t<std::is_base_of_v<IArithmetic, LhsType> &&
                                      std::is_convertible_v<RhsType, LhsType> && !std::is_same_v<LhsType, RhsType>>>
LhsType operator+(const LhsType &lhs, const RhsType &rhs) {
  return lhs + LhsType(rhs);
}

template <typename RhsType, typename LhsType,
          typename = std::enable_if_t<std::is_base_of_v<IArithmetic, RhsType> &&
                                      std::is_convertible_v<LhsType, RhsType> && !std::is_same_v<LhsType, RhsType>>>
RhsType operator+(const LhsType &lhs, const RhsType &rhs) {
  return RhsType(lhs) + rhs;
}

template <typename LhsType, typename RhsType,
          typename = std::enable_if_t<std::is_base_of_v<IArithmetic, LhsType> &&
                                      std::is_convertible_v<RhsType, LhsType> && !std::is_same_v<LhsType, RhsType>>>
LhsType operator-(const LhsType &lhs, const RhsType &rhs) {
  return lhs - LhsType(rhs);
}

template <typename RhsType, typename LhsType,
          typename = std::enable_if_t<std::is_base_of_v<IArithmetic, RhsType> &&
                                      std::is_convertible_v<LhsType, RhsType> && !std::is_same_v<LhsType, RhsType>>>
RhsType operator-(const LhsType &lhs, const RhsType &rhs) {
  return RhsType(lhs) - rhs;
}

template <typename LhsType, typename RhsType,
          typename = std::enable_if_t<std::is_base_of_v<IArithmetic, LhsType> &&
                                      std::is_convertible_v<RhsType, LhsType> && !std::is_same_v<LhsType, RhsType>>>
LhsType operator*(const LhsType &lhs, const RhsType &rhs) {
  return lhs * LhsType(rhs);
}

template <typename RhsType, typename LhsType,
          typename = std::enable_if_t<std::is_base_of_v<IArithmetic, RhsType> &&
                                      std::is_convertible_v<LhsType, RhsType> && !std::is_same_v<LhsType, RhsType>>>
RhsType operator*(const LhsType &lhs, const RhsType &rhs) {
  return RhsType(lhs) * rhs;
}

template <typename LhsType, typename RhsType,
          typename = std::enable_if_t<std::is_base_of_v<IArithmetic, LhsType> &&
                                      std::is_convertible_v<RhsType, LhsType> && !std::is_same_v<LhsType, RhsType>>>
LhsType operator/(const LhsType &lhs, const RhsType &rhs) {
  return lhs / LhsType(rhs);
}

template <typename RhsType, typename LhsType,
          typename = std::enable_if_t<std::is_base_of_v<IArithmetic, RhsType> &&
                                      std::is_convertible_v<LhsType, RhsType> && !std::is_same_v<LhsType, RhsType>>>
RhsType operator/(const LhsType &lhs, const RhsType &rhs) {
  return RhsType(lhs) / rhs;
}

}