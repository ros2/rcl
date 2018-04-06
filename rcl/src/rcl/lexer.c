// Copyright 2018 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "rcl/lexer.h"

/* The lexer tries to find a lexeme in a string.
 * It looks at one character at a time, and uses that character's value to decide how to transition
 * a state machine.
 * A transition is taken if a character's ASCII value falls within its range.
 * No transition's ranges overlap; there is never more than one matching transition.
 *
 * If no transition matches then the state machine takes an '<else,M>' transition.
 * Every state has exactly one '<else,M>' transition.
 * All states have an `<else,0>` to T_NONE unless otherwise specified in the diagram below.
 *
 * When a transition is taken it causes the lexer to move to another character in the string.
 * Normal transitions always move the lexer to the forwards one character.
 * '<else,M>' transitions may cause the lexer to move forwards 1, or backwards N.
 * The movement M is written as M = 1 - N so it can be stored in an unsigned integer.
 * For example, an `<else>` transition with M = 0 moves the lexer forwards 1 character, M = 1 keeps
 * the lexer at the current character, and M = 2 moves the lexer backwards one character.
 *
 *
 * dot graph of state machine

digraph remapping_lexer {
  rankdir=LR;
  node [shape = box, fontsize = 7];
    T_TILDE_SLASH
    T_URL_SERVICE
    T_URL_TOPIC
    T_COLON
    T_NODE
    T_NS
    T_SEPARATOR
    T_BR1
    T_BR2
    T_BR3
    T_BR4
    T_BR5
    T_BR6
    T_BR7
    T_BR8
    T_BR9
    T_TOKEN
    T_FORWARD_SLASH
    T_WILD_ONE
    T_WILD_MULTI
    T_EOF
    T_NONE
  node [shape = circle];
  S0 -> T_FORWARD_SLASH [ label = "/"];
  S0 -> S1 [ label = "\\"];
  S0 -> S2 [ label = "~"];
  S0 -> S3 [ label = "_" ];
  S0 -> S8 [ label = "a-qs-zA-Z"];
  S0 -> S10 [ label = "r"];
  S0 -> S29 [ label = "*"];
  S0 -> S30 [ label = ":"];
  S1 -> T_BR1 [ label = "1"];
  S1 -> T_BR2 [ label = "2"];
  S1 -> T_BR3 [ label = "3"];
  S1 -> T_BR4 [ label = "4"];
  S1 -> T_BR5 [ label = "5"];
  S1 -> T_BR6 [ label = "6"];
  S1 -> T_BR7 [ label = "7"];
  S1 -> T_BR8 [ label = "8"];
  S1 -> T_BR9 [ label = "9"];
  S2 -> T_TILDE_SLASH [ label ="/" ];
  S3 -> S4 [ label = "_" ];
  S3 -> S9 [ label = "<else,1>", color = crimson, fontcolor = crimson];
  S4 -> S5 [ label = "n" ];
  S5 -> T_NS [ label = "s"];
  S5 -> S6 [ label = "o" ];
  S6 -> S7 [ label = "d" ];
  S7 -> T_NODE [ label = "e"];
  S8 -> T_TOKEN [ label = "<else,1>", color=crimson, fontcolor=crimson];
  S8 -> S8 [ label = "a-zA-Z0-9"];
  S8 -> S9 [ label = "_"];
  S9 -> T_TOKEN [ label = "<else,1>", color=crimson, fontcolor=crimson];
  S9 -> S8 [ label = "a-zA-Z0-9"];
  S10 -> S8 [ label = "<else,1>", color=crimson, fontcolor=crimson];
  S10 -> S11 [ label = "o"];
  S11 -> S8 [ label = "<else,1>", color=crimson, fontcolor=crimson];
  S11 -> S12 [ label = "s"];
  S12 -> S8 [ label = "<else,1>", color=crimson, fontcolor=crimson];
  S12 -> S13 [ label = "t"];
  S12 -> S20 [ label = "s"];
  S13 -> S8 [ label = "<else,1>", color=crimson, fontcolor=crimson];
  S13 -> S14 [ label = "o"];
  S14 -> S8 [ label = "<else,1>", color=crimson, fontcolor=crimson];
  S14 -> S15 [ label = "p"];
  S15 -> S8 [ label = "<else,1>", color=crimson, fontcolor=crimson];
  S15 -> S16 [ label = "i"];
  S16 -> S8 [ label = "<else,1>", color=crimson, fontcolor=crimson];
  S16 -> S17 [ label = "c"];
  S17 -> S8 [ label = "<else,1>", color=crimson, fontcolor=crimson];
  S17 -> S18 [ label = ":"];
  S18 -> S19 [ label = "/"];
  S18 -> S8 [ label = "<else,2>", color=crimson, fontcolor=crimson];
  S19 -> T_URL_TOPIC [ label = "/"];
  S19 -> S8 [ label = "<else,3>", color=crimson, fontcolor=crimson];
  S20 -> S8 [ label = "<else,1>", color=crimson, fontcolor=crimson];
  S20 -> S21 [ label = "e"];
  S21 -> S8 [ label = "<else,1>", color=crimson, fontcolor=crimson];
  S21 -> S22 [ label = "r"];
  S22 -> S8 [ label = "<else,1>", color=crimson, fontcolor=crimson];
  S22 -> S23 [ label = "v"];
  S23 -> S8 [ label = "<else,1>", color=crimson, fontcolor=crimson];
  S23 -> S24 [ label = "i"];
  S24 -> S8 [ label = "<else,1>", color=crimson, fontcolor=crimson];
  S24 -> S25 [ label = "c"];
  S25 -> S8 [ label = "<else,1>", color=crimson, fontcolor=crimson];
  S25 -> S26 [ label = "e"];
  S26 -> S27 [ label = ":"];
  S26 -> S8 [ label = "<else,1>", color=crimson, fontcolor=crimson];
  S27 -> S28 [ label = "/"];
  S27 -> S8 [ label = "<else,2>", color=crimson, fontcolor=crimson];
  S28 -> T_URL_SERVICE [ label = "/"];
  S28 -> S8 [ label = "<else,3>", color=crimson, fontcolor=crimson];
  S29 -> T_WILD_MULTI[ label = "*"];
  S29 -> T_WILD_ONE [ label = "<else,1>", color=crimson, fontcolor=crimson];
  S30 -> T_SEPARATOR [ label = "="];
  S30 -> T_COLON [ label = "<else,1>", color=crimson, fontcolor=crimson];
}
*/


