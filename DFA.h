#pragma once

#include "FA.h"

#include <set>

class DFA :
  public FA {

public:

  DFA() = delete;

  DFA(const DFA&) = default;

  virtual bool match (const char*, const size_t&) const;

  virtual bool match( std::string::const_iterator,
                      std::string::const_iterator) const;

  virtual std::pair<const char*, const size_t> findNext(const char*, 
                                                        const size_t&) const;

  virtual std::pair<std::string::const_iterator, std::string::const_iterator>
    findNext(std::string::const_iterator, std::string::const_iterator) const;

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

  template <typename InputIterator>
  state_type delta(const state_type&, InputIterator, InputIterator) const;
  state_type delta(const state_type&, const symbol_type&) const;
};
