# Native geometry checks

These host tests run the actual Crownline Circuit and RD-01 generators, plus the vehicle physics. They validate triangles, winding, finite coordinates, road continuity, collision regions, gate locations, resource cleanup, acceleration, braking, steering, and comparable speeds at 30/60 fps. GPU calls are replaced by validating mesh uploads; this does not test Android rendering or on-device touch input.

From the repository root, with a C++17 compiler:

```sh
c++ -std=c++17 -Itests/native/stubs -Iengine/core -Iengine/graphics -Iengine/math -Iengine/physics tests/native/geometry_test.cpp engine/core/Track.cpp engine/graphics/DominionCar.cpp engine/physics/Vehicle.cpp -o geometry_test
./geometry_test
```

On Windows, name the output `geometry_test.exe`. `zig c++` can be used in place of `c++`. Keep the executable outside the source tree or remove it after testing. The stubs are exclusively for host testing.
