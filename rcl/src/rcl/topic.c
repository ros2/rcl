// Copyright 2015 Open Source Robotics Foundation, Inc.
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

#if __cplusplus
extern "C"
{
#endif

#include "rcl/topic.h"

#include "rmw/rmw.h"
#include "./common.h"

#include <stdio.h>

rcl_ret_t
rcl_get_remote_topic_names_and_types()
{
	printf("rcl_get_remote_topic_names_and_types\n");
	rmw_topic_names_and_types_t topic_names_and_types;
	topic_names_and_types.topic_count = 0;
	topic_names_and_types.topic_names = NULL;
	topic_names_and_types.type_names = NULL;

	auto ret = rmw_get_remote_topic_names_and_types(&topic_names_and_types);
	if (ret != RMW_RET_OK) {
    	RCL_SET_ERROR_MSG(rmw_get_error_string_safe());
		return RCL_RET_ERROR;
	}

	printf("topic_names_and_types size: %d\n", topic_names_and_types.topic_count);
	unsigned int i;
	for(i = 0; i < topic_names_and_types.topic_count; i++){
		printf("topic_names_and_types[%d] name: %s\t type_topic %s\t\n",i, topic_names_and_types.topic_names[i], topic_names_and_types.type_names[i] );
	}

	ret = rmw_destroy_topic_names_and_types(&topic_names_and_types);
	if (ret != RMW_RET_OK) {
    	RCL_SET_ERROR_MSG(rmw_get_error_string_safe());
		return RCL_RET_ERROR;
	}
 	return RCL_RET_OK;
}

#if __cplusplus
}
#endif
