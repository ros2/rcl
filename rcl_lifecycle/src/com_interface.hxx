// Copyright 2016 Open Source Robotics Foundation, Inc.
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

#ifndef COM_INTERFACE_HXX_
#define COM_INTERFACE_HXX_

#if __cplusplus
extern "C"
{
#endif

#include "rcl_lifecycle/data_types.h"
#include "rcl_lifecycle/visibility_control.h"

LIFECYCLE_EXPORT
rcl_com_interface_t
rcl_get_zero_initialized_com_interface();

LIFECYCLE_EXPORT
rcl_ret_t
rcl_com_interface_init(rcl_com_interface_t * com_interface,
  rcl_node_t * node_handle,
  const rosidl_message_type_support_t * ts_pub_notify,
  const rosidl_service_type_support_t * ts_srv_get_state,
  const rosidl_service_type_support_t * ts_srv_change_state);

LIFECYCLE_EXPORT
rcl_ret_t
rcl_com_interface_fini(rcl_com_interface_t * com_interface,
  rcl_node_t * node_handle);

LIFECYCLE_EXPORT
rcl_ret_t
rcl_com_interface_publish_notification(rcl_com_interface_t * com_interface,
  const rcl_state_t * start, const rcl_state_t * goal);

#if __cplusplus
}
#endif

#endif  // COM_INTERFACE_HXX_
