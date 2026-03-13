#include "rcl/event_callback.h"

#ifdef __cplusplus
extern "C"
{
#endif


rcl_event_callback_with_data_t
rcl_get_zero_initialized_event_callback_with_data()
{
  static rcl_event_callback_with_data_t event_callback;
  return event_callback;
}

#ifdef __cplusplus
}
#endif
