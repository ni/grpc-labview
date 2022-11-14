# Test

## Prerequisites
Latest grpc-libraries should be installed in LabVIEW through VI Package Manager. This can be done by following the instructions in [Building.md](/docs/Building.md)

## For Running:

### *All Tests*
Run the [python script](../tests/run_tests.py).

### *Individual Tests*
1. Open the Test VI in LabVIEW 2019.
2. Run it.
3. You will find the test result in the Front Panel fo the VI.

## For Adding New Tests

1. Create a Test VI.
2. Add the VI to the [Test Repository](../tests/AutoTests/)
3. Add the Test VI's relative path to [Tests.lst](../tests/Tests.lst)
