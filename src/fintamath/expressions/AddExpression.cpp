#include "fintamath/expressions/AddExpression.hpp"
#include "fintamath/exceptions/InvalidInputException.hpp"

namespace fintamath{

  AddExpression::AddExpression(const AddExpression & /*rhs*/) noexcept{

  }

  AddExpression::AddExpression(AddExpression && /*rhs*/) noexcept{

  }

  AddExpression::AddExpression(const TokenVector& tokens){ 
      parse(tokens);
  }

  std::string AddExpression::getClassName() const {
      return "AddExpression";
  }

  std::string AddExpression::toString() const {
    std::string result;
    result.push_back('(');
    for(const auto & var : addPolynom){
      result += var.info->toString();
      result += var.inverted ? '-' : '+';
    }
    result.pop_back();
    result.push_back(')');
    return result;
  }

  void AddExpression::parse(const TokenVector& tokens){
    for(size_t i = 0;i < tokens.size();i++){
      if(tokens[i] == "(" && !skipBrackets(tokens, i)){
        throw InvalidInputException(*this, " braces must be closed");
      }
      if(i == tokens.size()){
        break;
      }
      if(tokens[i] != "+" && tokens[i] != "-"){
        continue;
      }
      if(i == tokens.size() - 1){
        throw InvalidInputException(*this, " unexpected sign");
      }
      if (i == 0 || (isOneSymbolToken(tokens[i-1]) && tokens[i-1] != "%")) {
      continue;
      }

      addPolynom.emplace_back(Element(IExpression::parse(TokenVector(tokens.begin(), tokens.begin() + (long)i))));
      addPolynom.emplace_back(Element(IExpression::parse(TokenVector(tokens.begin() + (long)i + 1, tokens.end())), tokens[i] == "-"));
      return;
    }
    throw InvalidInputException(*this, " not an AddExpression");
  }
    
  AddExpression::Element::Element(const Element &rhs) : inverted(rhs.inverted) {
    info = rhs.info->clone();
  }

  AddExpression::Element &AddExpression::Element::operator=(const Element &rhs) {
    if (this != &rhs) {
      info = rhs.info->clone();
      inverted = rhs.inverted;
    }
    return *this;
  }

  AddExpression::Element::Element(MathObjectPtr info, bool inverted) : info(info->clone()), inverted(inverted){}


}