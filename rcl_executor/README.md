# The rcl_executor package

This real-time executor is targeted as C API  based on the RCL layer for ROS2 applications running
on a micro-controller[micro-ROS]. The rcl_executor is a small [ROS 2](http://www.ros2.org/) package
for providing real-time scheduling. Currently the package supports a static order scheduler with
logical-execution-time (LET) semantics.

Static order scheduling refers to the fact, that during configuration the order of DDS handles,
e.g. subscriptions and timers, is specfied. If at runtime multiple handles are ready, e.g.
subscriptions received new messages or timers are ready, then these handles are always processed
in the order as specified during configuration.

LET refers to a semantic, where input data is first read for all handles, then all handels are
processed. The benefit for static order scheduling, is that provides a deterministic execution in
the case when multiple data is available. The benefit of LET is that there is no interference of
input data for all handles processed. For example, if a timer callback evaluates the data processed
by a subscription callback and the order of these is not given, then the timer callback could
sometimes process the old data and sometimes the new. By reading all data first (and locally
copying it), this dependence is removed.
The benefit is very low synchronization effort because interference between input data is avoided
[EK2018] [BP2017]. See also a more detailed motivation of this Real Time Executor on the
[micro-ROS website](https://micro-ros.github.io/docs/concepts/client_library/real-time_executor/).

## LET semantics 

The LET-semantic is implemented by, 
  1) waiting for new data from DDS (rcl_wait()), then 
  2) requesting all new data for all ready handles from DDS (rcl_take()) and then
  3) executing all ready handles in the order specified by the user. 

 Waiting until all callbacks are processed and then publishing them, has been omitted in this
 implementation of the LET semantics, because this would require potentially unbounded buffers
 for publishing messages. However, this sequence already guarantees no interference between input
data for the handles.

## Memory allocation

Special care has been taken about memory allocation. Memory is only allocated in the configuration
phase, i.e. by specifying how many handles shall be executed, while at running phase, no memory is
allocated by the RCL Executor.

## User Interface

The API of the RCL-Executor provides functions, like `add_subscription` and `add_timer`, to add
rcl-subscriptions and rcl-timers to the Executor. The order, in which the user adds these handles
to the Executor, determines later on the execution order when new data is available for these
handles. The executor is activated by calling the `spin_some`, `spin_period()` or `spin()` method.

The API of the static LET scheduler provide the following functions for configuration, defining the
execution order, running the scheduler and cleaning-up:

### API Function Overview
**Configuration**
- rcle_let_executor_init()
- rcle_let_executor_set_timeout()

**Definition of the execution order**
- rcle_let_executor_add_subscription()
- rcle_let_executor_add_timer()

**Running the scheduler**
- rcle_let_executor_spin_some()
- rcle_let_executor_spin_period()
- rcle_let_executor_spin()

**Clean-up memory**
- rcle_let_executor_fini()

### API Description

In the function `rcle_let_executor_init`, the user must specify among other things how many handles
shall be scheduled.

The function `rcle_let_executor_set_timeout` is an optional configuration function, which defines
the timeout for calling rcl_wait(), i.e. the time to wait for new data from the DDS queue. The
default timeout is 100ms.

The functions `rcle_let_executor_add_subscription` and `rcle_let_executor_add_timer` add the
corresponding handle to the executor. The maximum number of handles is defined in
`rcle_let_executor_init`. The sequential order of these function calls defines the static
execution order in the spin-functions.

The function `rcle_let_executor_spin_some` checks for new data from the DDS queue once. It first
copies all data into local data structures and then executes all handles according the specified
order. This implements the LET semantics.

The function `rcle_let_executor_spin_period` calls `rcle_let_executor_spin_some` periodically
(as defined with the argument period) as long as the ROS system is alive.

The function `rcle_let_executor_spin` calls `rcle_let_executor_spin_some` indefinitely as long
as the ROS system is alive. This might create a high performance load on your processor.

The function `rlce_executor_fini` frees the dynamically allocated memory of the executor.

## Example

