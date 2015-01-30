#pragma once

#include "FA.h"

#include <string>
#include <set>
#include <memory>

struct Transition {

  std::string start;
  FA::symbol_type symbol;
  std::string end;
};

bool operator < (const Transition&, const Transition&);

class FABuilder {

public:

  FABuilder() = default;

  FABuilder& initial_state(const std::string&);

  FABuilder& transition(const std::string&, const FA::symbol_type&,
                        const std::string&);

  FABuilder& final_state(const std::string&);

  std::unique_ptr<FA> build() const;

private:

  std::string _initial_state;
  std::set<Transition> _transitions;
  std::set<std::string> _final_states;
};
