#include "FABuilder.h"

#include "NFA.h"
#include "DFA.h"

#include <map>
#include <vector>
#include <utility>
#include <algorithm>
#include <iterator>

bool operator < (const Transition& a, const Transition& b) {

  return a.start < b.start ? true :
    a.symbol < b.symbol ? true :
      a.end < b.end;
}

FABuilder& FABuilder::initial_state(const std::string& state) {

  _initial_state = state;

  return *this;
}

FABuilder& FABuilder::transition( const std::string& start,
                                  const FA::symbol_type& symbol,
                                  const std::string& end) {

  _transitions.insert({start, symbol, end});

  return *this;
}

FABuilder& FABuilder::final_state(const std::string& state) {

  _final_states.insert(state);

  return *this;
}

std::vector<std::string> enumerateStates( const std::string& initial_state,
                                          const std::set<Transition>& transitions) {

  std::vector<std::string> states {initial_state};
  size_t currState = 0;

  while (currState != states.size()) {

    for (const Transition& transition : transitions)

      if (transition.start == states[currState])

        if (std::find(std::begin(states), std::begin(states) + currState + 1, 
                      transition.end) ==  std::begin(states) + currState + 1)

          states.push_back(transition.end);

    ++currState;
  }

  return states;
}

std::unique_ptr<FA> FABuilder::build() const {

  std::vector<char> sigma;
  std::vector<std::vector<std::pair<FA::state_type, FA::state_type>>> delta;

  std::vector<std::string> states = enumerateStates(_initial_state, _transitions);

  /* This should always be 0, but why not be safe? */
  FA::state_type q_0 = std::distance( std::begin(states),
                                      std::find(std::begin(states),
                                      std::end(states),
                                      _initial_state));

  std::vector<FA::state_type> f;

  std::transform( std::begin(_final_states), std::end(_final_states),
                  std::back_inserter(f),
                  [&] (const std::string& state) -> FA::state_type {

                    return std::distance( std::begin(states),
                                          std::find(std::begin(states),
                                          std::end(states),
                                          state));
                  });

  std::sort(std::begin(f), std::end(f));

  std::transform( std::begin(_transitions), std::end(_transitions),
                  std::back_inserter(sigma),
                  [] (const Transition& transition) -> char {

                    return transition.symbol;
                  });

  std::sort(std::begin(sigma), std::end(sigma));

  sigma.erase(std::unique(std::begin(sigma), std::end(sigma)), std::end(sigma));

  delta.resize(sigma.size());

  for (const Transition& transition : _transitions)

    delta[
      std::distance(std::begin(sigma), 
                    std::lower_bound( std::begin(sigma), 
                                      std::end(sigma), 
                                      transition.symbol))
    ].emplace_back( std::distance(std::begin(states), 
                                  std::find(std::begin(states), 
                                            std::end(states), 
                                            transition.start)), 
                    std::distance(std::begin(states),
                                  std::find(std::begin(states),
                                            std::end(states),
                                            transition.end)));

  for (std::vector<std::pair<FA::state_type, FA::state_type>>& elem : delta)

    std::sort(std::begin(elem), std::end(elem));

  /* Determine if we have an NFA or a DFA */

  if (std::binary_search(std::begin(sigma), std::end(sigma), EPSILON)) {

    return std::unique_ptr<FA>(new NFA(q_0, f, sigma, delta));
  }

  for (const std::vector<std::pair<FA::state_type, FA::state_type>>& elem : delta)

    if (std::adjacent_find( std::begin(elem), std::end(elem),
                            [] (const std::pair<FA::state_type, FA::state_type>& first,
                                const std::pair<FA::state_type, FA::state_type>& second)
                                -> bool {

                              return first.first == second.first;
                            }) != std::end(elem)) {

      return std::unique_ptr<FA>(new NFA(q_0, f, sigma, delta));
  }

  return std::unique_ptr<FA>(new DFA(q_0, f, sigma, delta));
}
