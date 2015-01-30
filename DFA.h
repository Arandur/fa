#pragma once

#include "FA.h"

#include <set>

class DFA :
  public FA {

public:

  DFA() = delete;

  DFA(const DFA&) = default;

  virtual bool operator () (const std::string&) const;

  friend class FABuilder;
  friend class NFA;

private:

  DFA(const state_type& initial_state,
      const std::vector<state_type>& final_states,
      const std::vector<symbol_type>& symbols,
      const std::vector<std::vector<std::pair<state_type, state_type>>>& transitions) :
    FA(initial_state, final_states, symbols, transitions) {}

  virtual std::unique_ptr<FA> normalize()      const;
          std::unique_ptr<FA> reverse()        const;
          std::unique_ptr<FA> minimizeStates() const;

  state_type delta(const state_type&, const std::string&) const;
  state_type delta(const state_type&, const symbol_type&) const;
};
