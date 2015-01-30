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

  const TokenType type;
  const std::string value;
};

int _stackTopMatches( const std::vector<Token>& tokenStack,
                      const TokenType& type) {

  if (tokenStack.size() == 0)

    return -1;

  int index = tokenStack.size() - 1;

  if (tokenStack[index].type == type)

    return index;
  else

    return -1;
}

template <typename... Args>
int _stackTopMatches(const std::vector<Token>& tokenStack,
                              const TokenType& type,
                              Args... args) {

  int index = _stackTopMatches(tokenStack, args...) - 1;

  if (index < 0)

    return -1;

  if (tokenStack[index].type == type)

    return index;
  else

    return -1;
}

template <typename... Args>
bool stackTopMatches( const std::vector<Token>& tokenStack,
                      Args... args) {

  return _stackTopMatches(tokenStack, args...) >= 0;
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

    if (stackTopMatches(tokenStack, TokenType::CHAR)) {

      Token t0 = tokenStack.back(); tokenStack.pop_back();
      tokenStack.emplace_back(TokenType::EXPR, t0.value);
      faStack.push_back(std::move(fromChar(t0.value.front())));

#ifdef DEBUG
      fprintf(stderr, "Reduce '%s'\n", t0.value.c_str());
#endif  // DEBUG

      expr();
    } else if ( stackTopMatches(tokenStack, TokenType::EXPR, TokenType::EXPR) and
                it->type != TokenType::STAR) {

      Token t1 = tokenStack.back(); tokenStack.pop_back();
      Token t0 = tokenStack.back(); tokenStack.pop_back();
      tokenStack.emplace_back(TokenType::EXPR, t0.value + t1.value);

#ifdef DEBUG
      fprintf(stderr, "Reduce '%s' + '%s'\n", t0.value.c_str(), t1.value.c_str());
#endif  // DEBUG

      std::unique_ptr<FA> fa1 = std::move(faStack.back()); faStack.pop_back();
      std::unique_ptr<FA> fa0 = std::move(faStack.back()); faStack.pop_back();
      faStack.push_back(std::move(FA::concatenate(std::move(fa0), std::move(fa1))));
    } else if (stackTopMatches(tokenStack, TokenType::EXPR, TokenType::STAR)) {

      Token t1 = tokenStack.back(); tokenStack.pop_back();
      Token t0 = tokenStack.back(); tokenStack.pop_back();
      tokenStack.emplace_back(TokenType::EXPR, t0.value + t1.value);

#ifdef DEBUG
      fprintf(stderr, "Reduce '%s' + '%s'\n", t0.value.c_str(), t1.value.c_str());
#endif  // DEBUG

      std::unique_ptr<FA> fa = std::move(faStack.back()); faStack.pop_back();
      faStack.push_back(std::move(FA::repeat(std::move(fa))));
    } else if (stackTopMatches(tokenStack,  TokenType::EXPR, TokenType::V_BAR,
                                            TokenType::EXPR)) {

      Token t2 = tokenStack.back(); tokenStack.pop_back();
      Token t1 = tokenStack.back(); tokenStack.pop_back();
      Token t0 = tokenStack.back(); tokenStack.pop_back();
      tokenStack.emplace_back(TokenType::EXPR, t0.value + t1.value + t2.value);

#ifdef DEBUG
      fprintf(stderr, "Reduce '%s' + '%s' + '%s'\n", t0.value.c_str(), t1.value.c_str(), t2.value.c_str());
#endif  // DEBUG

      std::unique_ptr<FA> fa1 = std::move(faStack.back()); faStack.pop_back();
      std::unique_ptr<FA> fa0 = std::move(faStack.back()); faStack.pop_back();
      faStack.push_back(std::move(FA::alternate(std::move(fa0), std::move(fa1))));
    } else if (stackTopMatches(tokenStack,  TokenType::L_PAREN, TokenType::EXPR,
                                            TokenType::R_PAREN)) {

      Token t2 = tokenStack.back(); tokenStack.pop_back();
      Token t1 = tokenStack.back(); tokenStack.pop_back();
      Token t0 = tokenStack.back(); tokenStack.pop_back();
      tokenStack.emplace_back(TokenType::EXPR, t0.value + t1.value + t2.value);

#ifdef DEBUG
      fprintf(stderr, "Reduce '%s' + '%s' + '%s'\n", t0.value.c_str(), t1.value.c_str(), t2.value.c_str());
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
