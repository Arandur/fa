#include "FA.h"

#include "NFA.h"
#include "DFA.h"
#include "FABuilder.h"
#include "FAExcept.h"

#include <vector>
#include <algorithm>
#include <iterator>
#include <map>

static std::string prefix(unsigned int faNumber, const FA::state_type& state) {

  return std::to_string(faNumber) + std::to_string(state);
}

static auto p1 = [&] (const FA::state_type& state) -> std::string {

  return prefix(1, state);
};

static auto p2 = [&] (const FA::state_type& state) -> std::string {

  return prefix(2, state);
};

std::unique_ptr<FA> FA::concatenate(std::unique_ptr<FA> fa1, 
                                    std::unique_ptr<FA> fa2) {

  FABuilder faBuilder;

  faBuilder.initial_state(p1(fa1->initial_state));

  for (size_t i = 0; i < fa1->symbols.size(); ++i) {

    const symbol_type symbol = fa1->symbols[i];

    for (const std::pair<state_type, state_type>& pair : fa1->transitions[i]) {

      faBuilder.transition(p1(pair.first), symbol, p1(pair.second));
    }
  }

  for (const state_type& f : fa1->final_states)

    faBuilder.transition(p1(f), EPSILON, p2(fa2->initial_state));

  for (size_t i = 0; i < fa2->symbols.size(); ++i) {

    const symbol_type symbol = fa2->symbols[i];

    for (const std::pair<state_type, state_type>& pair : fa2->transitions[i]) {

      faBuilder.transition(p2(pair.first), symbol, p2(pair.second));
    }
  }

  for (const state_type& f : fa2->final_states)

    faBuilder.final_state(p2(f));

  return faBuilder.build();
}

std::unique_ptr<FA> FA::alternate  (std::unique_ptr<FA> fa1, 
                                    std::unique_ptr<FA> fa2) {

  FABuilder faBuilder;

  std::string q_0 = "ALT1_2";
  faBuilder.initial_state(q_0);
  faBuilder.transition(q_0, EPSILON, p1(fa1->initial_state));
  faBuilder.transition(q_0, EPSILON, p2(fa2->initial_state));

  for (size_t i = 0; i < fa1->symbols.size(); ++i) {

    const symbol_type symbol = fa1->symbols[i];

    for (const std::pair<state_type, state_type>& pair : fa1->transitions[i]) {

      const state_type startState = pair.first;
      const state_type endState   = pair.second;

      faBuilder.transition(p1(startState), symbol, p1(endState));
    }
  }

  for (size_t i = 0; i < fa2->symbols.size(); ++i) {

    const symbol_type symbol = fa2->symbols[i];

    for (const std::pair<state_type, state_type>& pair : fa2->transitions[i]) {

      const state_type startState = pair.first;
      const state_type endState   = pair.second;

      faBuilder.transition(p2(startState), symbol, p2(endState));
    }
  }

  for (const state_type& f : fa1->final_states)

    faBuilder.final_state(p1(f));

  for (const state_type& f : fa2->final_states)

    faBuilder.final_state(p2(f));

  return faBuilder.build();
}

std::unique_ptr<FA> FA::repeat     (std::unique_ptr<FA> fa1) {

  FABuilder faBuilder;

  faBuilder.initial_state(p1(fa1->initial_state));
  faBuilder.final_state(p1(fa1->initial_state));

  for (size_t i = 0; i < fa1->symbols.size(); ++i) {

    const symbol_type symbol = fa1->symbols[i];

    for (const std::pair<state_type, state_type>& pair : fa1->transitions[i]) {

      const state_type startState = pair.first;
      const state_type endState   = pair.second;

      faBuilder.transition(p1(startState), symbol, p1(endState));
    }
  }

  for (const state_type& f : fa1->final_states)

    faBuilder.transition(p1(f), EPSILON, p1(fa1->initial_state));

  return faBuilder.build();
}

