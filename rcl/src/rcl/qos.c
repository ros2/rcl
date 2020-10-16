// Copyright 2020 Open Source Robotics Foundation, Inc.
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

#include "rcl/qos.h"

const char *
rcl_qos_durability_policy_to_str(enum rmw_qos_durability_policy_t value)
{
  switch (value) {
    case RMW_QOS_POLICY_DURABILITY_SYSTEM_DEFAULT:
      return "system_default";
    case RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL:
      return "transient_local";
    case RMW_QOS_POLICY_DURABILITY_VOLATILE:
      return "volatile";
    case RMW_QOS_POLICY_DURABILITY_UNKNOWN:  // fallthrough
    default:
      return NULL;
  }
}

const char *
rcl_qos_history_policy_to_str(enum rmw_qos_history_policy_t value)
{
  switch (value) {
    case RMW_QOS_POLICY_HISTORY_SYSTEM_DEFAULT:
      return "system_default";
    case RMW_QOS_POLICY_HISTORY_KEEP_LAST:
      return "keep_last";
    case RMW_QOS_POLICY_HISTORY_KEEP_ALL:
      return "keep_all";
    case RMW_QOS_POLICY_HISTORY_UNKNOWN:  // fallthrough
    default:
      return NULL;
  }
}

const char *
rcl_qos_liveliness_policy_to_str(enum rmw_qos_liveliness_policy_t value)
{
  switch (value) {
    case RMW_QOS_POLICY_LIVELINESS_SYSTEM_DEFAULT:
      return "system_default";
    case RMW_QOS_POLICY_LIVELINESS_AUTOMATIC:
      return "automatic";
    case RMW_QOS_POLICY_LIVELINESS_MANUAL_BY_TOPIC:
      return "manual_by_topic";
    case RMW_QOS_POLICY_LIVELINESS_UNKNOWN:  // fallthrough
    default:
      return NULL;
  }
}

const char *
rcl_qos_reliability_policy_to_str(enum rmw_qos_reliability_policy_t value)
{
  switch (value) {
    case RMW_QOS_POLICY_RELIABILITY_SYSTEM_DEFAULT:
      return "system_default";
    case RMW_QOS_POLICY_RELIABILITY_RELIABLE:
      return "reliable";
    case RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT:
      return "best_effort";
    case RMW_QOS_POLICY_RELIABILITY_UNKNOWN:  // fallthrough
    default:
      return NULL;
  }
}

enum rmw_qos_durability_policy_t
rcl_qos_durability_policy_from_str(const char * str)
{
  if (0 == strcmp("system_default", str)) {
    return RMW_QOS_POLICY_DURABILITY_SYSTEM_DEFAULT;
  }
  if (0 == strcmp("transient_local", str)) {
    return RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL;
  }
  if (0 == strcmp("volatile", str)) {
    return RMW_QOS_POLICY_DURABILITY_VOLATILE;
  }
  return RMW_QOS_POLICY_DURABILITY_UNKNOWN;
}

enum rmw_qos_history_policy_t
rcl_qos_history_policy_from_str(const char * str)
{
  if (0 == strcmp("system_default", str)) {
    return RMW_QOS_POLICY_HISTORY_SYSTEM_DEFAULT;
  }
  if (0 == strcmp("keep_last", str)) {
    return RMW_QOS_POLICY_HISTORY_KEEP_LAST;
  }
  if (0 == strcmp("keep_all", str)) {
    return RMW_QOS_POLICY_HISTORY_KEEP_ALL;
  }
  return RMW_QOS_POLICY_HISTORY_UNKNOWN;
}

enum rmw_qos_liveliness_policy_t
rcl_qos_liveliness_policy_from_str(const char * str)
{
  if (0 == strcmp("system_default", str)) {
    return RMW_QOS_POLICY_LIVELINESS_SYSTEM_DEFAULT;
  }
  if (0 == strcmp("automatic", str)) {
    return RMW_QOS_POLICY_LIVELINESS_AUTOMATIC;
  }
  if (0 == strcmp("manual_by_topic", str)) {
    return RMW_QOS_POLICY_LIVELINESS_MANUAL_BY_TOPIC;
  }
  return RMW_QOS_POLICY_LIVELINESS_UNKNOWN;
}

enum rmw_qos_reliability_policy_t
rcl_qos_reliability_policy_from_str(const char * str)
{
  if (0 == strcmp("system_default", str)) {
    return RMW_QOS_POLICY_RELIABILITY_SYSTEM_DEFAULT;
  }
  if (0 == strcmp("reliable", str)) {
    return RMW_QOS_POLICY_RELIABILITY_RELIABLE;
  }
  if (0 == strcmp("best_effort", str)) {
    return RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT;
  }
  return RMW_QOS_POLICY_RELIABILITY_UNKNOWN;
}
