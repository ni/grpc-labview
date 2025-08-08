# Introducing Feature Toggles

Starting from release v1.2.0.1 we have added support for feature toggles. This allows you to enable or disable certain features in the application without the need to redeploy the application.

## How to use feature toggles

Feature toggles are managed through the `feature_config.ini` file (located next to _labview_grpc_server_ library). You can find the `data` section in the file. Here is an example of how to use feature toggles:

```ini
[data]
EfficientMessageCopy = TRUE
useOccurrence = TRUE
```

In the example above, the `EfficientMessageCopy` and `useOccurrence` features are enabled by default. If you want to disable a feature, you can set the value to `FALSE`.

### More about the flags

- `EfficientMessageCopy` - This feature is used to enable or disable the efficient message copy feature. When enabled, the client will use efficient message copy to have throughput. When disabled, the client will use the default message copy.

- `useOccurrence` - This feature is used to enable or disable the occurrence feature. When enabled, the client will use occurrence to manage synchroniation between LabVIEW execution threads. When disabled, the client will use not use LabVIEW occurrences.

- `utf8Strings` - This feature is used to enable or disable UTF-8 string support. When enabled, the client will treat strings in protobuf messages as UTF-8, as documented in https://protobuf.dev/programming-guides/encoding/. When disabled, the client will treat strings in protobuf messages as LabVIEW's native string encoding. This feature is enabled by default.

- `verifyStringEncoding` - This feature is used to enable or disable verification of string encoding. When enabled, functions that take protobuf message/field names will error if the names are not ASCII, parsing gRPC string fields will error if they are not valid UTF-8, and writing gRPC string fields will log if they are not valid UTF-8. This feature is enabled by default.