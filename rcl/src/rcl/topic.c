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

#include "rcl/rcl.h"
#include "rmw/rmw.h"
#include "./common.h"

#include <stdio.h>

rcl_ret_t
rcl_get_remote_topic_names_and_types(rcl_strings_t* topic_names_string, rcl_strings_t* type_names_string)
{

	RCL_CHECK_ARGUMENT_FOR_NULL(topic_names_string, RCL_RET_INVALID_ARGUMENT);

	RCL_CHECK_FOR_NULL_WITH_MSG(
		topic_names_string->allocator.allocate,
		"invalid allocator, allocate not set", return RCL_RET_INVALID_ARGUMENT);
	RCL_CHECK_FOR_NULL_WITH_MSG(
		topic_names_string->allocator.deallocate,
		"invalid allocator, deallocate not set", return RCL_RET_INVALID_ARGUMENT);

	RCL_CHECK_ARGUMENT_FOR_NULL(type_names_string, RCL_RET_INVALID_ARGUMENT);

	RCL_CHECK_FOR_NULL_WITH_MSG(
		type_names_string->allocator.allocate,
		"invalid allocator, allocate not set", return RCL_RET_INVALID_ARGUMENT);
	RCL_CHECK_FOR_NULL_WITH_MSG(
		type_names_string->allocator.deallocate,
		"invalid allocator, deallocate not set", return RCL_RET_INVALID_ARGUMENT);

	rmw_topic_names_and_types_t topic_names_and_types;
	topic_names_and_types.topic_count = 0;
	topic_names_and_types.topic_names = NULL;
	topic_names_and_types.type_names = NULL;

    auto ret = rmw_get_remote_topic_names_and_types(&topic_names_and_types);
	if (ret != RMW_RET_OK) {
    	RCL_SET_ERROR_MSG(rmw_get_error_string_safe());
		return RCL_RET_ERROR;
	}

	if(topic_names_and_types.topic_count>0){

		topic_names_string->count = topic_names_and_types.topic_count;
	    topic_names_string->data = (char **)topic_names_string->allocator.allocate(sizeof(char *) * topic_names_and_types.topic_count, topic_names_string->allocator.state);
	    RCL_CHECK_FOR_NULL_WITH_MSG(topic_names_string->data, "allocating memory failed", return RCL_RET_BAD_ALLOC);
	    memset(topic_names_string->data, 0, sizeof(char **) * topic_names_string->count);

		type_names_string->count = topic_names_and_types.topic_count;
	    type_names_string->data = (char **)type_names_string->allocator.allocate(sizeof(char *) * topic_names_and_types.topic_count, type_names_string->allocator.state);
	    RCL_CHECK_FOR_NULL_WITH_MSG(type_names_string->data, "allocating memory failed", return RCL_RET_BAD_ALLOC);
	    memset(type_names_string->data, 0, sizeof(char **) * type_names_string->count);

		unsigned int i;
		for(i = 0; i < topic_names_and_types.topic_count; i++){
			int len_string = strlen(topic_names_and_types.topic_names[i])+1;
			topic_names_string->data[i] = (char *)topic_names_string->allocator.allocate(sizeof(char)*len_string, topic_names_string->allocator.state);
			//TODO deallocate the rest of the array
			RCL_CHECK_FOR_NULL_WITH_MSG(topic_names_string->data[i], "allocating memory failed", return RCL_RET_BAD_ALLOC);
			memcpy(topic_names_string->data[i], topic_names_and_types.topic_names[i], len_string);

			len_string = strlen(topic_names_and_types.type_names[i])+1;
			type_names_string->data[i] = (char *)type_names_string->allocator.allocate(sizeof(char)*len_string, type_names_string->allocator.state);
			//TODO deallocate the rest of the array
			RCL_CHECK_FOR_NULL_WITH_MSG(type_names_string->data[i], "allocating memory failed", return RCL_RET_BAD_ALLOC);
			memcpy(type_names_string->data[i], topic_names_and_types.type_names[i], len_string);
		}
	}else{
		return RCL_RET_ERROR;
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