/**
 * Returns a normalized FA which is functionally identical to fa1.
 *
 * A normalized FA is an NFA with the minimim possible number of states.
 * 
 * @param  fa1 The FA.
 * @return     A normalized FA.
 */
std::unique_ptr<FA> FA::normalize  (std::unique_ptr<FA> fa1) {

  return fa1->normalize();
}

/**
 * Finds the dead states in the FA.
 *
 * A dead state is a state which cannot be reached from the initial_state, or
 * one from which no final_state can be reached.
 *
 * @return The set of all dead states in the FA.
 */
std::set<FA::state_type> FA::findDeadStates() const {

  std::map<state_type, std::set<state_type>> reachableFrom;
  std::vector<state_type> stateStack;
  std::set<state_type> deadStates;

  for (const std::vector<std::pair<state_type, state_type>>& pairs : transitions) {

    for (const std::pair<state_type, state_type>& pair : pairs) {

      reachableFrom[pair.first].insert(pair.first);
      reachableFrom[pair.first].insert(pair.second);
    }
  }

  for (std::pair<const state_type, std::set<state_type>>& pair : reachableFrom) {

    stateStack.clear();

    stateStack.push_back(pair.first);

    std::copy(std::begin(pair.second), std::end(pair.second),
              std::back_inserter(stateStack));

    stateStack.erase( std::unique(std::begin(stateStack), 
                                  std::end(stateStack)), 
                      std::end(stateStack));

    size_t currState = 1;

    for (; currState < stateStack.size(); ++currState) {

      for (const state_type& reachableState : reachableFrom[stateStack[currState]]) {

        if (!std::binary_search(std::begin(pair.second), std::end(pair.second),
                                reachableState)) {

          stateStack.push_back(reachableState);

          pair.second.insert(reachableState);
        }
      }
    }

    if (std::find_first_of(std::begin(final_states), std::end(final_states),
                           std::begin(pair.second),  std::end(pair.second)) ==
        std::end(final_states))

      deadStates.insert(pair.first);
  }

  /* We also need to make sure that every state is reachable from the initial
   * state!
   */
  const std::set<state_type> reachableFromFirst = reachableFrom[initial_state];
  
  for (const std::pair<state_type, std::set<state_type>>& pair : reachableFrom) {

    const state_type currState = pair.first;

    if (!std::binary_search(std::begin(reachableFromFirst),
                            std::end(reachableFromFirst),
                            currState))

      deadStates.insert(pair.first);
  }

  return deadStates;
}

/**
 * Returns an FA which is functionally identical to this one, but with its dead
 * states removed.
 *
 * Dead states do not affect the set of strings which an FA will accept,
 * because by definition no accepted string passes through a dead state.
 * However, removing dead states slightly decreases the space required to store 
 * the FA, and can decrease the time needed to reject certain strings. This is
 * only called during the process of FA minimization.
 * 
 * @return an FA functionally equivalent to this one, wtih no dead states.
 */
std::unique_ptr<FA> FA::removeDeadStates()  const {

  const std::set<state_type> deadStates = findDeadStates();

  FABuilder faBuilder;

  auto isDead = [&deadStates] (const state_type& q) -> bool {

    return std::binary_search(std::begin(deadStates), std::end(deadStates), q);
  };

  faBuilder.initial_state(std::to_string(initial_state));

  /* If the initial_state is a dead state, then we're done. Nothing else
   * matters.
   */
  if (isDead(initial_state))

    return faBuilder.build();

  for (size_t i = 0; i < symbols.size(); ++i) {

    const symbol_type symbol = symbols[i];

    for (const std::pair<state_type, state_type>& pair : transitions[i]) {

      const state_type startState = pair.first;
      const state_type endState   = pair.second;

      if (!isDead(startState) and !isDead(endState))

        faBuilder.transition( std::to_string(startState), symbol, 
                              std::to_string(endState));
    }
  }

  for (const state_type& final_state : final_states) {

    if (!isDead(final_state))

      faBuilder.final_state(std::to_string(final_state));
  }

  return faBuilder.build();
}
