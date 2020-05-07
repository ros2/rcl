This document is a declaration of software quality for the `rcl_action` package, based on the guidelines in [REP-2004](https://www.ros.org/reps/rep-2004.html).

# `rcl_action` Quality Declaration

The package `rcl_action` claims to be in the **Quality Level 4** category.

Below are the rationales, notes, and caveats for this claim, organized by each requirement listed in the [Package Requirements for Quality Level 4 in REP-2004](https://www.ros.org/reps/rep-2004.html).

## Version Policy [1]

### Version Scheme [1.i]

`rcl_action` uses `semver` according to the recommendation for ROS Core packages in the [ROS 2 Developer Guide](https://index.ros.org/doc/ros2/Contributing/Developer-Guide/#versioning).

### Version Stability [1.ii]

`rcl_action` is not yet at a stable version, i.e. `>= 1.0.0`.

### Public API Declaration [1.iii]

All symbols in the installed headers are considered part of the public API.

All installed headers are in the `include` directory of the package, headers in any other folders are not installed and considered private.

### API Stability Within a Released ROS Distribution [1.iv]/[1.vi]

`rcl_action` will not break public API within a released ROS distribution, i.e. no major releases once the ROS distribution is released.

### ABI Stability Within a Released ROS Distribution [1.v]/[1.vi]

`rcl_action` contains C and C++ code and therefore must be concerned with ABI stability, and will maintain ABI stability within a ROS distribution.

## Change Control Process [2]

`rcl_action` follows the recommended guidelines for ROS Core packages in the [ROS 2 Developer Guide](https://index.ros.org/doc/ros2/Contributing/Developer-Guide/#package-requirements).

### Change Requests [2.i]

This package requires that all changes occur through a pull request.

### Contributor Origin [2.ii]

This package uses DCO as its confirmation of contributor origin policy. More information can be found in [CONTRIBUTING](../CONTRIBUTING.md).

### Peer Review Policy [2.iii]

Following the recommended guidelines for ROS Core packages, all pull requests must have at least 1 peer review.

### Continuous Integration [2.iv]

All pull requests must pass CI on all [tier 1 platforms](https://www.ros.org/reps/rep-2000.html#support-tiers).

### Documentation Policy [2.v]

All pull requests must resolve related documentation changes before merging.

## Documentation [3]

### Feature Documentation [3.i]

`rcl_action` has feature documentation actions in ROS 2.
It is [hosted](http://design.ros2.org/articles/actions.html).

### Public API Documentation [3.ii]

Most of `rcl_action` has embedded API documentation and it is [hosted](http://docs.ros2.org/latest/api/rcl_action/).

### License [3.iii]

The license for `rcl_action` is Apache 2.0, and a summary is in each source file, the type is declared in the [package.xml](package.xml) manifest file, and a full copy of the license is in the [LICENSE](../LICENSE) file.

There is an automated test which runs a linter that ensures each file has a license statement.

The most recent test results can be found [here](https://ci.ros2.org/view/nightly/job/nightly_linux_release/lastBuild/testReport/rcl_action/copyright/).

### Copyright Statements [3.iv]

The copyright holders each provide a statement of copyright in each source code file in `rcl_action`.

There is an automated test which runs a linter that ensures each file has at least one copyright statement.

The results of the test can be found [here](https://ci.ros2.org/view/nightly/job/nightly_linux_release/lastBuild/testReport/rcl_action/copyright/).

## Testing [4]

### Feature Testing [4.i]

`rcl_action` has feature tests, which test for proper node state transitions.
The tests are located in the [test](test) subdirectory.

### Public API Testing [4.ii]

Much of the API in `rcl_action` is tested.
The tests are located in the [test](test) subdirectory.

### Coverage [4.iii]

`rcl_action` does not currently track test coverage.

### Performance [4.iv]

`rcl_action` does not currently have performance tests.

### Linters and Static Analysis [4.v]

`rcl_action` uses and passes all the standard linters and static analysis tools for a C package as described in the [ROS 2 Developer Guide](https://index.ros.org/doc/ros2/Contributing/Developer-Guide/#linters).

Results of the nightly linter tests can be found [here](https://ci.ros2.org/view/nightly/job/nightly_linux_release/lastBuild/testReport/rcl_action).

## Dependencies [5]

Below are evaluations of each of `rcl_action`'s run-time and build-time dependencies that have been determined to influence the quality.

It has several "buildtool" dependencies, which do not affect the resulting quality of the package, because they do not contribute to the public library API.
It also has several test dependencies, which do not affect the resulting quality of the package, because they are only used to build and run the test code.

### Direct Runtime ROS Dependencies [5.i]/[5.ii]

`rcl_action` has the following runtime ROS dependencies:

#### `action_msgs`

`action_msgs` provides messages and services for ROS 2 actions.

It is **Quality Level 4**, see its [Quality Declaration document](https://github.com/ros2/rcl_interfaces/blob/master/action_msgs/QUALITY_DECLARATION.md).

#### `rcl`

`rcl` is the ROS 2 client library in C.

It is **Quality Level 4**, see its [Quality Declaration document](../rcl/QUALITY_DECLARATION).

#### `rcutils`

`rcutils` provides commonly used functionality in C.

It is **Quality Level 4**, see its [Quality Declaration document](https://github.com/ros2/rcutils/blob/master/QUALITY_DECLARATION.md).

#### `rmw`

`rmw` is the ROS 2 middleware library.

It is **Quality Level 4**, see its [Quality Declaration document](https://github.com/ros2/rmw/blob/master/rmw/QUALITY_DECLARATION.md).

#### `rosidl_runtime_c`

`rosidl_runtime_c` provides runtime functionality for rosidl message and service interfaces.

It is **Quality Level 4**, see its [Quality Declaration document](https://github.com/ros2/rosidl/blob/master/rosidl_runtime_c/QUALITY_DECLARATION.md).

### Direct Runtime Non-ROS Dependencies [5.iii]

`rcl_action` does not have any runtime non-ROS dependencies.

## Platform Support [6]

`rcl_action` supports all of the tier 1 platforms as described in [REP-2000](https://www.ros.org/reps/rep-2000.html#support-tiers), and tests each change against all of them.

Currently nightly results can be seen here:
* [linux-aarch64_release](https://ci.ros2.org/view/nightly/job/nightly_linux-aarch64_release/lastBuild/testReport/rcl_action/)
* [linux_release](https://ci.ros2.org/view/nightly/job/nightly_linux_release/lastBuild/testReport/rcl_action/)
* [mac_osx_release](https://ci.ros2.org/view/nightly/job/nightly_osx_release/lastBuild/testReport/rcl_action/)
* [windows_release](https://ci.ros2.org/view/nightly/job/nightly_win_rel/lastBuild/testReport/rcl_action/)

# Security [7]

## Vulnerability Disclosure Policy [7.i]

This package does not yet have a Vulnerability Disclosure Policy.
