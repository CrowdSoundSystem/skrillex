# Skrillex
The database that powers CrowdSound. Rest assured, the base never dies.

## Building
Building is as simple as running `CMake`. However, it is recommended
to create a seperate directory for building. For example, from the project root:

```
$ mkdir build
$ cd build
$ cmake ..
$ make
```

## Testing
Testing is done using Google Test, which is embedded in repo. To run
tests, simply build the project (as shown above), and `bin/skrillex-tests`.
For example, from the build directory:

```
$ make
$ bin/skrillex-tests
```
