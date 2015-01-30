#include "DFA.h"

#include "NFA.h"
#include "FABuilder.h"
#include "FAExcept.h"

#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <functional>

/**
 * Runs the DFA on str, returning true if it accepts.
 * 
 * @param  str The string.
 * @return     true if the DFA accepts str, else false.
 */
bool DFA::operator () (const std::string& str) const {

  try {

    state_type final_state = delta(initial_state, str);

    return std::binary_search(std::begin(final_states), std::end(final_states),
                              final_state);
  } catch (const NoTransitionException& e) {

    return false;
  }
}

std::unique_ptr<FA> DFA::normalize()      const {

  return dynamic_cast<DFA*>(removeDeadStates().get())->minimizeStates();
}

std::unique_ptr<FA> DFA::reverse()        const {

  FABuilder faBuilder;

  std::string q_0 = "q_0";

  faBuilder.initial_state(q_0);

  for (const state_type& f : final_states)

    faBuilder.transition(q_0, EPSILON, std::to_string(f));

  for (size_t i = 0; i < symbols.size(); ++i) {

    const symbol_type symbol = symbols[i];

    for (const std::pair<state_type, state_type>& pair : transitions[i]) {

      faBuilder.transition( std::to_string(pair.second), symbol,
                            std::to_string(pair.first));
    }
  }

  faBuilder.final_state(std::to_string(initial_state));

  return faBuilder.build();
}

/* Uses Brzozowsiki's Algorithm for DFA minimization. */
std::unique_ptr<FA> DFA::minimizeStates() const {

  if (final_states.size() == 0)

    return std::unique_ptr<FA>(new DFA(*this));

  return dynamic_cast<const NFA*>(
    dynamic_cast<const DFA*>(
      dynamic_cast<const NFA*>(
        reverse().get()
      )->makeDeterministic().get()
    )->reverse().get()
  )->makeDeterministic();
}

FA::state_type DFA::delta(const state_type& q, const std::string& str) const {

  state_type currState = q;

  for (const symbol_type& c : str)

    currState = delta(currState, c);

  return currState;
}

FA::state_type DFA::delta(const state_type& q, const symbol_type& a) const {

  size_t i = std::distance( std::begin(symbols),
                            std::lower_bound( std::begin(symbols),
                                              std::end(symbols),
                                              a));

  if (i == transitions.size() || symbols[i] != a)

    throw NoTransitionException(q, a);

  const std::vector<std::pair<state_type, state_type>> ts = transitions[i];

  auto it = std::lower_bound( std::begin(ts), std::end(ts), q,
                              [] (const std::pair<state_type, state_type>& pair,
                                  const state_type& q) -> bool {

                                return pair.first < q;
                              });

  if (it == std::end(ts))

    throw NoTransitionException(q, a);

  return it->second;
}
