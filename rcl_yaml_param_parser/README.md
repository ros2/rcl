**ROS2 rcl YAML paramter parser**

Parse a YAML parameter file and populate the C data structure

The data structure params_st will then be used during node initialization

YAML parameter file should follow the yaml syntax shown below

NOTE: It only supports canonical int and float types

```
<node_namespace_string>:  # optional
  <node1_name>:
    ros__parameters:
      <field_name>: <field_value>
      <parameter_namespace_string>:   # optional
        <field1_name>: <field1_value>
        <field2_name>: <field2_value>
  <node2_name>:
    ros__parameters:
      <field_name>: <field_value>
      <parameter_namespace_string>:   # optional
        <field1_name>: <field1_value>
        <field2_name>: <field2_value>
```

This package depends on C libyaml
