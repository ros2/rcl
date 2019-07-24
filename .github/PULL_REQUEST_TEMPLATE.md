<!--
Thanks for submitting a Pull Request!

Please shortly explain your contribution, and if fixing an issue from the tracker, please add "Fixes #XXX", replacing XXX with the issue number.

Be sure that your contribution follows the [Developer Guide](https://index.ros.org/doc/ros2/Contributing/Developer-Guide/).

Be sure to go over each item in the list below before submitting your pull request.
-->

### Description

Briefly describe what your changes are doing. It is more important to mention why the changes are necessary, rather than the how.

### Checklist

- [ ] The commit messages include a [DCO](https://discourse.ros.org/t/starting-to-enforce-developer-certificate-of-origin-dco-for-some-ros-2-repos/7420).
- [ ] The commit messages follow [good practices](https://chris.beams.io/posts/git-commit/).
- [ ] The build passes tests locally.
- [ ] The PR is minimal and any unrelated changes are submitted separately.
- [ ] The API documentation has been updated based on changes in APIs or addition of new APIs, if any.
- [ ] The [documentation](https://index.ros.org/doc/ros2/) has been updated to describe the new feature or package added, if any.
- [ ] Added or updated tests corresponding to the PR. If tests cannot be added for some reason, it has been explained in the description.
- [ ] While waiting for someone to review your PR, please consider reviewing [another open pull request](https://github.com/ros2/rcl/pulls) to support the maintainers of RCL. Refer to the [ROS code review guide](https://github.com/rosin-project/ros_code_review_guide/blob/master/README.md) for general reviewing guidelines and [Quality Guide: Ensuring code quality](https://index.ros.org/doc/ros2/Contributing/Quality-Guide/) for ROS 2 specific guidelines

### Testing

PRs must be tested on each Tier 1 platform as documented by the [REP-2000]( http://www.ros.org/reps/rep-2000.html).

- [ ] Linux Ubuntu x86-64
- [ ] Linux Ubuntu arch64
- [ ] OS X Testing
- [ ] Windows Testing

### Release Engineering

- [ ] This PR shoub be backported to supported ROS 2 releases. I understand changes that break API or ABI are not generally eligible to be backported.
