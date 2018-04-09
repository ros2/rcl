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

#include <gtest/gtest.h>

#include <string>

#include "../scope_exit.hpp"

#include "rcl/error_handling.h"
#include "rcl/lexer_lookahead.h"

#ifdef RMW_IMPLEMENTATION
# define CLASSNAME_(NAME, SUFFIX) NAME ## __ ## SUFFIX
# define CLASSNAME(NAME, SUFFIX) CLASSNAME_(NAME, SUFFIX)
#else
# define CLASSNAME(NAME, SUFFIX) NAME
#endif

class CLASSNAME (TestLexerLookaheadFixture, RMW_IMPLEMENTATION) : public ::testing::Test
{
public:
  void SetUp()
  {
  }

  void TearDown()
  {
  }
};

#define SCOPE_LOOKAHEAD2(name, text) \
  { \
    name = rcl_get_zero_initialized_lexer_lookahead2(); \
    rcl_ret_t ret = rcl_lexer_lookahead2_init(&name, text, rcl_get_default_allocator()); \
    ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe(); \
  } \
  auto __scope_lookahead2_ ## name = make_scope_exit( \
    [&name]() { \
      rcl_ret_t ret = rcl_lexer_lookahead2_fini(&buffer); \
      ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe(); \
    })

TEST_F(CLASSNAME(TestLexerLookaheadFixture, RMW_IMPLEMENTATION), test_init_fini_twice)
{
  rcl_lexer_lookahead2_t buffer = rcl_get_zero_initialized_lexer_lookahead2();
  rcl_ret_t ret = rcl_lexer_lookahead2_init(&buffer, "foobar", rcl_get_default_allocator());
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  ret = rcl_lexer_lookahead2_fini(&buffer);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();

  ret = rcl_lexer_lookahead2_fini(&buffer);
  EXPECT_EQ(RCL_RET_INVALID_ARGUMENT, ret);
  rcl_reset_error();
}

TEST_F(CLASSNAME(TestLexerLookaheadFixture, RMW_IMPLEMENTATION), test_peek)
{
  rcl_ret_t ret;
  rcl_lexer_lookahead2_t buffer;
  SCOPE_LOOKAHEAD2(buffer, "foobar");

  rcl_lexeme_t lexeme = RCL_LEXEME_NONE;

  ret = rcl_lexer_lookahead2_peek(&buffer, &lexeme);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_EQ(RCL_LEXEME_TOKEN, lexeme);

  // Test again to make sure peek isn't advancing the lexer
  lexeme = RCL_LEXEME_NONE;
  ret = rcl_lexer_lookahead2_peek(&buffer, &lexeme);
  EXPECT_EQ(RCL_RET_OK, ret);
  EXPECT_EQ(RCL_LEXEME_TOKEN, lexeme);
}

TEST_F(CLASSNAME(TestLexerLookaheadFixture, RMW_IMPLEMENTATION), test_peek2)
{
  rcl_ret_t ret;
  rcl_lexer_lookahead2_t buffer;
  SCOPE_LOOKAHEAD2(buffer, "foobar/");

  rcl_lexeme_t lexeme1 = RCL_LEXEME_NONE;
  rcl_lexeme_t lexeme2 = RCL_LEXEME_NONE;

  ret = rcl_lexer_lookahead2_peek2(&buffer, &lexeme1, &lexeme2);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_EQ(RCL_LEXEME_TOKEN, lexeme1);
  EXPECT_EQ(RCL_LEXEME_FORWARD_SLASH, lexeme2);

  // Test again to make sure peek2 isn't advancing the lexer
  lexeme1 = RCL_LEXEME_NONE;
  lexeme2 = RCL_LEXEME_NONE;
  ret = rcl_lexer_lookahead2_peek2(&buffer, &lexeme1, &lexeme2);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_EQ(RCL_LEXEME_TOKEN, lexeme1);
  EXPECT_EQ(RCL_LEXEME_FORWARD_SLASH, lexeme2);
}