An example, how to use the RCL Executor with the
[rcl_ext package](https://github.com/micro-ROS/rcl/tree/feature/rcl_ext_and_executor/rcl_ext)
(a package to simplify creation of rcl handles) can be found in the repository micro-ROS-demos on
branch [feature/rcl_executor_examples](https://github.com/micro-ROS/micro-ROS-demos/tree/feature/rcl_executor_examples/C/rcl_executor_examples)

Building and running the example:
After having built the rcl package with rcl_executor and rcl_ext), do the following:
``` C
$>colcon build --packages-select rcl_executor_examples
$>ros2 run rcl_executor_examples executor_with_rcl_ext
```

## Limitations: 

- support for subscriptions and timers (services, clients, guard conditions are not supported yet)

## References

[micro-ROS] [micro-ROS project](https://micro-ros.github.io/) 

[EK2018] R. Ernst, S. Kuntz, S. Quinton, M. Simons: The Logical Execution Time Paradigm: New Perspectives for Multicore Systems, February 25-28 2018 (Dagstuhl Seminar 18092). [Paper](http://drops.dagstuhl.de/opus/volltexte/2018/9293/pdf/dagrep_v008_i002_p122_18092.pdf)

[BP2017] A. Biondi, P. Pazzaglia, A. Balsini, M. D. Natale: Logical Execution Time Implementation and Memory Optimization Issues in AUTOSAR Applications for Multicores, International Worshop on Analysis Tools and Methodologies for Embedded and Real-Time Systems (WATERS2017), Dubrovnik, Croatia.[Paper](https://pdfs.semanticscholar.org/4a9e/b9a616c25fd0b4a4f7810924e73eee0e7515.pdf)


## Step-by-step guide

**Step 1:** <a name="Step1"> </a> Include the `let_executor.h` from the rcl_executor package in your C code.

```C
#include "rcl_executor/let_executor.h"
```

**Step 2:** <a name="Step2"> </a> Define a subscription callback `cmd_hello_callback`.

```C
// callback for topic "cmd_hello"
#include <std_msgs/msg/string.h>
void cmd_hello_callback(const void * msgin)
{
  const std_msgs__msg__String * msg = (const std_msgs__msg__String *)msgin;
  printf("Callback 'cmd_hello': I heard: %s\n", msg->data.data);
}
```

**Step 3:** <a name="Step3"> </a> Define a timer callback `my_timer_callback`.

```C
// timer callback
void my_timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{
  if (timer != NULL) {
    printf("Timer: time since last call %d\n", (int) last_call_time);
  }
}
```


**Step 4:** <a name="Step4"> </a> Create a ROS node with rcl library in main function.

```C
int main(int argc, const char * argv[])
{
  // rcl node initialization
  rcl_context_t context;                // global static var in rcl
  rcl_init_options_t init_options;      // global static var in rcl
  rcl_ret_t rc;

  // create init_options
  init_options = rcl_get_zero_initialized_init_options();
  rc = rcl_init_options_init(&init_options, rcl_get_default_allocator());
  if (rc != RCL_RET_OK) {
      printf("Error rcl_init_options_init.\n");
      return -1;
  }

  // create context
  context = rcl_get_zero_initialized_context();
  rc = rcl_init(argc, argv, &init_options, &context);
  if (rc != RCL_RET_OK) {
    printf("Error rcl_init.\n");
    return -1;
  }

  // create ROS node
  rcl_node_t node = rcl_get_zero_initialized_node();
  rcl_node_options_t node_ops = rcl_node_get_default_options();
  rc = rcl_node_init(&node, "rcle_let_executor_test1_node", "", &context, &node_ops);
  if (rc != RCL_RET_OK) {
    printf("Error rcl_node_init\n");
    return -1;
  }
```

**Step 5:** <a name="Step5"> </a> Create a subscription `sub_cmd_hello`with rcl library.

```C
  // create subscription
  const char * cmd_hello_topic_name = "cmd_hello";
  rcl_subscription_t sub_cmd_hello = rcl_get_zero_initialized_subscription();
  rcl_subscription_options_t subscription_ops2 = rcl_subscription_get_default_options();
  const rosidl_message_type_support_t * sub_type_support2 = ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs,
      msg,
      String);
  std_msgs__msg__String msg2;

  rc = rcl_subscription_init(
    &sub_cmd_hello,
    &node,
    sub_type_support2,
    cmd_hello_topic_name,
    &subscription_ops2);

  if (rc != RCL_RET_OK) {
    printf("Failed to create subscriber %s.\n",cmd_hello_topic_name);
    return -1;
  } else {
    printf("Created subscriber %s:\n", cmd_hello_topic_name);
  }
```


**Step 6:** <a name="Step6"> </a> Create a timer `timer1_timeout` with rcl library with a timeout
of 100ms with timer callback `timer1_timeout`, as defined in [Step 3](#Step3).

```C
  // create timer with rcl
  rcl_clock_t clock;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rc = rcl_clock_init(RCL_STEADY_TIME, &clock, &allocator);
  if (rc != RCL_RET_OK) {
    printf("Error calling rcl_clock_init.\n");
    return -1;
  }
  rcl_timer_t timer1 = rcl_get_zero_initialized_timer();
  const unsigned int timer1_timeout = 100;
  rc = rcl_timer_init(&timer1, &clock, &context, RCL_MS_TO_NS(timer1_timeout),
      my_timer_callback, allocator);
  if (rc != RCL_RET_OK) {
    PRINT_RCL_ERROR(create_timer, rcl_timer_init);
    return -1;
  } else {
    printf("Created timer1 with timeout %d ms.\n", timer1_timeout);
  }
```

**Step 7:** <a name="Step7"> </a> Create an static-let executor and initialize it with the ROS context
(`context`), number of handles (`2`) and provide an allocator for memory allocation.
(See [Step 6](#Step6) for defining the `allocator`).

The user can configure, when the callback shall be invoked: Options are `ALWAYS` and `ON_NEW_DATA`. If `ALWAYS` is selected, the callback is always called, even if no new data is available. In this case, the callback is given a `NULL`pointer for the argument `msgin` and the callback needs to handle this correctly. If `ON_NEW_DATA` is selected, then the callback is called only if new data from the DDS queue is available. In this case the parameter `msgin` of the callback always points to memory-allocated message.

```C
  rcle_let_executor_t exe;
  rcle_let_executor_init(&exe, &context, 2, &allocator);
```

**Step 8:** <a name="Step8"> </a> Add subscription to the executor using the subscription `sub_cmd_hello`, the message variable `msg2`, in which the new data is stored (See [Step 2](#Step2)), and the callback function `cmd_hello_callback`(See [Step 5](#Step5)).

```C
  rc = rcle_let_executor_add_subscription(&exe, &sub_cmd_hello, &msg2, &cmd_hello_callback,
      ON_NEW_DATA);
  if (rc != RCL_RET_OK) {printf("Failed to add subscription.\n");}
```

**Step 9:** <a name="Step9"> </a> Add timer to the executor with the timer `timer1`, as defined in [Step 6](#Step6). The period of the timer is already configured in the timer object.

```C
  rcle_let_executor_add_timer(&exe, &timer1);
  if (rc != RCL_RET_OK) {PRINT_RCL_ERROR(rcle_executor, add_timer);}
```


**Step 10:** <a name="Step10"> </a>(Optionally) Define the timeout for accessing the wait-set of DDS queue. Here the timeout is `100ms`.

```C
  //set time_out for wait_set in nanoseconds
  rc = rcle_let_executor_set_timeout(&exe, 100000000);
  if (rc != RCL_RET_OK) { printf("Setup of timeout failed.\n");}
```
**Step 11:** <a name="Step11"> </a> Run the executor. This example shows the execution of the spin function with a period of 20ms.

```C
  // spin with 20ms period
  rcle_let_executor_spin_period(&exe, 20);
```

**Step 12:** <a name="Step12"> </a> Clean up memory for Executor.

```C
  rcle_let_executor_fini(&exe);
```

**Step 13:** <a name="Step13"> </a> Clean up memory for other ROS objects.

```C
  rc = rcl_subscription_fini(&sub_cmd_hello, &node);
  rc = rcl_timer_fini(&timer1);
  rc = rcl_node_fini(&node);
```