// Represents a transition from one state to another
typedef struct rcl_lexer_transition_t
{
  // Index of a state to transition to
  const size_t to_state;
  // Start of a range of chars (inclusive) which activates this transition
  const char range_start;
  // End of a range of chars (inclusive) which activates this transition
  const char range_end;
} rcl_lexer_transition_t;

// Represents a state that either has transitions or is terminal
typedef struct rcl_lexer_state_t
{
  // If no transition matches this causes a character to be analyzed a second time in another state
  const size_t else_state;
  // Movement associated with taking else state
  const size_t else_movement;
  // A value of a terminal if this is a terminal state
  const rcl_lexer_terminal_t terminal;
  // Transitions in the state machine (NULL value at end of array)
  const rcl_lexer_transition_t transitions[11];
} rcl_lexer_state_t;

#define S0 0u
#define S1 1u
#define S2 2u
#define S3 3u
#define S4 4u
#define S5 5u
#define S6 6u
#define S7 7u
#define S8 8u
#define S9 9u
#define S10 10u
#define S11 11u
#define S12 12u
#define S13 13u
#define S14 14u
#define S15 15u
#define S16 16u
#define S17 17u
#define S18 18u
#define S19 19u
#define S20 20u
#define S21 21u
#define S22 22u
#define S23 23u
#define S24 24u
#define S25 25u
#define S26 26u
#define S27 27u
#define S28 28u
#define S29 29u
#define S30 30u

