#pragma once

#include "FA.h"

#include <set>

class NFA :
  public FA {

public:

  NFA() = delete;

  NFA(const NFA&) = default;

  virtual bool operator () (const std::string&) const;

  friend class FABuilder;
  friend class DFA;

private:

  NFA(const state_type& initial_state,
      const std::vector<state_type>& final_states,
      const std::vector<symbol_type>& symbols,
      const std::vector<std::vector<std::pair<state_type, state_type>>>& transitions) :
    FA(initial_state, final_states, symbols, transitions) {}


  virtual std::unique_ptr<FA> normalize()         const;

          std::unique_ptr<FA> makeDeterministic() const;

  std::set<state_type>    delta(const std::set<state_type>&, 
                                const std::string&) const;
  std::set<state_type>    delta(const std::set<state_type>&, 
                                const symbol_type&) const;
  std::vector<state_type> epsilon_closure(const state_type&) const;
};
