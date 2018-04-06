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

#ifndef RCL__ARGUMENTS_H_
#define RCL__ARGUMENTS_H_

#include <stddef.h>

#include "rcl/allocator.h"
#include "rcl/macros.h"
#include "rcl/types.h"
#include "rcl/visibility_control.h"

#if __cplusplus
extern "C"
{
#endif

typedef enum rcl_lexer_terminal_t
{
  // Indicates no valid terminal was found
  RCL_TERMINAL_NONE = 0,
  // Indicates end of input has been reached
  RCL_TERMINAL_EOF = 1,
  // ~/
  RCL_TERMINAL_TILDE_SLASH = 2,
  // rosservice://
  RCL_TERMINAL_URL_SERVICE = 3,
  // rostopic://
  RCL_TERMINAL_URL_TOPIC = 4,
  // :
  RCL_TERMINAL_COLON = 5,
  // __node
  RCL_TERMINAL_NODE = 6,
  // __ns
  RCL_TERMINAL_NS = 7,
  // :=
  RCL_TERMINAL_SEPARATOR = 8,
  // \1
  RCL_TERMINAL_BR1 = 9,
  // \2
  RCL_TERMINAL_BR2 = 10,
  // \3
  RCL_TERMINAL_BR3 = 11,
  // \4
  RCL_TERMINAL_BR4 = 12,
  // \5
  RCL_TERMINAL_BR5 = 13,
  // \6
  RCL_TERMINAL_BR6 = 14,
  // \7
  RCL_TERMINAL_BR7 = 15,
  // \8
  RCL_TERMINAL_BR8 = 16,
  // \9
  RCL_TERMINAL_BR9 = 17,
  // a name between slashes, must match (([a-zA-Z](_)?)|_)([0-9a-zA-Z](_)?)*
  RCL_TERMINAL_TOKEN = 18,
  // /
  RCL_TERMINAL_FORWARD_SLASH = 19,
  // *
  RCL_TERMINAL_WILD_ONE = 20,
  // **
  RCL_TERMINAL_WILD_MULTI = 21
} rcl_lexer_terminal_t;

// Analize string until one terminal is found
// If the string does not begin with a valid terminal, terminal will be RCL_TERMINAL_NONE
// If the first character is '\0', the terminal will be RCL_TERMINAL_EOF
rcl_ret_t
rcl_lexer_analyze(
  const char * text,
  rcl_lexer_terminal_t * terminal,
  size_t * length);

#if __cplusplus
}
#endif

#endif  // RCL__LEXER_H_
