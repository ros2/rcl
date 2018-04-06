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

#ifndef RCL__LEXER_H_
#define RCL__LEXER_H_

#include <stddef.h>

#include "rcl/allocator.h"
#include "rcl/macros.h"
#include "rcl/types.h"
#include "rcl/visibility_control.h"

#if __cplusplus
extern "C"
{
#endif

typedef enum rcl_lexeme_t
{
  // Indicates no valid lexeme was found
  RCL_LEXEME_NONE = 0,
  // Indicates end of input has been reached
  RCL_LEXEME_EOF = 1,
  // ~/
  RCL_LEXEME_TILDE_SLASH = 2,
  // rosservice://
  RCL_LEXEME_URL_SERVICE = 3,
  // rostopic://
  RCL_LEXEME_URL_TOPIC = 4,
  // :
  RCL_LEXEME_COLON = 5,
  // __node
  RCL_LEXEME_NODE = 6,
  // __ns
  RCL_LEXEME_NS = 7,
  // :=
  RCL_LEXEME_SEPARATOR = 8,
  // \1
  RCL_LEXEME_BR1 = 9,
  // \2
  RCL_LEXEME_BR2 = 10,
  // \3
  RCL_LEXEME_BR3 = 11,
  // \4
  RCL_LEXEME_BR4 = 12,
  // \5
  RCL_LEXEME_BR5 = 13,
  // \6
  RCL_LEXEME_BR6 = 14,
  // \7
  RCL_LEXEME_BR7 = 15,
  // \8
  RCL_LEXEME_BR8 = 16,
  // \9
  RCL_LEXEME_BR9 = 17,
  // a name between slashes, must match (([a-zA-Z](_)?)|_)([0-9a-zA-Z](_)?)*
  RCL_LEXEME_TOKEN = 18,
  // /
  RCL_LEXEME_FORWARD_SLASH = 19,
  // *
  RCL_LEXEME_WILD_ONE = 20,
  // **
  RCL_LEXEME_WILD_MULTI = 21
} rcl_lexeme_t;

// Analize string until one lexeme is found
// If the string does not begin with a valid lexeme, lexeme will be RCL_LEXEME_NONE
// If the first character is '\0', the lexeme will be RCL_LEXEME_EOF
RCL_PUBLIC
RCL_WARN_UNUSED
rcl_ret_t
rcl_lexer_analyze(
  const char * text,
  rcl_lexeme_t * lexeme,
  size_t * length,
  rcl_allocator_t allocator);

#if __cplusplus
}
#endif

#endif  // RCL__LEXER_H_
