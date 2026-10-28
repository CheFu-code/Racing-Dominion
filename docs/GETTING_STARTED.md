# Getting Started with Racing Dominion

## Edit, build on GitHub, install on your phone

Before the first release build, configure the four repository signing secrets using the [release signing guide](RELEASE_SIGNING.md). Push and manual runs produce signed release APKs; pull requests produce unsigned validation APKs.

Android Studio is optional. Edit in VS Code, push to `main` or `develop`, and use **Actions > Racing Dominion Release APK** in [the repository](https://github.com/CheFu-code/Racing-Dominion/actions).

Download the completed run's `racing-dominion-release-apk` artifact, extract it, and install `app-release.apk` on your phone. Allow installation from that source if prompted. A code change requires a new build and installation.

## Local command-line builds

Install JDK 17, Android SDK platform/build-tools 34, NDK 26.1.10909125, CMake 3.22.1, and Android platform-tools. Configure `ANDROID_HOME` or `local.properties` with your SDK location. Install Gradle 8.2 locally. CI installs the pinned Gradle distribution directly; the repository does not include the Gradle wrapper JAR.

From the repository root in PowerShell:

```powershell
gradle assembleDebug
```

With USB debugging enabled and your phone connected:

```powershell
adb devices
gradle installDebug
```

Launch Racing Dominion on your phone after installation. The application ID is `co.za.chefu.racingdominion`.

## Where to edit

The [README](../README.md#customize) maps the visual and gameplay systems to their source files. Active shaders are embedded in `engine/graphics/Renderer.cpp`; files under `shaders/` are not the active rendering path.

The default game uses the authored Crownline Circuit and procedural RD-01 car. Legacy sample model assets are not used for the player's car.

## Device checks after changes

- Check the orange/ice-blue/plum UI, illuminated gantries, chevron murals, Foundry Spire, and the RD-01's livery, round wheels, headlights, and brake lights.
- Start a run: confirm 3, 2, 1, GO; the race timer must exclude the countdown.
- Hold gas with the right thumb while steering with the left. Release each finger separately; the other control should remain active. Move the pedal finger to brake.
- Check chase-camera response at speed and brief tire smoke during hard braking/turning.
- Cross four gates forward and in order, checking sector times/medals. A reverse or skipped gate must not award a sector. Barrier contact should slow and realign the car, rather than allow an infield shortcut.
- Complete three laps. Check all twelve medals, points out of 36, Dominion rank, best lap, and personal-best banner. Gold requires 30 points, Silver 20, Bronze 10.
- Retry and confirm medals/time reset. Relaunch the app and confirm the saved best time/rank and sound setting remain.
- Check engine pitch, tire/collision effects, countdown beeps, sector cue, and finish jingle. Toggle mute; background the app or interrupt it with other audio to check sound stops.
- Pause during countdown and racing. Background and reopen the app: time and controls must stay paused until resumed. Check small-phone and tablet layouts, especially the HUD, real pause/sound buttons, and results panel.

These device checks are still required: host tests do not exercise Android GPU output, touch delivery, SoundPool, or audio focus on a real phone. If Android destroys the GL context while backgrounded, the current run resets to the menu; saved bests/settings remain.

## Logs

```powershell
adb logcat -s RacingEngine Track Renderer
```
