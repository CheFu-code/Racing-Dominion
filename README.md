# Racing Dominion

An Android circuit-racing game built around **Crownline Circuit** and the **RD-01** prototype. Racing Dominion uses a custom C++17/OpenGL ES 3.0 engine with a native Android interface.

## The Dominion identity

- **Crownline Circuit:** an asymmetric 96-segment course with sweeping turns, lime kerbs, a chequered grid, low barriers, and a geometric skyline.
- **RD-01:** an original procedural car with a tapered body, dark canopy, rear wing, racing stripe, wheels, and light strips. The playable car no longer loads the upstream sample model.
- **Dusk palette:** midnight blue, graphite, ice white, and Dominion lime.
- **Race interface:** branded start screen, course preview, speed display, lap progress, timer, and results panel.
- **Launcher:** a crown above a racing straight, with adaptive and legacy icons.

This is a playable-game foundation under development. Vehicle handling, touch controls, performance, and visual balance still need testing and refinement on Android devices. There are no AI opponents, garage upgrades, multiplayer, or career progression yet.

## Build and play without Android Studio

Before the first release build, configure the four repository signing secrets using the [release signing guide](docs/RELEASE_SIGNING.md). Push and manual runs produce signed release APKs; pull requests produce unsigned validation APKs.

1. Edit the project in VS Code or your preferred editor.
2. Push to `main` or `develop` on [this repository](https://github.com/CheFu-code/Racing-Dominion).
3. Open **Actions > Racing Dominion Release APK** and select the completed run.
4. Download **racing-dominion-release-apk**, extract it, and install `app-release.apk` on your Android phone.
5. Repeat after changes. Each APK must be rebuilt and installed; this project has no live reload.

You can also run the workflow manually from its Actions page.

For local builds, install JDK 17 and Android SDK command-line tools, SDK platform/build-tools 34, NDK 26.1.10909125, and CMake 3.22.1. Set `ANDROID_HOME` to your SDK directory. The repository currently obtains the Gradle 8.2 wrapper JAR during CI; a local build needs that JAR too.

```powershell
.\gradlew.bat assembleDebug
# With a phone connected and USB debugging enabled:
.\gradlew.bat installDebug
```

The APK is written to `app/build/outputs/apk/debug/app-debug.apk`.

## Controls

- Tap anywhere on the start or results screen to race.
- Left half: touch higher to steer left, lower to steer right.
- Right half: touch higher to accelerate, lower to brake.

Control labels are guides to these zones. The current input implementation uses a single touch; simultaneous steering and pedal controls are future work.

## Customize

| Part | Source |
| --- | --- |
| Menus, course preview, HUD | `app/src/main/java/com/racing/engine/RacingHudView.java` |
| Circuit, kerbs, skyline | `engine/core/Track.cpp` |
| RD-01 body and details | `engine/graphics/DominionCar.cpp` |
| Sky and race state | `engine/core/Engine.cpp` |
| Lighting and active shaders | `engine/graphics/Renderer.cpp` |
| Vehicle handling | `engine/physics/Vehicle.cpp` |
| Adaptive launcher emblem | `app/src/main/res/drawable/ic_launcher_foreground.xml` |
| Legacy icon generator | `tools/generate_launcher_icons.py` |

The Java source folder retains its original directory layout, but the application ID and Java package are `co.za.chefu.racingdominion`. Keep native JNI names synchronized if changing the Java package.

To regenerate legacy launcher icons, run `python tools/generate_launcher_icons.py` with Pillow installed, or trigger **Generate Racing Dominion Icons** in Actions. Keep the generator's polygon coordinates aligned with the adaptive vector.

The menu course preview mirrors the circuit formula. Update both when changing the layout.

## Development direction

Next improvements: responsive multitouch controls and handling, then saved personal bests and ghost time trials. Additional cars, circuits, AI rivals, and progression should follow device testing of the core race experience.

## Credits and license

Racing Dominion is derived from Subhobhai's MIT-licensed Android 3D Racing Game engine. The original copyright notice and license remain in [LICENSE](LICENSE). Its inherited sample assets remain in the repository for reference; the default car and circuit now use Racing Dominion's procedural geometry.

See [architecture](docs/ARCHITECTURE.md) and [asset documentation](docs/ASSETS.md) for the underlying engine. Some inherited technical documentation describes the earlier prototype.
