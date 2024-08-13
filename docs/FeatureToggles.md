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

1. `EfficientMessageCopy` - This feature is used to enable or disable the efficient message copy feature. When enabled, the client will use efficient message copy to have throughput. When disabled, the client will use the default message copy.

2. `useOccurrence` - This feature is used to enable or disable the occurrence feature. When enabled, the client will use occurrence to manage synchroniation between LabVIEW execution threads. When disabled, the client will use not use LabVIEW occurrences.