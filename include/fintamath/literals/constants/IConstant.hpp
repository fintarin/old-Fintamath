#pragma once

#include <memory>

#include "fintamath/literals/ILiteral.hpp"

namespace fintamath {

class IConstant;
using ConstantPtr = std::unique_ptr<IConstant>;

class IConstant : virtual public ILiteral {
public:
  template <typename T, typename = std::enable_if_t<std::is_base_of_v<IConstant, T>>>
  static void registerParser() {
    Parser::registerParser<T>(parserMap);
  }

  static ConstantPtr parse(const std::string &parsedStr) {
    return Parser::parse<ConstantPtr>(parserMap, parsedStr, [](const ConstantPtr &) { return true; });
  }

  MathObjectPtr operator()() const {
    return call();
  }

protected:
  virtual MathObjectPtr call() const = 0;

private:
  static Parser::ParserMap<ConstantPtr> parserMap;
};

template <typename Derived>
class IConstantCRTP : virtual public IConstant, virtual public ILiteralCRTP<Derived> {};

}