#define T_TILDE_SLASH 31u
#define T_URL_SERVICE 32u
#define T_URL_TOPIC 33u
#define T_COLON 34u
#define T_NODE 35u
#define T_NS 36u
#define T_SEPARATOR 37u
#define T_BR1 38u
#define T_BR2 39u
#define T_BR3 40u
#define T_BR4 41u
#define T_BR5 42u
#define T_BR6 43u
#define T_BR7 44u
#define T_BR8 45u
#define T_BR9 46u
#define T_TOKEN 47u
#define T_FORWARD_SLASH 48u
#define T_WILD_ONE 49u
#define T_WILD_MULTI 50u
#define T_EOF 51u
#define T_NONE 52u

// used to figure out if a state is terminal or not
#define FIRST_TERMINAL T_TILDE_SLASH

#define END_TRANSITIONS {0, '\0', '\0'}

static const rcl_lexer_state_t g_states[] =
{
  // Nonterminal states
  // S0
  {
    T_NONE,
    0,
    RCL_TERMINAL_NONE,
    {
      {T_FORWARD_SLASH, '/', '/'},
      {S1, '\\', '\\'},
      {S2, '~', '~'},
      {S3, '_', '_'},
      {S8, 'a', 'q'},
      {S8, 's', 'z'},
      {S8, 'A', 'Z'},
      {S10, 'r', 'r'},
      {S29, '*', '*'},
      {S30, ':', ':'},
      END_TRANSITIONS
    }
  },
  // S1
  {
    T_NONE,
    0,
    RCL_TERMINAL_NONE,
    {
      {T_BR1, '1', '1'},
      {T_BR2, '2', '2'},
      {T_BR3, '3', '3'},
      {T_BR4, '4', '4'},
      {T_BR5, '5', '5'},
      {T_BR6, '6', '6'},
      {T_BR7, '7', '7'},
      {T_BR8, '8', '8'},
      {T_BR9, '9', '9'},
      END_TRANSITIONS
    }
  },
  // S2
  {
    T_NONE,
    0,
    RCL_TERMINAL_NONE,
    {
      {T_TILDE_SLASH, '/', '/'},
      END_TRANSITIONS
    }
  },
  // S3
  {
    S9,
    1,
    RCL_TERMINAL_NONE,
    {
      {S4, '_', '_'},
      END_TRANSITIONS
    }
  },
  // S4
  {
    T_NONE,
    0,
    RCL_TERMINAL_NONE,
    {
      {S5, 'n', 'n'},
      END_TRANSITIONS
    }
  },
  // S5
  {
    T_NONE,
    0,
    RCL_TERMINAL_NONE,
    {
      {T_NS, 's', 's'},
      {S6, 'o', 'o'},
      END_TRANSITIONS
    }
  },
  // S6
  {
    T_NONE,
    0,
    RCL_TERMINAL_NONE,
    {
      {S7, 'd', 'd'},
      END_TRANSITIONS
    }
  },
  // S7
  {
    T_NONE,
    0,
    RCL_TERMINAL_NONE,
    {
      {T_NODE, 'e', 'e'},
      END_TRANSITIONS
    }
  },
  // S8
  {
    T_TOKEN,
    1,
    RCL_TERMINAL_NONE,
    {
      {S8, 'a', 'z'},
      {S8, 'A', 'Z'},
      {S8, '0', '9'},
      {S9, '_', '_'},
      END_TRANSITIONS
    }
  },
  // S9
  {
    T_TOKEN,
    1,
    RCL_TERMINAL_NONE,
    {
      {S8, 'a', 'z'},
      {S8, 'A', 'Z'},
      {S8, '0', '9'},
      END_TRANSITIONS
    }
  },
  // S10
  {
    S8,
    1,
    RCL_TERMINAL_NONE,
    {
      {S11, 'o', 'o'},
      END_TRANSITIONS
    }
  },
  // S11
  {
    S8,
    1,
    RCL_TERMINAL_NONE,
    {
      {S12, 's', 's'},
      END_TRANSITIONS
    }
  },
  // S12
  {
    S8,
    1,
    RCL_TERMINAL_NONE,
    {
      {S13, 't', 't'},
      {S20, 's', 's'},
      END_TRANSITIONS
    }
  },
  // S13
  {
    S8,
    1,
    RCL_TERMINAL_NONE,
    {
      {S14, 'o', 'o'},
      END_TRANSITIONS
    }
  },
  // S14
  {
    S8,
    1,
    RCL_TERMINAL_NONE,
    {
      {S15, 'p', 'p'},
      END_TRANSITIONS
    }
  },
  // S15
  {
    S8,
    1,
    RCL_TERMINAL_NONE,
    {
      {S16, 'i', 'i'},
      END_TRANSITIONS
    }
  },
  // S16
  {
    S8,
    1,
    RCL_TERMINAL_NONE,
    {
      {S17, 'c', 'c'},
      END_TRANSITIONS
    }
  },
  // S17
  {
    S8,
    1,
    RCL_TERMINAL_NONE,
    {
      {S18, ':', ':'},
      END_TRANSITIONS
    }
  },
  // S18
  {
    S8,
    2,
    RCL_TERMINAL_NONE,
    {
      {S19, '/', '/'},
      END_TRANSITIONS
    }
  },
  // S19
  {
    S8,
    3,
    RCL_TERMINAL_NONE,
    {
      {T_URL_TOPIC, '/', '/'},
      END_TRANSITIONS
    }
  },
  // S20
  {
    S8,
    1,
    RCL_TERMINAL_NONE,
    {
      {S21, 'e', 'e'},
      END_TRANSITIONS
    }
  },
  // S21
  {
    S8,
    1,
    RCL_TERMINAL_NONE,
    {
      {S22, 'r', 'r'},
      END_TRANSITIONS
    }
  },
  // S22
  {
    S8,
    1,
    RCL_TERMINAL_NONE,
    {
      {S23, 'v', 'v'},
      END_TRANSITIONS
    }
  },
  // S23
  {
    S8,
    1,
    RCL_TERMINAL_NONE,
    {
      {S24, 'i', 'i'},
      END_TRANSITIONS
    }
  },
  // S24
  {
    S8,
    1,
    RCL_TERMINAL_NONE,
    {
      {S25, 'c', 'c'},
      END_TRANSITIONS
    }
  },
  // S25
  {
    S8,
    1,
    RCL_TERMINAL_NONE,
    {
      {S26, 'e', 'e'},
      END_TRANSITIONS
    }
  },
  // S26
  {
    S8,
    1,
    RCL_TERMINAL_NONE,
    {
      {S27, ':', ':'},
      END_TRANSITIONS
    }
  },
  // S27
  {
    S8,
    2,
    RCL_TERMINAL_NONE,
    {
      {S28, '/', '/'},
      END_TRANSITIONS
    }
  },
  // S28
  {
    S8,
    3,
    RCL_TERMINAL_NONE,
    {
      {T_URL_SERVICE, '/', '/'},
      END_TRANSITIONS
    }
  },
  // S29
  {
    T_WILD_ONE,
    1,
    RCL_TERMINAL_NONE,
    {
      {T_WILD_MULTI, '*', '*'},
      END_TRANSITIONS
    }
  },
  // S30
  {
    T_COLON,
    1,
    RCL_TERMINAL_NONE,
    {
      {T_SEPARATOR, '=', '='},
      END_TRANSITIONS
    }
  },
  // Terminal states
  // 31
  {S0, 0, RCL_TERMINAL_TILDE_SLASH, {END_TRANSITIONS}},
  // 32
  {S0, 0, RCL_TERMINAL_URL_SERVICE, {END_TRANSITIONS}},
  // 33
  {S0, 0, RCL_TERMINAL_URL_TOPIC, {END_TRANSITIONS}},
  // 34
  {S0, 0, RCL_TERMINAL_COLON, {END_TRANSITIONS}},
  // 35
  {S0, 0, RCL_TERMINAL_NODE, {END_TRANSITIONS}},
  // 36
  {S0, 0, RCL_TERMINAL_NS, {END_TRANSITIONS}},
  // 37
  {S0, 0, RCL_TERMINAL_SEPARATOR, {END_TRANSITIONS}},
  // 38
  {S0, 0, RCL_TERMINAL_BR1, {END_TRANSITIONS}},
  // 39
  {S0, 0, RCL_TERMINAL_BR2, {END_TRANSITIONS}},
  // 40
  {S0, 0, RCL_TERMINAL_BR3, {END_TRANSITIONS}},
  // 41
  {S0, 0, RCL_TERMINAL_BR4, {END_TRANSITIONS}},
  // 42
  {S0, 0, RCL_TERMINAL_BR5, {END_TRANSITIONS}},
  // 43
  {S0, 0, RCL_TERMINAL_BR6, {END_TRANSITIONS}},
  // 44
  {S0, 0, RCL_TERMINAL_BR7, {END_TRANSITIONS}},
  // 45
  {S0, 0, RCL_TERMINAL_BR8, {END_TRANSITIONS}},
  // 46
  {S0, 0, RCL_TERMINAL_BR9, {END_TRANSITIONS}},
  // 47
  {S0, 0, RCL_TERMINAL_TOKEN, {END_TRANSITIONS}},
  // 48
  {S0, 0, RCL_TERMINAL_FORWARD_SLASH, {END_TRANSITIONS}},
  // 49
  {S0, 0, RCL_TERMINAL_WILD_ONE, {END_TRANSITIONS}},
  // 50
  {S0, 0, RCL_TERMINAL_WILD_MULTI, {END_TRANSITIONS}},
  // 51
  {S0, 0, RCL_TERMINAL_EOF, {END_TRANSITIONS}},
  // 52
  {S0, 0, RCL_TERMINAL_NONE, {END_TRANSITIONS}},
};

