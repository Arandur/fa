#include "FA.h"
#include "FABuilder.h"

#include <cstdio>
#include <string>

int main() {

  std::unique_ptr<FA> fa = FA::fromRegex("(a|b)*abb");

  for (const std::string& str : {

    "",
    "a", "b",
    "aa", "ab", "ba", "bb",
    "aaa", "aab", "aba", "abb", "baa", "bab", "bba", "bbb",
    "abababb"
  }) {

    printf((fa->match(&*std::begin(str), &*std::end(str)) ? 
      "'%s' matches!\n" : 
      "'%s' doesn't match.\n"
      ), str.c_str());
  }

  return 0;
}
