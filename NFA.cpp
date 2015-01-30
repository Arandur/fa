#include "NFA.h"

#include "FABuilder.h"

#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <iterator>

/**
 * Runs the NFA on str, returning true if it accepts.
 * 
 * @param  str The string.
 * @return     true if the NFA accepts str, else false.
 */
bool NFA::operator () (const std::string& str) const {

  std::set<state_type> end_states = delta({initial_state}, str);

  return std::find_first_of(std::begin(end_states), std::end(end_states),
                            std::begin(final_states), std::end(final_states)) !=
    std::end(end_states);
}

std::unique_ptr<FA> NFA::normalize() const {

  return FA::normalize(makeDeterministic());
}

std::string state_from_set(const std::set<FA::state_type>& states) {

  std::string newState;

  for (const FA::state_type& state : states)

    newState += std::to_string(state);

  return newState;
}

std::unique_ptr<FA> NFA::makeDeterministic() const {

  FABuilder faBuilder;

  std::vector<std::set<state_type>> stateStack;
  std::vector<state_type> eps_init = epsilon_closure(initial_state);

  stateStack.emplace_back(std::begin(eps_init),
                          std::end(eps_init));

  faBuilder.initial_state(state_from_set(stateStack.back()));

  for (size_t currState = 0; currState < stateStack.size(); ++currState) {

    for (size_t i = 0; i < symbols.size(); ++i) {

      if (symbols[i] == EPSILON) continue;

      const symbol_type symbol = symbols[i];
      const std::set<state_type> startState = stateStack[currState];
      const std::set<state_type> endState = delta(startState, symbol);

      if (std::find(std::begin(stateStack), std::end(stateStack), endState) == 
                    std::end(stateStack)) {

        stateStack.push_back(endState);

        if (std::find_first_of( std::begin(final_states),
                                std::end(final_states),
                                std::begin(endState), std::end(endState)) !=
            std::end(final_states))

          faBuilder.final_state(state_from_set(endState));
      }

      faBuilder.transition( state_from_set(startState), symbol,
                            state_from_set(endState));
    }
  }

  return faBuilder.build();
}

std::set<FA::state_type> NFA::delta(const std::set<state_type>& qs, 
                                    const std::string& str) const {

  std::set<state_type> currStates = qs;

  for (const symbol_type& a : str)

    if ((currStates = delta(currStates, a)).empty())

      return currStates;

  return currStates;
}

std::set<FA::state_type> NFA::delta(const std::set<state_type>& qs,
                                    const symbol_type& a) const {

  std::set<state_type> eps_set, end_states, eps_end_states;

  size_t index = std::distance( std::begin(symbols),
                                std::find(std::begin(symbols),
                                          std::end(symbols),
                                          a));

  if (index == symbols.size())

    return {};

  for (const state_type& q : qs)

    for (const state_type& q_eps : epsilon_closure(q))

      eps_set.insert(q_eps);

  for (const std::pair<state_type, state_type>& pair : transitions[index])

    if (eps_set.find(pair.first) != std::end(eps_set))

      end_states.insert(pair.second);

  for (const state_type& q : end_states)

    for (const state_type& q_eps : epsilon_closure(q))

      eps_end_states.insert(q_eps);

  return eps_end_states;
}

std::vector<FA::state_type> NFA::epsilon_closure(const state_type& q) const {

  std::vector<state_type> eps_cls {q};

  size_t index = std::distance( std::begin(symbols), 
                                std::find(std::begin(symbols), 
                                          std::end(symbols), 
                                          EPSILON));

  if (index == symbols.size())

    return eps_cls;

  for (size_t currState = 0; currState < eps_cls.size(); ++currState)

    for (const std::pair<state_type, state_type>& pair : transitions[index])

      if (pair.first == eps_cls[currState] &&
          std::find(std::begin(eps_cls), std::end(eps_cls), pair.second) ==
          std::end(eps_cls))

        eps_cls.push_back(pair.second);

  return eps_cls;
}