rcl_ret_t
rcl_lexer_analyze(
  const char * text,
  rcl_lexer_terminal_t * terminal,
  size_t * length)
{
  // TODO(sloretz) accept allocator just for error checking
  // RCL_CHECK_ARGUMENT_FOR_NULL(text, RCL_RET_INVALID_ARGUMENT, allocator);
  // RCL_CHECK_ARGUMENT_FOR_NULL(terminal, RCL_RET_INVALID_ARGUMENT, allocator);
  // RCL_CHECK_ARGUMENT_FOR_NULL(length, RCL_RET_INVALID_ARGUMENT, allocator);

  *length = 0;

  if ('\0' == text[0]) {
    // Early exit if string is empty
    *terminal = RCL_TERMINAL_EOF;
    return RCL_RET_OK;
  }

  const rcl_lexer_state_t * state = &(g_states[S0]);
  char current_char;
  size_t next_state;
  size_t movement;
  do {
    current_char = text[*length];
    next_state = 0;
    movement = 0;

    // Loop through all transitions in current state and find one that matches
    size_t transition_idx = 0;
    const rcl_lexer_transition_t * transition;
    do {
      transition = &(state->transitions[transition_idx]);
      if (transition->range_start <= current_char && transition->range_end >= current_char) {
        next_state = transition->to_state;
        break;
      }
      ++transition_idx;
    } while (transition->to_state != 0);

    if (0 == next_state) {
      // no transition found, take the else transition
      next_state = state->else_state;
      movement = state->else_movement;
    }

    if (0 == movement) {
      // Advance position in string to test next char
      ++(*length);
    } else {
      // Go backwards in string
      *length -= movement - 1;
      // TODO(sloretz) Error if movement would cause length to overflow
    }

    state = &(g_states[next_state]);
  } while (next_state < FIRST_TERMINAL);

  *terminal = state->terminal;
  return RCL_RET_OK;
}
