This document is a declaration of software quality for the `rcl` package, based on the guidelines in [REP-2004](https://www.ros.org/reps/rep-2004.html).

# `rcl` Quality Declaration

The package `rcl` claims to be in the **Quality Level 1** category.

Below are the rationales, notes, and caveats for this claim, organized by each requirement listed in the [Package Requirements for Quality Level 1 in REP-2004](https://www.ros.org/reps/rep-2004.html).

## Version Policy

### Version Scheme

`rcl` uses `semver` according to the recommendation for ROS Core packages in the [ROS 2 Developer Guide](https://index.ros.org/doc/ros2/Contributing/Developer-Guide/#versioning), and is at or above a stable version, i.e. `>= 1.0.0`.

### API Stability Within a Released ROS Distribution

`rcl` will not break public API within a released ROS distribution, i.e. no major releases once the ROS distribution is released.

### ABI Stability Within a Released ROS Distribution

`rcl` contains C code and therefore must be concerned with ABI stability, and will maintain ABI stability within a ROS distribution.

### Public API Declaration

All symbols in the installed headers are considered part of the public API.

All installed headers are in the `include` directory of the package, headers in any other folders are not installed and considered private.

## Change Control Process

TODO fix link

`rcl` follows the recommended guidelines for ROS Core packages in the [ROS 2 Developer Guide](https://index.ros.org/doc/ros2/Contributing/Developer-Guide/#change_control_process).

This includes:

- all changes occur through a pull request
- all pull request have two peer reviews
- all pull request must pass CI on all [tier 1 platforms](https://www.ros.org/reps/rep-2000.html#support-tiers)
- all pull request must resolve related documentation changes before merging

## Documentation

### Feature Documentation

TODO fix link

`rcl` has a [feature list](TODO) and each item in the list links to the corresponding feature documentation.
There is documentation for all of the features, and new features require documentation before being added.

### Public API Documentation

TODO fix link

`rcl` has embedded API documentation and it is generated using doxygen and is [hosted](TODO) along side the feature documentation.
There is documentation for all of the public API, and new additions to the public API require documentation before being added.

### License

The license for `rcl` is Apache 2.0, and a summary is in each source file, the type is declared in the `package.xml` manifest file, and a full copy of the license is in the `LICENSE` file.

There is an automated test which runs a linter that ensures each file has a license statement.

### Copyright Statements

The copyright holders each provide a statement of copyright in each source code file in `rcl`.

There is an automated test which runs a linter that ensures each file has at least one copyright statement.

## Testing

### Feature Testing

Each feature in `rcl` has corresponding tests which simulate typical usage, and they are located in the `test` directory.
New features are required to have tests before being added.

### Public API Testing

Each part of the public API have tests, and new additions or changes to the public API require tests before being added.
The tests aim to cover both typical usage and corner cases, but are quantified by contributing to code coverage.

### Coverage

TODO fix link

`rcl` follows the recommendations for ROS Core packages in the [ROS 2 Developer Guide](https://index.ros.org/doc/ros2/Contributing/Developer-Guide/#coverage), and opts to use branch coverage instead of line coverage.

This includes:

- tracking and reporting branch coverage statistics
- achieving and maintaining branch coverage at or above 95%
- no lines are manually skipped in coverage calculations

Changes are required to make a best effort to keep or increase coverage before being accepted, but decreases are allowed if properly justified and accepted by reviewers.

Current coverage statistics can be viewed here:

TODO FIXME

### Performance

TODO fix link

`rcl` follows the recommendations for performance testing of C code in the [ROS 2 Developer Guide](https://index.ros.org/doc/ros2/Contributing/Developer-Guide/#performance-c), and opts to do performance analysis on each release rather than each change.

TODO how to run perf tests, where do they live, etc.

TODO exclusions of parts of the code from perf testing listed here? or linked to?

### Linters and Static Analysis

TODO fix link

`rcl` uses and passes all the standard linters and static analysis tools for a C package as described in the [ROS 2 Developer Guide](https://index.ros.org/doc/ros2/Contributing/Developer-Guide/#linters-and-static-analysis).

TODO any qualifications on what "passing" means for certain linters

## Dependencies

Below are evaluations of each of `rcl`'s run-time and build-time dependencies that have been determined to influence the quality.

It has several "buildtool" dependencies, which do not affect the resulting quality of the package, because they do not contribute to the public library API.
It also has several test dependencies, which do not affect the resulting quality of the package, because they are only used to build and run the test code.

### `rmw`

TODO fix link

The `rmw` package provides the API used by `rcl` to interact with the underlying middleware in an abstract way.

It is **Quality Level 1**, see its [Quality Declaration document](FIX).

### `rcl_interfaces`

TODO fix link

The `rcl_interfaces` package provides some common ROS Message and ROS Service types which are used to implement certain client library features.

It is **Quality Level 1**, see its [Quality Declaration document](FIX).

### `rcl_logging_spdlog`

TODO fix link
TODO should this direct dependency be here?

The `rcl_logging_spdlog` package provides the default logging implementation by wrapping the `spdlog` library.

It is **Quality Level 1**, see its [Quality Declaration document](FIX).

### `rcl_yaml_param_parser`

TODO fix link

The `rcl_yaml_param_parser` package provides an API that is used to parse YAML configuration files which may be used to configure ROS and specific nodes.

It is **Quality Level 1**, see its [Quality Declaration document](FIX).

### `rcutils`

TODO fix link

The `rcutils` package provides an API which contains common utilities and data structures needed when programming in C.

It is **Quality Level 1**, see its [Quality Declaration document](FIX).

### `rmw_implementation`

TODO fix link

The `rmw_implementation` package provides access to the default rmw implementation, and provides the ability to dynamically switch rmw implementations if more than one is available.

It is **Quality Level 1**, see its [Quality Declaration document](FIX).

### `rosidl_generator_c`

TODO fix link

The `rosidl_generator_c` package provides templates for generating data structures in C based on user defined ROS Messages and ROS Services, as well as associated support functions for those types.
`rcl` uses this pacakge because it uses the C instances of these user defined types in some cases, in combination with the types defined in the `rcl_interfaces` package.

It is **Quality Level 1**, see its [Quality Declaration document](FIX).

### `tinydir_vendor`

TODO fix link

The `tinydir_vendor` package wraps the third-party library called `tinydir`, which itself provides some basic filesystem operations.

It is **Quality Level 1**, see its [Quality Declaration document](FIX).

### `tracetools`

TODO fix link

The `tracetools` package provides utilities for instrumenting the code in `rcl` so that it may be traced for debugging and performance analysis.

It is **Quality Level 1**, see its [Quality Declaration document](FIX).

## Platform Support

`rcl` supports all of the tier 1 platforms as described in [REP-2000](https://www.ros.org/reps/rep-2000.html#support-tiers), and tests each change against all of them.

TODO make additional statements about non-tier 1 platforms?
