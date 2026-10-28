# Getting Started with Racing Dominion

## Edit, build on GitHub, install on your phone

Before the first release build, configure the four repository signing secrets using the [release signing guide](RELEASE_SIGNING.md). Push and manual runs produce signed release APKs; pull requests produce unsigned validation APKs.

Android Studio is optional. Edit in VS Code, push to `main` or `develop`, and use **Actions > Racing Dominion Release APK** in [the repository](https://github.com/CheFu-code/Racing-Dominion/actions).

Download the completed run's `racing-dominion-release-apk` artifact, extract it, and install `app-release.apk` on your phone. Allow installation from that source if prompted. A code change requires a new build and installation.

## Local command-line builds

Install JDK 17, Android SDK platform/build-tools 34, NDK 26.1.10909125, CMake 3.22.1, and Android platform-tools. Configure `ANDROID_HOME` or `local.properties` with your SDK location. CI downloads the Gradle 8.2 wrapper JAR; local builds also require `gradle/wrapper/gradle-wrapper.jar`.

From the repository root in PowerShell:

```powershell
.\gradlew.bat assembleDebug
```

With USB debugging enabled and your phone connected:

```powershell
adb devices
.\gradlew.bat installDebug
```

Launch Racing Dominion on your phone after installation. The application ID is `co.za.chefu.racingdominion`.

## Where to edit

The [README](../README.md#customize) maps the visual and gameplay systems to their source files. Active shaders are embedded in `engine/graphics/Renderer.cpp`; files under `shaders/` are not the active rendering path.

The default game uses the authored Crownline Circuit and procedural RD-01 car. Legacy sample model assets are not used for the player's car.

## Device checks after visual changes

- Start a race and confirm the menu disappears.
- Check the timer, speed, lap display, and control zones on your screen size.
- Drive the four course gates in order; the start line should count a lap only after the other gates.
- Complete three laps and tap the results screen to restart.
- Background and reopen the app, checking rendering and touch input.

## Logs

```powershell
adb logcat -s RacingEngine Track Renderer
```
