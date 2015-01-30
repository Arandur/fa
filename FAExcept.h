#pragma once

#include "FA.h"

#include <stdexcept>
#include <sstream>

struct FAException :
  public std::exception {};

class NoTransitionException :
  public FAException {

public:

  NoTransitionException(const FA::state_type& q, const FA::symbol_type& a) :
    q(q),
    a(a) {}

  virtual const char* what() const noexcept(true) {

    std::stringstream ss;

    ss << "No transition exists from state " << q << " on symbol '" << a <<
          "'.";

    return ss.str().c_str();
  }

private:

  const FA::state_type q;
  const FA::symbol_type a;
};

class BadParse :
  public FAException {};

class BadRegex :
  public FAException {

public:

  BadRegex(const std::string& regex) :
    regex(regex) {}

  virtual const char* what() const noexcept(true) {

    std::stringstream ss;

    ss << "Could not parse \"" << regex << "\".";

    return ss.str().c_str();
  }

private:

  const std::string& regex;
};
