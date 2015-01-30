#pragma once

#include <string>
#include <vector>
#include <set>
#include <utility>
#include <memory>

const char EPSILON = '\0';

class FA {

public:

  typedef char     symbol_type;
  typedef unsigned state_type;

  FA() = delete;

  FA(const FA&) = default;

  virtual bool match (const char*, const char*) const = 0;

  virtual std::pair<const char*, const char*> findNext( const char*, 
                                                            const char*) const = 0;

  static std::unique_ptr<FA> concatenate(std::unique_ptr<FA>, std::unique_ptr<FA>);
  static std::unique_ptr<FA> alternate  (std::unique_ptr<FA>, std::unique_ptr<FA>);
  static std::unique_ptr<FA> repeat     (std::unique_ptr<FA>);
  static std::unique_ptr<FA> normalize  (std::unique_ptr<FA>);

  static std::unique_ptr<FA> fromRegex  (const std::string&);

  friend class FABuilder;

protected:

  FA( const state_type& initial_state,
      const std::vector<state_type>& final_states,
      const std::vector<symbol_type>& symbols,
      const std::vector<std::vector<std::pair<state_type, state_type>>>& transitions) :
    initial_state(initial_state),
    final_states(final_states),
    symbols(symbols),
    transitions(transitions) {}

  const state_type initial_state;
  const std::vector<state_type> final_states;
  const std::vector<symbol_type> symbols;
  const std::vector<std::vector<std::pair<state_type, state_type>>> transitions;

          std::set<state_type> findDeadStates()   const;
  virtual std::unique_ptr<FA>  removeDeadStates() const;

private:

  virtual std::unique_ptr<FA>  normalize()        const = 0;
};
