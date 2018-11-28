# Copyright 2018 Open Source Robotics Foundation, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#
# Get the package name of the default logging implementation.
#
# Either selecting it using the variable RC_LOGGING_IMPLEMENTATION or
# choosing a default from the available implementations.
#
# :param var: the output variable name containing the package name
# :type var: string
#
macro(get_default_rc_logging_implementation var)

  # if logging implementation already specified or RC_LOGGING_IMPLEMENTATION environment variable
  # is set then use that, otherwise default to using rc_logging_log4cxx
  if(NOT "${RC_LOGGING_IMPLEMENTATION}" STREQUAL "")
    set(_logging_implementation "${RC_LOGGING_IMPLEMENTATION}")
  elseif(NOT "$ENV{RC_LOGGING_IMPLEMENTATION}" STREQUAL "")
    set(_logging_implementation "$ENV{RC_LOGGING_IMPLEMENTATION}")
  else()
    set(_logging_implementation rc_logging_log4cxx)
  endif()

  # persist implementation decision in cache
  # if it was not determined dynamically
  set(
    RC_LOGGING_IMPLEMENTATION "${_logging_implementation}"
    CACHE STRING "Select ROS middleware implementation to link against" FORCE
  )

  find_package("${_logging_implementation}" REQUIRED)

  set(${var} ${_logging_implementation})
endmacro()
