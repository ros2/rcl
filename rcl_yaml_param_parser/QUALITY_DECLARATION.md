This document is a declaration of software quality for the `rcl_yaml_param_parser` package, based on the guidelines in [REP-2004](https://www.ros.org/reps/rep-2004.html).

# `rcl_yaml_param_parser` Quality Declaration

The package `rcl_yaml_param_parser` claims to be in the **Quality Level 4** category.

Below are the rationales, notes, and caveats for this claim, organized by each requirement listed in the [Package Requirements for Quality Level 4 in REP-2004](https://www.ros.org/reps/rep-2004.html).

## Version Policy [1]

### Version Scheme [1.i]

`rcl_yaml_param_parser` uses `semver` according to the recommendation for ROS Core packages in the [ROS 2 Developer Guide](https://index.ros.org/doc/ros2/Contributing/Developer-Guide/#versioning).

### Version Stability [1.ii]

`rcl_yaml_param_parser` is not yet at a stable version, i.e. `>= 1.0.0`.

### Public API Declaration [1.iii]

All symbols in the installed headers are considered part of the public API.

All installed headers are in the `include` directory of the package, headers in any other folders are not installed and considered private.

### API Stability Within a Released ROS Distribution [1.iv]/[1.vi]

`rcl_yaml_param_parser` will not break public API within a released ROS distribution, i.e. no major releases once the ROS distribution is released.

### ABI Stability Within a Released ROS Distribution [1.v]/[1.vi]

`rcl_yaml_param_parser` contains C and C++ code and therefore must be concerned with ABI stability, and will maintain ABI stability within a ROS distribution.

## Change Control Process [2]

`rcl_yaml_param_parser` follows the recommended guidelines for ROS Core packages in the [ROS 2 Developer Guide](https://index.ros.org/doc/ros2/Contributing/Developer-Guide/#package-requirements).

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

`rcl_yaml_param_parser` does not have feature documentation.

### Public API Documentation [3.ii]

All of `rcl_yaml_param_parser` has embedded API documentation. It is not yet hosted publicly.

### License [3.iii]

The license for `rcl_yaml_param_parser` is Apache 2.0, and a summary is in each source file, the type is declared in the [package.xml](package.xml) manifest file, and a full copy of the license is in the [LICENSE](../LICENSE) file.

There is an automated test which runs a linter that ensures each file has a license statement.

The most recent test results can be found [here](https://ci.ros2.org/view/nightly/job/nightly_linux_release/lastBuild/testReport/rcl_yaml_param_parser/copyright/).

### Copyright Statements [3.iv]

The copyright holders each provide a statement of copyright in each source code file in `rcl_yaml_param_parser`.

There is an automated test which runs a linter that ensures each file has at least one copyright statement.

The results of the test can be found [here](https://ci.ros2.org/view/nightly/job/nightly_linux_release/lastBuild/testReport/rcl_yaml_param_parser/copyright/).

## Testing [4]

### Feature Testing [4.i]

`rcl_yaml_param_parser` has feature tests, which ensure that a wide variety of yaml files are checked for parsing.

### Public API Testing [4.ii]

Much of the API in `rcl_yaml_param_parser` is tested in the aforementioned feature tests, but it is not tested explicitly.

### Coverage [4.iii]

`rcl_yaml_param_parser` does not currently track test coverage.

### Performance [4.iv]

`rcl_yaml_param_parser` does not currently have performance tests.

### Linters and Static Analysis [4.v]

`rcl_yaml_param_parser` uses and passes all the standard linters and static analysis tools for a C package as described in the [ROS 2 Developer Guide](https://index.ros.org/doc/ros2/Contributing/Developer-Guide/#linters).

Results of the nightly linter tests can be found [here](https://ci.ros2.org/view/nightly/job/nightly_linux_release/lastBuild/testReport/rcl_yaml_param_parser).

## Dependencies [5]

Below are evaluations of each of `rcl_yaml_param_parser`'s run-time and build-time dependencies that have been determined to influence the quality.

It has several "buildtool" dependencies, which do not affect the resulting quality of the package, because they do not contribute to the public library API.
It also has several test dependencies, which do not affect the resulting quality of the package, because they are only used to build and run the test code.

### Direct Runtime ROS Dependencies [5.i]/[5.ii]

#### `rcutils`

`rcutils` provides commonly used functionality in C.

It is **Quality Level 4**, see its [Quality Declaration document](https://github.com/ros2/rcutils/blob/master/QUALITY_DECLARATION.md).

#### `libyaml_vendor`

`libyaml_vendor` is a vendor package for the libyaml C library.

It is **Quality Level 4**, see its [Quality Declaration document](https://github.com/ros2/libyaml_vendor/blob/master/QUALITY_DECLARATION.md).

### Direct Runtime Non-ROS Dependencies [5.iii]

`rcl_yaml_param_parser` has the following runtime non-ROS dependencies.

#### `libyaml`

`libyaml` is a YAML parsing library written in C.

It is **Quality Level 4**, see its [Quality Declaration document](https://github.com/ros2/libyaml_vendor/blob/master/libyaml_q_declaration.md).

## Platform Support [6]

`rcl_yaml_param_parser` supports all of the tier 1 platforms as described in [REP-2000](https://www.ros.org/reps/rep-2000.html#support-tiers), and tests each change against all of them.

Currently nightly results can be seen here:
* [linux-aarch64_release](https://ci.ros2.org/view/nightly/job/nightly_linux-aarch64_release/lastBuild/testReport/rcl_yaml_param_parser/)
* [linux_release](https://ci.ros2.org/view/nightly/job/nightly_linux_release/lastBuild/testReport/rcl_yaml_param_parser/)
* [mac_osx_release](https://ci.ros2.org/view/nightly/job/nightly_osx_release/lastBuild/testReport/rcl_yaml_param_parser/)
* [windows_release](https://ci.ros2.org/view/nightly/job/nightly_win_rel/lastBuild/testReport/rcl_yaml_param_parser/)

# Security [7]

## Vulnerability Disclosure Policy [7.i]

This package does not yet have a Vulnerability Disclosure Policy.
