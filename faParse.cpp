#include "FA.h"

#include "FABuilder.h"
#include "FAExcept.h"

#include <memory>
#include <string>
#include <vector>

#ifdef DEBUG
#include <cstdio>
#endif  // DEBUG

enum class TokenType {

  CHAR,
  STAR,
  V_BAR,
  L_PAREN,
  R_PAREN,
  EXPR
};

struct Token {

  Token(const TokenType& type, const std::string& value) :
    type(type),
    value(value) {}

  Token(const Token& cToken) :
    type(cToken.type),
    value(cToken.value) {}

  Token(Token&& mToken) :
    type(mToken.type),
    value(mToken.value) {}

  Token& operator = (Token&& mToken) {

    std::swap(*this, mToken);

    return *this;
  }

  const TokenType type;
  const std::string value;
};

typename std::vector<Token>::iterator stackTopMatch(
  std::vector<Token>& tokenStack,
  const TokenType& type) {

  if (tokenStack.size() == 0)

    return std::end(tokenStack);

  auto it = --std::end(tokenStack);

  if (it->type == type)

    return it;
  else

    return std::end(tokenStack);
}

template <typename... Args>
typename std::vector<Token>::iterator stackTopMatch(
  std::vector<Token>& tokenStack,
  const TokenType& type,
  Args... args) {

  auto it = stackTopMatch(tokenStack, args...);

  if (it == std::end(tokenStack))

    return it;

  if ((--it)->type == type)

    return it;
  else

    return std::end(tokenStack);
}

template <typename... Args>
std::vector<Token> popIfMatch(std::vector<Token>& tokenStack,
                              Args... args) {

  auto it = stackTopMatch(tokenStack, args...);

  std::vector<Token> stackTop;

  std::copy(it, std::end(tokenStack), std::back_inserter(stackTop));
  tokenStack.erase(it, std::end(tokenStack));

  return stackTop;
}

std::vector<Token> lex(const std::string& regex) {

  std::vector<Token> tokens;

  /* Can't use std::transform because I have to deal with backslashes... */
  for (auto it = std::begin(regex); it != std::end(regex); ++it)

    switch (*it) {

    default :
      tokens.emplace_back(TokenType::CHAR, std::string {*it});
      break;

    case '\\' :
      tokens.emplace_back(TokenType::CHAR, std::string {*++it});
      break;

    case '*' :
      tokens.emplace_back(TokenType::STAR, "*");
      break;

    case '|' :
      tokens.emplace_back(TokenType::V_BAR, "|");
      break;

    case '(' :
      tokens.emplace_back(TokenType::L_PAREN, "(");
      break;

    case ')' :
      tokens.emplace_back(TokenType::R_PAREN, ")");
      break;
    }

  return tokens;
}

std::unique_ptr<FA> parse(const std::vector<Token> tokens) {

  std::vector<Token> tokenStack;
  std::vector<Token> tokenSlice;
  std::vector<std::unique_ptr<FA>> faStack;

  auto fromChar = [] (const char& c) -> std::unique_ptr<FA> {

    return FABuilder()
      .initial_state("0")
      .transition("0", c, "1")
      .final_state("1")
      .build();
  };

  auto it = std::begin(tokens);
 
  std::function<void (void)> expr = [&] () {

    if (!(tokenSlice = popIfMatch(tokenStack, TokenType::CHAR)).empty()) {

      tokenStack.emplace_back(TokenType::EXPR, tokenSlice[0].value);
      faStack.push_back(std::move(fromChar(tokenSlice[0].value.front())));

#ifdef DEBUG
      fprintf(stderr, "Reduce '%s'\n", tokenSlice[0].value.c_str());
#endif  // DEBUG

      expr();
    } else if (!(tokenSlice = popIfMatch(tokenStack,  TokenType::EXPR, 
                                                      TokenType::EXPR)).empty() 
                and it->type != TokenType::STAR) {

      tokenStack.emplace_back(TokenType::EXPR,  tokenSlice[0].value + 
                                                tokenSlice[1].value);

#ifdef DEBUG
      fprintf(stderr, "Reduce '%s' + '%s'\n", tokenSlice[0].value.c_str(), 
                                              tokenSlice[1].value.c_str());
#endif  // DEBUG

      std::unique_ptr<FA> fa1 = std::move(faStack.back()); faStack.pop_back();
      std::unique_ptr<FA> fa0 = std::move(faStack.back()); faStack.pop_back();
      faStack.push_back(std::move(FA::concatenate(std::move(fa0), std::move(fa1))));
    } else if (!(tokenSlice = popIfMatch(tokenStack,  TokenType::EXPR, 
                                                      TokenType::STAR)).empty()) {

      tokenStack.emplace_back(TokenType::EXPR,  tokenSlice[0].value + 
                                                tokenSlice[1].value);

#ifdef DEBUG
      fprintf(stderr, "Reduce '%s' + '%s'\n", tokenSlice[0].value.c_str(), 
                                              tokenSlice[1].value.c_str());
#endif  // DEBUG

      std::unique_ptr<FA> fa = std::move(faStack.back()); faStack.pop_back();
      faStack.push_back(std::move(FA::repeat(std::move(fa))));
    } else if (!(tokenSlice = popIfMatch(tokenStack,  TokenType::EXPR, 
                                                      TokenType::V_BAR,
                                                      TokenType::EXPR)).empty()) {

      tokenStack.emplace_back(TokenType::EXPR,  tokenSlice[0].value + 
                                                tokenSlice[1].value + 
                                                tokenSlice[2].value);

#ifdef DEBUG
      fprintf(stderr, "Reduce '%s' + '%s' + '%s'\n",  tokenSlice[0].value.c_str(),
                                                      tokenSlice[1].value.c_str(),
                                                      tokenslice[2].value.c_str());
#endif  // DEBUG

      std::unique_ptr<FA> fa1 = std::move(faStack.back()); faStack.pop_back();
      std::unique_ptr<FA> fa0 = std::move(faStack.back()); faStack.pop_back();
      faStack.push_back(std::move(FA::alternate(std::move(fa0), std::move(fa1))));
    } else if (!(tokenSlice = popIfMatch(tokenStack,  TokenType::L_PAREN, 
                                                      TokenType::EXPR,
                                                      TokenType::R_PAREN)).empty()) {

      tokenStack.emplace_back(TokenType::EXPR,  tokenSlice[0].value + 
                                                tokenSlice[1].value + 
                                                tokenSlice[2].value);

#ifdef DEBUG
      fprintf(stderr, "Reduce '%s' + '%s' + '%s'\n",  tokenSlice[0].value.c_str(), 
                                                      tokenSlice[1].value.c_str(), 
                                                      tokenSlice[2].value.c_str());
#endif  // DEBUG
    } else {

      if (it == std::end(tokens))

        return;

      tokenStack.push_back(*it++);

#ifdef DEBUG
      fprintf(stderr, "Shift '%s'\n", tokenStack.back().value.c_str());
#endif  // DEBUG
    }

    expr();
  };

  expr();

  if (faStack.size() != 1)

    throw BadParse();

  return std::move(faStack.front());
}

std::unique_ptr<FA> FA::fromRegex(const std::string& regex) {

  if (regex.length() == 0)

    return FABuilder()
      .initial_state("0")
      .final_state("0")
      .build();

  std::vector<Token> tokens = lex(regex);

  try {

    return parse(tokens);
  } catch (const BadParse& e) {

    throw BadRegex(regex);
  }
}