TEST_F(CLASSNAME(TestLexerLookaheadFixture, RMW_IMPLEMENTATION), test_eof)
{
  rcl_ret_t ret;
  rcl_lexer_lookahead2_t buffer;
  SCOPE_LOOKAHEAD2(buffer, "");

  {
    rcl_lexeme_t lexeme = RCL_LEXEME_NONE;
    ret = rcl_lexer_lookahead2_peek(&buffer, &lexeme);
    EXPECT_EQ(RCL_RET_OK, ret);
    EXPECT_EQ(RCL_LEXEME_EOF, lexeme);
  }
  {
    rcl_lexeme_t lexeme1 = RCL_LEXEME_NONE;
    rcl_lexeme_t lexeme2 = RCL_LEXEME_NONE;
    ret = rcl_lexer_lookahead2_peek2(&buffer, &lexeme1, &lexeme2);
    EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
    EXPECT_EQ(RCL_LEXEME_EOF, lexeme1);
    EXPECT_EQ(RCL_LEXEME_EOF, lexeme2);
  }
  // Accepting keeps the lexer at EOF
  {
    EXPECT_EQ(RCL_RET_OK, rcl_lexer_lookahead2_accept(&buffer, NULL, NULL));
    rcl_lexeme_t lexeme = RCL_LEXEME_NONE;
    ret = rcl_lexer_lookahead2_peek(&buffer, &lexeme);
    EXPECT_EQ(RCL_RET_OK, ret);
    EXPECT_EQ(RCL_LEXEME_EOF, lexeme);
  }
}


TEST_F(CLASSNAME(TestLexerLookaheadFixture, RMW_IMPLEMENTATION), test_accept)
{
  rcl_ret_t ret;
  rcl_lexer_lookahead2_t buffer;
  SCOPE_LOOKAHEAD2(buffer, "foobar/");

  rcl_lexeme_t lexeme = RCL_LEXEME_NONE;
  const char * lexeme_text;
  size_t lexeme_text_length;

  // Peek token
  ret = rcl_lexer_lookahead2_peek(&buffer, &lexeme);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_EQ(RCL_LEXEME_TOKEN, lexeme);

  // accept token
  ret = rcl_lexer_lookahead2_accept(&buffer, &lexeme_text, &lexeme_text_length);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_STREQ("foobar", std::string(lexeme_text, lexeme_text_length).c_str());

  // peek forward slash
  ret = rcl_lexer_lookahead2_peek(&buffer, &lexeme);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_EQ(RCL_LEXEME_FORWARD_SLASH, lexeme);

  // accept forward slash
  ret = rcl_lexer_lookahead2_accept(&buffer, &lexeme_text, &lexeme_text_length);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_STREQ("/", std::string(lexeme_text, lexeme_text_length).c_str());

  // peek eof
  ret = rcl_lexer_lookahead2_peek(&buffer, &lexeme);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_EQ(RCL_LEXEME_EOF, lexeme);

  // accept eof
  ret = rcl_lexer_lookahead2_accept(&buffer, &lexeme_text, &lexeme_text_length);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_STREQ("", std::string(lexeme_text, lexeme_text_length).c_str());

  // peek eof again
  ret = rcl_lexer_lookahead2_peek(&buffer, &lexeme);
  EXPECT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_EQ(RCL_LEXEME_EOF, lexeme);
}

TEST_F(CLASSNAME(TestLexerLookaheadFixture, RMW_IMPLEMENTATION), test_expect)
{
  rcl_ret_t ret;
  rcl_lexer_lookahead2_t buffer;
  SCOPE_LOOKAHEAD2(buffer, "node_name:__node:=new_1");
  const char * lexeme_text;
  size_t lexeme_text_length;

  ret = rcl_lexer_lookahead2_expect(&buffer, RCL_LEXEME_TOKEN, &lexeme_text, &lexeme_text_length);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_STREQ("node_name", std::string(lexeme_text, lexeme_text_length).c_str());

  ret = rcl_lexer_lookahead2_expect(
    &buffer, RCL_LEXEME_FORWARD_SLASH, &lexeme_text, &lexeme_text_length);
  EXPECT_EQ(RCL_RET_WRONG_LEXEME, ret) << rcl_get_error_string_safe();
}

