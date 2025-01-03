# New and Improved Automatic Testing Suite for gRPC

### How does it work?

1. The testing suite executes the tests located within the [Tests](Tests/) folder. Name of the tests are the same as the folder names.

2. The configuration of the testing suite is saved in the [testlist.json](pylib/testlist.json) file. We can modify it to run specific tests with specific labview version and bitness according to our needs.

3. For each test, inside the test folder, we have a protofile, a `Python_client` folder that contains the python clients, optionally a `Python_server` folder that contains the python server, an `Impl` folder that stores pre-implemented `Start Sync.vi` for that particular protofile and a `testcases` folder that contain the testcases for each rpc in json format.

4. When executing the testing suite, the following steps are performed for each test:
   - Delete the pre-existing `Generated Server` folder that contains the gRPC Server.
   - Regenerate the gRPC server using the protofile.
   - Copy the `Start Sync.vi` from the `Impl` folder into the new `Generated Server` folder.
   - Regenerate the server (without deleting the previously generated server) if we are not doing clean generation.
   - Run the pre-written python clients in the `Python_client` folder which uses pytest to run all the testcases for each rpc. The testcases are defined in the form of json files in the `testcases` folder.
   - Prints the verbose output of each testcase onto the terminal.

### How to run?

**Prerequisites:**

- Install [Python](https://www.python.org/downloads/windows/)
- Install LabVIEW and [gRPC-LabVIEW](https://github.com/ni/grpc-labview/blob/master/docs/QuickStart.md#labview-grpc)

Now follow the below steps to run the testing suite on windows. Currently only windows is supported.

1. Configure the testing suite by editing the [testlist.json](pylib/testlist.json).

   - Modify the array associated with `"name"` key by adding or removing the names of different tests that you want to run (names of the tests can be found in the [Tests](Tests/) folder)

     ```json
     "name": ["helloworld", "simple-oneof"],
     ```

   - Modify the value associated with `"gen_type"` as follows:

     - 0 - Generate only server
     - 1 - Generate only client
     - 2 - Generate both server and client

     (By default it generates only server)

     ```json
     "gen_type": 0,
     ```

   - Modify the value associated with `"labview-version"` to run the specified labview version.

     (By default it runs Labview 2019)

     ```json
     "labview_version": "2019",
     ```

   - Modify the value associated with `"labview-bitness"` to run the specified labview version with the sepcified bitness.

     (By default it runs Labview 2019 32 bit)

     ```json
     "labview_bitness": "32",
     ```

   - Modify the value associated with `"clean_gen"` to specify whether or not you want to do a clean generation.

     (By default it is set to 'true' which means we are doing a clean generation)

     ```json
     "clean_gen": true
     ```

2. Run the [pylib/run_tests.py](pylib/run_tests.py)

   ```bash
   python pylib/run_tests.py
   ```

### How to add more tests?

Follow the below steps to add more tests in the testing suite.

1. Create a new test by creating a corresponding folder with the same name in the [Tests](Tests/) folder. Folder structure should look like `Tests/<test_name>`.

2. Create the protofile for the new test, naming it `<test_name>.proto`, mirroring the folder name.

3. Create a new LabVIEW project with the test name as `<test_name>.lvproj`.

4. Use LabVIEW-gRPC code generator to generate server. Only select the protofile and the labview project in the code generator and leave all other fields empty.

5. Write the rpc definition in `Start Sync.vi` of the generated server.

6. Create a new folder with the name `Impl` inside the test folder. Folder structure should look like `Tests/<test_name>/Impl`.

7. Copy `Start Sync.vi` from the generated server into the newly created `Impl` folder.

8. Now create another new folder with the name `testcases` inside the test folder. Folder structure should look like `Tests/<test_name>/testcases`

9. Inside the `testcases` folder, create a json file for each rpc method defined in the protofile. The name of the json file will be same as the rpc method's name. These will contain the testcases that the testing suite will run corresponding to each rpc method.

10. Create a `Python_client` folder and add python clients for each rpc into it that will interact with the LabVIEW gRPC Server. The name of the client can be anything but should strictly end with `_client.py` like `<client_name>_client.py`.

11. Optionally, you may also create a `Python_server` folder and write the python server with all the rpc's defined into it. The name of the python server can be anything but we prefer it to be like `<test_name>_server.py`.

### TODO:

1. Add the following tests:

   - [x] Streaming tests
   - [ ] Reflection tests
   - [ ] Client tests (currently we are only testing gRPC Server)
   - [ ] Modification scenarios (do some modification after first generation and then generate and test again)
     - Add/Remove/modify RPC
     - Add/Remove/modify services
     - Add/Remove/modify messages
   - [ ] Backward compatibility tests (server generated without the current feature but needs to work with the changes to the current features)
   - [ ] Imported proto-file tests
   - [x] Multiple RPC methods tests
   - [ ] Oneof only inside a nested message
   - [ ] Multi services tests
   - [ ] Abort tests
   - [ ] Add tests for repeated fields (arrays)
