# Quick Start - LabVIEW RT Addendum

After executing the steps in the [Quick Start](https://github.com/ni/grpc-labview/blob/master/docs/QuickStart.md) guide, one more step must be completed to run on an RT target.
The shared object file must be copied to the RT target. Do the following:

1. Find `<vi.lib>/gRPC/LabVIEW gRPC Library/Libraries/LinuxRT/liblabview_grpc_server.so` on your development device.
1. Copy this file to `/home/lvuser/natinst/bin/liblabview_grpc_server.so` on the RT device. You can use `scp` or `sftp` from a command line interface
or a GUI-based program like the **File Transfer** utility in **NI MAX** or the [**WinSCP**](https://winscp.net/eng/index.php) app to do this file transfer. You will need the admin account name and
password.

Your RT code should now work correctly.