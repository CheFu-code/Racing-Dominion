# Racing Dominion

An Android circuit-racing game built around **Crownline Circuit** and the **RD-01** prototype. Racing Dominion uses a custom C++17/OpenGL ES 3.0 engine with a native Android interface.

## The Dominion identity

- **Crownline Circuit:** an asymmetric 96-segment course with sweeping turns, orange kerbs, two illuminated bridge gantries, chevron murals, an industrial skyline, and the Foundry Spire landmark.
- **RD-01:** an original procedural car with a tapered body, dark canopy, round tires and rims, rear wing, Foundry orange/ice-blue livery, signature headlights, and brake-reactive tail lights. The playable car no longer loads the upstream sample model.
- **Dusk palette:** electric orange, ice blue, deep plum, luminous track lighting, and distance haze.
- **Race interface:** countdown and GO cue, live sector targets and medals, speed and lap displays, accessible pause/sound buttons, results, and saved personal bests.
- **Launcher:** a custom angular RD monogram in electric orange and ice blue on deep plum, with adaptive and legacy icons.

This is a playable-game foundation under development. Vehicle handling, touch controls, performance, and visual balance still need testing and refinement on Android devices. The current mode is a solo sector challenge. AI opponents, garage upgrades, multiplayer, and a career campaign remain future work.

## Build and play without Android Studio

Before the first release build, configure the four repository signing secrets using the [release signing guide](docs/RELEASE_SIGNING.md). Push and manual runs produce signed release APKs; pull requests produce unsigned validation APKs.

1. Edit the project in VS Code or your preferred editor.
2. Push to `main` or `develop` on [this repository](https://github.com/CheFu-code/Racing-Dominion).
3. Open **Actions > Racing Dominion Release APK** and select the completed run.
4. Download **racing-dominion-release-apk**, extract it, and install `app-release.apk` on your Android phone.
5. Repeat after changes. Each APK must be rebuilt and installed; this project has no live reload.

You can also run the workflow manually from its Actions page.

For local builds, install JDK 17 and Android SDK command-line tools, SDK platform/build-tools 34, NDK 26.1.10909125, and CMake 3.22.1. Set `ANDROID_HOME` to your SDK directory. Install Gradle 8.2 for local command-line builds. CI installs that pinned distribution directly; the repository does not include the Gradle wrapper JAR.

```powershell
gradle assembleDebug
# With a phone connected and USB debugging enabled:
gradle installDebug
```

The APK is written to `app/build/outputs/apk/debug/app-debug.apk`.

## Claim the Circuit

Complete three laps through four ordered sectors per lap. Each sector has a course-length-based Gold target; Silver allows another 2.5 seconds and Bronze another 6 seconds. The live HUD displays all three target times. Cross a gate forward and within the road to bank that sector's medal. Reverse crossings and out-of-order gates do not score; barriers prevent cutting across the infield.

Gold/Silver/Bronze sectors award 3/2/1 points. Twelve sectors give a maximum of 36 points. Earn **Gold Dominion at 30+**, **Silver at 20+**, or **Bronze at 10+**. Missed targets award no points, but you can still complete the run and retry. Targets are an initial tuning pass that needs phone playtesting.

Your fastest completed run, highest Dominion rank, and sound preference are saved on the device. The fastest time and highest rank can come from different runs. Start a new run to reset the current medals and timer.

## Controls and presentation

- Tap the track from the menu to begin the three-second countdown.
- **Left thumb:** drag left/right around the bottom-left pad to steer. Releasing centers the steering.
- **Right thumb:** hold the upper right pedal to accelerate; slide to the lower pedal to brake.
- Use both thumbs simultaneously. Releasing one finger leaves the other control active.
- **PAUSE / RESUME** or Android Back pauses/resumes the run. Tap the track to resume from pause.
- Backgrounding pauses the race and releases all controls. Resume explicitly after returning.
- **SOUND / MUTED** controls original synthesized engine, tire scrub, collision, countdown, sector, and finish effects. Backgrounding or losing audio focus stops playback.

A speed-sensitive chase camera looks ahead into corners. Brief, bounded tire-smoke particles appear during hard braking or tire scrub. A barrier hit reduces speed and realigns the car with the road so it can recover.

## Customize

| Part | Source |
| --- | --- |
| Menus, course preview, HUD | `app/src/main/java/com/racing/engine/RacingHudView.java` |
| Circuit, kerbs, skyline | `engine/core/Track.cpp` |
| RD-01 body and details | `engine/graphics/DominionCar.cpp` |
| Race state, countdown, camera, smoke rendering | `engine/core/Engine.cpp` |
| Sector times, medals, Dominion ranks | `engine/core/CircuitChallenge.h` |
| Multitouch pointer ownership | `app/src/main/java/com/racing/engine/TouchControls.java` |
| Sound playback and audio focus | `app/src/main/java/com/racing/engine/AudioHelper.java` |
| Original sound synthesis | `tools/generate_audio.py` |
| Lighting and active shaders | `engine/graphics/Renderer.cpp` |
| Vehicle handling | `engine/physics/Vehicle.cpp` |
| Adaptive launcher emblem | `app/src/main/res/drawable/ic_launcher_foreground.xml` |
| Legacy icon generator | `tools/generate_launcher_icons.py` |

The Java source folder retains its original directory layout, but the application ID and Java package are `co.za.chefu.racingdominion`. Keep native JNI names synchronized if changing the Java package.

To regenerate legacy launcher icons, run `python tools/generate_launcher_icons.py` with Pillow installed, or trigger **Generate Racing Dominion Icons** in Actions. Keep the generator's polygon coordinates aligned with the adaptive vector.

The menu course preview mirrors the circuit formula. Update both when changing the layout.

## Development direction

Next: on-device handling, target-time and sound-balance tuning, followed by ghost time trials. Additional cars, circuits, rivals, and a career campaign should build on that tested race experience.

Host race/geometry tests and plain-Java multitouch tests run in CI. See [test commands](tests/native/README.md) and [device checks](docs/GETTING_STARTED.md#device-checks-after-changes). Regenerate the seven original WAV effects with `python tools/generate_audio.py` (standard library only).

## Credits and license

Racing Dominion is derived from Subhobhai's MIT-licensed Android 3D Racing Game engine. The original copyright notice and license remain in [LICENSE](LICENSE). The audio in `res/raw/rd_*.wav` is synthesized by this project's generator. Its inherited sample assets remain in the repository for reference; the default car and circuit now use Racing Dominion's procedural geometry.

See [architecture](docs/ARCHITECTURE.md) and [asset documentation](docs/ASSETS.md) for the underlying engine. Some inherited technical documentation describes the earlier prototype.