TEST_F(CLASSNAME(TestLexerLookaheadFixture, RMW_IMPLEMENTATION), test_lex_long_string)
{
  rcl_ret_t ret;
  rcl_lexer_lookahead2_t buffer;
  SCOPE_LOOKAHEAD2(buffer, ":\\1rostopic://\\2rosservice://~/\\8:=**:*foobar");
  const char * lexeme_text;
  size_t lexeme_text_length;
  rcl_lexeme_t lexeme;

  // :
  ret = rcl_lexer_lookahead2_peek(&buffer, &lexeme);
  EXPECT_EQ(RCL_LEXEME_COLON, lexeme);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  ret = rcl_lexer_lookahead2_accept(&buffer, &lexeme_text, &lexeme_text_length);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_STREQ(":", std::string(lexeme_text, lexeme_text_length).c_str());

  // \1
  ret = rcl_lexer_lookahead2_peek(&buffer, &lexeme);
  EXPECT_EQ(RCL_LEXEME_BR1, lexeme);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  ret = rcl_lexer_lookahead2_accept(&buffer, &lexeme_text, &lexeme_text_length);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_STREQ("\\1", std::string(lexeme_text, lexeme_text_length).c_str());

  // rostopic://
  ret = rcl_lexer_lookahead2_peek(&buffer, &lexeme);
  EXPECT_EQ(RCL_LEXEME_URL_TOPIC, lexeme);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  ret = rcl_lexer_lookahead2_accept(&buffer, &lexeme_text, &lexeme_text_length);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_STREQ("rostopic://", std::string(lexeme_text, lexeme_text_length).c_str());

  // \2
  ret = rcl_lexer_lookahead2_peek(&buffer, &lexeme);
  EXPECT_EQ(RCL_LEXEME_BR2, lexeme);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  ret = rcl_lexer_lookahead2_accept(&buffer, &lexeme_text, &lexeme_text_length);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_STREQ("\\2", std::string(lexeme_text, lexeme_text_length).c_str());

  // rosservice://
  ret = rcl_lexer_lookahead2_peek(&buffer, &lexeme);
  EXPECT_EQ(RCL_LEXEME_URL_SERVICE, lexeme);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  ret = rcl_lexer_lookahead2_accept(&buffer, &lexeme_text, &lexeme_text_length);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_STREQ("rosservice://", std::string(lexeme_text, lexeme_text_length).c_str());

  // ~/
  ret = rcl_lexer_lookahead2_peek(&buffer, &lexeme);
  EXPECT_EQ(RCL_LEXEME_TILDE_SLASH, lexeme);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  ret = rcl_lexer_lookahead2_accept(&buffer, &lexeme_text, &lexeme_text_length);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_STREQ("~/", std::string(lexeme_text, lexeme_text_length).c_str());

  // \8
  ret = rcl_lexer_lookahead2_peek(&buffer, &lexeme);
  EXPECT_EQ(RCL_LEXEME_BR8, lexeme);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  ret = rcl_lexer_lookahead2_accept(&buffer, &lexeme_text, &lexeme_text_length);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_STREQ("\\8", std::string(lexeme_text, lexeme_text_length).c_str());

  // :=
  ret = rcl_lexer_lookahead2_peek(&buffer, &lexeme);
  EXPECT_EQ(RCL_LEXEME_SEPARATOR, lexeme);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  ret = rcl_lexer_lookahead2_accept(&buffer, &lexeme_text, &lexeme_text_length);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_STREQ(":=", std::string(lexeme_text, lexeme_text_length).c_str());

  // **
  ret = rcl_lexer_lookahead2_peek(&buffer, &lexeme);
  EXPECT_EQ(RCL_LEXEME_WILD_MULTI, lexeme);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  ret = rcl_lexer_lookahead2_accept(&buffer, &lexeme_text, &lexeme_text_length);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_STREQ("**", std::string(lexeme_text, lexeme_text_length).c_str());

  // :
  ret = rcl_lexer_lookahead2_peek(&buffer, &lexeme);
  EXPECT_EQ(RCL_LEXEME_COLON, lexeme);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  ret = rcl_lexer_lookahead2_accept(&buffer, &lexeme_text, &lexeme_text_length);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_STREQ(":", std::string(lexeme_text, lexeme_text_length).c_str());

  // *
  ret = rcl_lexer_lookahead2_peek(&buffer, &lexeme);
  EXPECT_EQ(RCL_LEXEME_WILD_ONE, lexeme);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  ret = rcl_lexer_lookahead2_accept(&buffer, &lexeme_text, &lexeme_text_length);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_STREQ("*", std::string(lexeme_text, lexeme_text_length).c_str());

  // foobar
  ret = rcl_lexer_lookahead2_peek(&buffer, &lexeme);
  EXPECT_EQ(RCL_LEXEME_TOKEN, lexeme);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  ret = rcl_lexer_lookahead2_accept(&buffer, &lexeme_text, &lexeme_text_length);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_STREQ("foobar", std::string(lexeme_text, lexeme_text_length).c_str());

  // eof
  ret = rcl_lexer_lookahead2_peek(&buffer, &lexeme);
  EXPECT_EQ(RCL_LEXEME_EOF, lexeme);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  ret = rcl_lexer_lookahead2_accept(&buffer, &lexeme_text, &lexeme_text_length);
  ASSERT_EQ(RCL_RET_OK, ret) << rcl_get_error_string_safe();
  EXPECT_STREQ("", std::string(lexeme_text, lexeme_text_length).c_str());
}
