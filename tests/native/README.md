# Race, geometry, and controls checks

The native tests run the actual race state machine, sector rules, course and RD-01 generators, vehicle physics, and smoke simulation. They check countdown/pause timing, medal thresholds and gate order, swept forward gate crossings, course continuity, triangle validity, barrier recovery, braking, steering, comparable speeds at 30/60 fps, particle expiry, and resource cleanup.

GPU calls and Android platform integration are replaced by host stubs. These tests do not render the game or play audio on Android.

From the repository root, with a C++17 compiler:

```sh
c++ -std=c++17 -Itests/native/stubs -Iengine/core -Iengine/graphics -Iengine/math -Iengine/physics -Iengine/assets tests/native/geometry_test.cpp engine/core/Track.cpp engine/core/Engine.cpp engine/graphics/DominionCar.cpp engine/graphics/Camera.cpp engine/physics/Vehicle.cpp -o geometry_test
./geometry_test
```

On Windows, name the output `geometry_test.exe`. `zig c++` can replace `c++`. Write the executable outside the source tree. The stubs are exclusively for host testing and must never enter Android's include path.

For plain-Java pointer-ownership and immutable-telemetry checks, use a temporary output directory:

```sh
javac -d /tmp/dominion-java-check app/src/main/java/com/racing/engine/TouchControls.java app/src/main/java/com/racing/engine/RaceSnapshot.java tests/java/TouchControlsTest.java
java -cp /tmp/dominion-java-check co.za.chefu.racingdominion.TouchControlsTest
```

On Windows, replace `/tmp/dominion-java-check` with a directory under your temporary folder. Both checks run before the release build in GitHub Actions. Follow [device checks](../../docs/GETTING_STARTED.md#device-checks-after-changes) for rendering, live input, audio, and lifecycle validation.
