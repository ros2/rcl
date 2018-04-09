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

#include "rcl/error_handling.h"
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
 * The movement M is written as M = 1 + N so it can be stored in an unsigned integer.
 * For example, an `<else>` transition with M = 0 moves the lexer forwards 1 character, M = 1 keeps
 * the lexer at the current character, and M = 2 moves the lexer backwards one character.

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
  const unsigned char to_state;
  // Start of a range of chars (inclusive) which activates this transition
  const char range_start;
  // End of a range of chars (inclusive) which activates this transition
  const char range_end;
} rcl_lexer_transition_t;

// Represents a non-terminal state
typedef struct rcl_lexer_state_t
{
  // If no transition matches this causes a character to be analyzed a second time in another state
  const unsigned char else_state;
  // Movement associated with taking else state
  const unsigned char else_movement;
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
#define LAST_STATE S30

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
#define LAST_TERMINAL T_NONE

// Used to mark where the last transition is in a state
#define END_TRANSITIONS {0, '\0', '\0'}

static const rcl_lexer_state_t g_states[LAST_STATE + 1] =
{
  // S0
  {
    T_NONE,
    0u,
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
    0u,
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
    0u,
    {
      {T_TILDE_SLASH, '/', '/'},
      END_TRANSITIONS
    }
  },
  // S3
  {
    S9,
    1u,
    {
      {S4, '_', '_'},
      END_TRANSITIONS
    }
  },
  // S4
  {
    T_NONE,
    0u,
    {
      {S5, 'n', 'n'},
      END_TRANSITIONS
    }
  },
  // S5
  {
    T_NONE,
    0u,
    {
      {T_NS, 's', 's'},
      {S6, 'o', 'o'},
      END_TRANSITIONS
    }
  },
  // S6
  {
    T_NONE,
    0u,
    {
      {S7, 'd', 'd'},
      END_TRANSITIONS
    }
  },
  // S7
  {
    T_NONE,
    0u,
    {
      {T_NODE, 'e', 'e'},
      END_TRANSITIONS
    }
  },
  // S8
  {
    T_TOKEN,
    1u,
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
    1u,
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
    1u,
    {
      {S11, 'o', 'o'},
      END_TRANSITIONS
    }
  },
  // S11
  {
    S8,
    1u,
    {
      {S12, 's', 's'},
      END_TRANSITIONS
    }
  },
  // S12
  {
    S8,
    1u,
    {
      {S13, 't', 't'},
      {S20, 's', 's'},
      END_TRANSITIONS
    }
  },
  // S13
  {
    S8,
    1u,
    {
      {S14, 'o', 'o'},
      END_TRANSITIONS
    }
  },
  // S14
  {
    S8,
    1u,
    {
      {S15, 'p', 'p'},
      END_TRANSITIONS
    }
  },
  // S15
  {
    S8,
    1u,
    {
      {S16, 'i', 'i'},
      END_TRANSITIONS
    }
  },
  // S16
  {
    S8,
    1u,
    {
      {S17, 'c', 'c'},
      END_TRANSITIONS
    }
  },
  // S17
  {
    S8,
    1u,
    {
      {S18, ':', ':'},
      END_TRANSITIONS
    }
  },
  // S18
  {
    S8,
    2u,
    {
      {S19, '/', '/'},
      END_TRANSITIONS
    }
  },
  // S19
  {
    S8,
    3u,
    {
      {T_URL_TOPIC, '/', '/'},
      END_TRANSITIONS
    }
  },
  // S20
  {
    S8,
    1u,
    {
      {S21, 'e', 'e'},
      END_TRANSITIONS
    }
  },
  // S21
  {
    S8,
    1u,
    {
      {S22, 'r', 'r'},
      END_TRANSITIONS
    }
  },
  // S22
  {
    S8,
    1u,
    {
      {S23, 'v', 'v'},
      END_TRANSITIONS
    }
  },
  // S23
  {
    S8,
    1u,
    {
      {S24, 'i', 'i'},
      END_TRANSITIONS
    }
  },
  // S24
  {
    S8,
    1u,
    {
      {S25, 'c', 'c'},
      END_TRANSITIONS
    }
  },
  // S25
  {
    S8,
    1u,
    {
      {S26, 'e', 'e'},
      END_TRANSITIONS
    }
  },
  // S26
  {
    S8,
    1u,
    {
      {S27, ':', ':'},
      END_TRANSITIONS
    }
  },
  // S27
  {
    S8,
    2u,
    {
      {S28, '/', '/'},
      END_TRANSITIONS
    }
  },
  // S28
  {
    S8,
    3u,
    {
      {T_URL_SERVICE, '/', '/'},
      END_TRANSITIONS
    }
  },
  // S29
  {
    T_WILD_ONE,
    1u,
    {
      {T_WILD_MULTI, '*', '*'},
      END_TRANSITIONS
    }
  },
  // S30
  {
    T_COLON,
    1u,
    {
      {T_SEPARATOR, '=', '='},
      END_TRANSITIONS
    }
  },
};

static const rcl_lexeme_t g_terminals[LAST_TERMINAL + 1] = {
  // 0
  RCL_LEXEME_TILDE_SLASH,
  // 1
  RCL_LEXEME_URL_SERVICE,
  // 2
  RCL_LEXEME_URL_TOPIC,
  // 3
  RCL_LEXEME_COLON,
  // 4
  RCL_LEXEME_NODE,
  // 5
  RCL_LEXEME_NS,
  // 6
  RCL_LEXEME_SEPARATOR,
  // 7
  RCL_LEXEME_BR1,
  // 8
  RCL_LEXEME_BR2,
  // 9
  RCL_LEXEME_BR3,
  // 10
  RCL_LEXEME_BR4,
  // 11
  RCL_LEXEME_BR5,
  // 12
  RCL_LEXEME_BR6,
  // 13
  RCL_LEXEME_BR7,
  // 14
  RCL_LEXEME_BR8,
  // 15
  RCL_LEXEME_BR9,
  // 16
  RCL_LEXEME_TOKEN,
  // 17
  RCL_LEXEME_FORWARD_SLASH,
  // 18
  RCL_LEXEME_WILD_ONE,
  // 19
  RCL_LEXEME_WILD_MULTI,
  // 20
  RCL_LEXEME_EOF,
  // 21
  RCL_LEXEME_NONE,
};

rcl_ret_t
rcl_lexer_analyze(
  const char * text,
  rcl_allocator_t alloc,
  rcl_lexeme_t * lexeme,
  size_t * length)
{
  RCL_CHECK_ALLOCATOR_WITH_MSG(&alloc, "invalid allocator", return RCL_RET_INVALID_ARGUMENT);
  RCL_CHECK_ARGUMENT_FOR_NULL(text, RCL_RET_INVALID_ARGUMENT, alloc);
  RCL_CHECK_ARGUMENT_FOR_NULL(lexeme, RCL_RET_INVALID_ARGUMENT, alloc);
  RCL_CHECK_ARGUMENT_FOR_NULL(length, RCL_RET_INVALID_ARGUMENT, alloc);

  *length = 0;

  if ('\0' == text[0]) {
    // Early exit if string is empty
    *lexeme = RCL_LEXEME_EOF;
    return RCL_RET_OK;
  }

  const rcl_lexer_state_t * state;
  char current_char;
  size_t next_state = S0;
  size_t movement;

  // Analyze one character at a time until lexeme is found
  do {
    if (next_state > LAST_STATE) {
      RCL_SET_ERROR_MSG("Internal lexer bug: next state does not exist", alloc);
      return RCL_RET_ERROR;
    }
    state = &(g_states[next_state]);
    current_char = text[*length];
    next_state = 0;
    movement = 0;

    // Look for a transition that contains this character in its range
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

    // if no transition was found, take the else transition
    if (0 == next_state) {
      next_state = state->else_state;
      movement = state->else_movement;
    }

    // Move the lexer to another character in the string
    if (0 == movement) {
      // Go forwards 1 char
      ++(*length);
    } else {
      // Go backwards N chars
      if (movement - 1 > *length) {
        RCL_SET_ERROR_MSG("Internal lexer bug: movement would read before start of string", alloc);
        return RCL_RET_ERROR;
      }
      *length -= movement - 1;
    }
  } while (next_state < FIRST_TERMINAL);

  if (FIRST_TERMINAL > next_state || next_state - FIRST_TERMINAL > LAST_TERMINAL) {
    RCL_SET_ERROR_MSG("Internal lexer bug: terminal state does not exist", alloc);
    return RCL_RET_ERROR;
  }
  *lexeme = g_terminals[next_state - FIRST_TERMINAL];
  return RCL_RET_OK;
}
