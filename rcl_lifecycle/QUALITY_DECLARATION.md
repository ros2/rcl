This document is a declaration of software quality for the `rcl_lifecycle` package, based on the guidelines in [REP-2004](https://www.ros.org/reps/rep-2004.html).

# `rcl_lifecycle` Quality Declaration

The package `rcl_lifecycle` claims to be in the **Quality Level 1** category when it is used with a **Quality Level 1** middleware.

Below are the rationales, notes, and caveats for this claim, organized by each requirement listed in the [Package Quality Categories in REP-2004](https://www.ros.org/reps/rep-2004.html).

## Version Policy [1]

### Version Scheme [1.i]

`rcl_lifecycle` uses `semver` according to the recommendation for ROS Core packages in the [ROS 2 Developer Guide](https://docs.ros.org/en/foxy/Contributing/Developer-Guide.html#versioning).

### Version Stability [1.ii]

`rcl_lifecycle` is at a stable version, i.e. `>= 1.0.0`.
The current version can be found in its [package.xml](package.xml), and its change history can be found in its [CHANGELOG](CHANGELOG.rst).

### Public API Declaration [1.iii]

All symbols in the installed headers are considered part of the public API.

All installed headers are in the `include` directory of the package, headers in any other folders are not installed and considered private.

### API Stability Within a Released ROS Distribution [1.iv]/[1.vi]

`rcl_lifecycle` will not break public API within a released ROS distribution, i.e. no major releases once the ROS distribution is released.

### ABI Stability Within a Released ROS Distribution [1.v]/[1.vi]

`rcl_lifecycle` contains C and C++ code and therefore must be concerned with ABI stability, and will maintain ABI stability within a ROS distribution.

## Change Control Process [2]

`rcl_lifecycle` follows the recommended guidelines for ROS Core packages in the [ROS 2 Developer Guide](https://docs.ros.org/en/foxy/Contributing/Developer-Guide.html#quality-practices).

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

`rcl_lifecycle` has feature documentation describing lifecycle nodes.
It is [hosted](https://design.ros2.org/articles/node_lifecycle.html).

### Public API Documentation [3.ii]

Most of `rcl_lifecycle` has embedded API documentation. It is not yet hosted publicly.

### License [3.iii]

The license for `rcl_lifecycle` is Apache 2.0, and a summary is in each source file, the type is declared in the [package.xml](package.xml) manifest file, and a full copy of the license is in the [LICENSE](../LICENSE) file.

There is an automated test which runs a linter that ensures each file has a license statement.

The most recent test results can be found [here](http://build.ros2.org/view/Fpr/job/Fpr__rcl__ubuntu_focal_amd64/lastCompletedBuild/testReport/rcl_lifecycle).

### Copyright Statements [3.iv]

The copyright holders each provide a statement of copyright in each source code file in `rcl_lifecycle`.

There is an automated test which runs a linter that ensures each file has at least one copyright statement.

The results of the test can be found [here](http://build.ros2.org/view/Fpr/job/Fpr__rcl__ubuntu_focal_amd64/lastCompletedBuild/testReport/rcl_lifecycle).

## Testing [4]

### Feature Testing [4.i]

`rcl_lifecycle` has feature tests, which test for proper node state transitions.
The tests are located in the [test](test) subdirectory.
New features are required to have tests before being added.
Though there are no nightly jobs for foxy outside of linux, each change is tested on ci.ros2.org.
* [linux-aarch64](https://ci.ros2.org/job/ci_linux-aarch64)
* [linux](https://ci.ros2.org/job/ci_linux)
* [mac_osx](https://ci.ros2.org/job/ci_osx)
* [windows](https://ci.ros2.org/job/ci_windows)

### Public API Testing [4.ii]

Each part of the public API has tests, and new additions or changes to the public API require tests before being added. The tests aim to cover both typical usage and corner cases, but are quantified by contributing to code coverage.

### Coverage [4.iii]

`rcl_lifecycle` follows the recommendations for ROS Core packages in the [ROS 2 Developer Guide](https://docs.ros.org/en/foxy/Contributing/Developer-Guide.html#code-coverage), and opts to use line coverage instead of branch coverage.

This includes:

- tracking and reporting line coverage statistics
- no lines are manually skipped in coverage calculations

Changes are required to make a best effort to keep or increase coverage before being accepted, but decreases are allowed if properly justified and accepted by reviewers.

Current coverage statistics can be viewed [here](https://ci.ros2.org/job/nightly_linux_foxy_coverage/lastCompletedBuild/cobertura/src_ros2_rcl_rcl_lifecycle_src/). A description of how coverage statistics are calculated is summarized in this page ["ROS 2 Onboarding Guide"](https://docs.ros.org/en/foxy/Contributing/Developer-Guide.html#note-on-coverage-runs).

### Performance [4.iv]

`rcl_lifecycle` follows the recommendations for performance testing of C code in the [ROS 2 Developer Guide](https://docs.ros.org/en/foxy/Contributing/Developer-Guide.html#performance), and opts to do performance analysis on each release rather than each change.

System level performance benchmarks that cover features of `rcl_lifecycle` can be found at:
* [Benchmarks](http://build.ros2.org/view/Fci/job/Fci__benchmark_ubuntu_focal_amd64/BenchmarkTable/)
* [Performance](http://build.ros2.org/view/Fci/job/Fci__nightly-performance_ubuntu_focal_amd64/lastCompletedBuild/)

Changes that introduce regressions in performance must be adequately justified in order to be accepted and merged.

### Linters and Static Analysis [4.v]

`rcl_lifecycle` uses and passes all the standard linters and static analysis tools for a C package as described in the [ROS 2 Developer Guide](https://docs.ros.org/en/foxy/Contributing/Developer-Guide.html#linters-and-static-analysis).

Results of the nightly linter tests can be found [here](http://build.ros2.org/view/Fpr/job/Fpr__rcl__ubuntu_focal_amd64/lastCompletedBuild/testReport/rcl_lifecycle).

## Dependencies [5]

Below are evaluations of each of `rcl_lifecycle`'s run-time and build-time dependencies that have been determined to influence the quality.

It has several "buildtool" dependencies, which do not affect the resulting quality of the package, because they do not contribute to the public library API.
It also has several test dependencies, which do not affect the resulting quality of the package, because they are only used to build and run the test code.

### Direct Runtime ROS Dependencies [5.i]/[5.ii]

`rcl_lifecycle` has the following runtime ROS dependencies:

#### `lifecycle_msgs`

`lifecycle_msgs` provides message and services for managing lifecycle nodes.

It is **Quality Level 1**, see its [Quality Declaration document](https://github.com/ros2/rcl_interfaces/blob/foxy/lifecycle_msgs/QUALITY_DECLARATION.md).

#### `rcl`

`rcl` is the ROS 2 client library in C.

It is **Quality Level 1**, see its [Quality Declaration document](../rcl/QUALITY_DECLARATION.md).

#### `rcutils`

`rcutils` provides commonly used functionality in C.

It is **Quality Level 1**, see its [Quality Declaration document](https://github.com/ros2/rcutils/blob/foxy/QUALITY_DECLARATION.md).

#### `rmw`

`rmw` is the ROS 2 middleware library.

It is **Quality Level 1**, see its [Quality Declaration document](https://github.com/ros2/rmw/blob/foxy/rmw/QUALITY_DECLARATION.md).

#### `rosidl_runtime_c`

`rosidl_runtime_c` provides runtime functionality for rosidl message and service interfaces.

It is **Quality Level 1**, see its [Quality Declaration document](https://github.com/ros2/rosidl/blob/foxy/rosidl_runtime_c/QUALITY_DECLARATION.md).

### Direct Runtime Non-ROS Dependencies [5.iii]

`rcl_lifecycle` does not have any runtime non-ROS dependencies.

## Platform Support [6]

`rcl_lifecycle` supports all of the tier 1 platforms as described in [REP-2000](https://www.ros.org/reps/rep-2000.html#support-tiers), and tests each change against all of them.

Though there are no nightly jobs for foxy outside of linux, each change is tested on ci.ros2.org.
* [linux-aarch64](https://ci.ros2.org/job/ci_linux-aarch64)
* [linux](https://ci.ros2.org/job/ci_linux)
* [mac_osx](https://ci.ros2.org/job/ci_osx)
* [windows](https://ci.ros2.org/job/ci_windows)

# Security [7]

## Vulnerability Disclosure Policy [7.i]

This package conforms to the Vulnerability Disclosure Policy in [REP-2006](https://www.ros.org/reps/rep-2006.html).
