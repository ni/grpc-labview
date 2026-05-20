# Overview
`CopyServerCode.vi` is a utility to hack around a problem in the LabVIEW gRPC generation - one cannot change a .proto file and then successfully change an existing server. This utility copies user code from an older set of server code into a newer set of server code, allowing the user to generate new functionality while maintaining old code.

# Version 
LabVIEW 2025Q1

# Prerequisites
The code assumes several things. These are:
1. The only part of the server code which was modified is the interior of the event structure frames.
2. All data from the event structure internal data node is wired to subVIs.
3. The original working code is in one server in the project, and a blank, just generated server is in the same project.
4. The servers are in folders with the same name as the library containing the server.
5. There are absolutely no external dependencies on the server library code.
6. Data converters to/from LabVIEW gRPC typedefs are in a directory called 'Data Converters'. This directory will be transferred to the new server during the copy process.

# Use
Follow these steps to add/subtract functions to an existing server:
1. Modify your existing .proto file and save it with a new name (versioning is highly recommended)
2. With this new .proto file, use the NI gRPC server/client code generator to create a new server in the same project as the original. Make sure you give the server a new name. Note that at the end of the process, the server will have the original name and location so that code linkages will not be broken.
3. Save and close the project.
4. Open the `CopyServerCode.vi`.
5. Fill out the project path with the full path to the project containing the old and new servers.
6. Fill out the target name with the project target the servers are under. This is usually `My Computer`, but can be an embedded target, as well.
7. Enter the full name of the old (source) and new (destination) server names. This is the full name of the folder containing the servers. The library in the folder must be the same name (with .lvlib appended) for this to work.
8. Run the utility. It will make a backup in the `<temp directory>\ServerBackups\Backup_*n*` directory in case something goes wrong. In Windows, '<temp directory>' is 'C:\Users\<username>\AppData\Local\Temp'.
9. Open the project and fill in the functionality in the new event structure frames, if any. Add new data converters as needed.

# Contact
damien.gray@3dsystems.com
