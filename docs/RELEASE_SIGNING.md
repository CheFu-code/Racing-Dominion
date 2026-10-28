# Release APK signing without Android Studio

The **Racing Dominion Release APK** workflow builds a signed `app-release.apk` on pushes to `main`/`develop` and manual runs. Configure the four secrets below before running it. Missing secrets fail the build with their names; passwords and the key are not printed.

Pull requests build `app-release-unsigned.apk` without signing secrets. That artifact is for build validation and cannot be installed until signed.

## Create your key once

If you already have a release keystore for this app, reuse it. Otherwise, run this in PowerShell with JDK 17 installed:

```powershell
keytool -genkeypair -v -storetype PKCS12 -keystore "$env:USERPROFILE\racing-dominion-release.keystore" -alias racing-dominion -keyalg RSA -keysize 3072 -validity 10000
```

Enter your own keystore password and certificate details when prompted. This PKCS12 keystore uses the same password for the store and key. Back up the keystore and password: subsequent APK updates must use the same signing identity. Do not commit the keystore to Git.

Copy its Base64 representation to your clipboard:

```powershell
[Convert]::ToBase64String([IO.File]::ReadAllBytes("$env:USERPROFILE\racing-dominion-release.keystore")) | Set-Clipboard
```

## Add repository secrets

Open your GitHub repository's **Settings > Secrets and variables > Actions > New repository secret**.

| Secret | Value |
| --- | --- |
| `RELEASE_KEYSTORE_BASE64` | Paste the Base64 keystore from your clipboard. |
| `RELEASE_STORE_PASSWORD` | Your keystore password. |
| `RELEASE_KEY_ALIAS` | `racing-dominion`, or your existing key's alias. |
| `RELEASE_KEY_PASSWORD` | Your key password; same as the store password for the PKCS12 key above. |

Use repository **secrets**, not ordinary Actions variables. The workflow decodes the key into the runner's temporary directory, verifies the APK signature with `apksigner`, and removes the temporary keystore afterward.

## Build and install

Push to `main` or `develop`, or select **Actions > Racing Dominion Release APK > Run workflow**. Download **racing-dominion-release-apk**, extract it, and install `app-release.apk` on your phone.

A previously installed debug APK normally has a different signing certificate. Android cannot install this release over that debug build; uninstall the debug build first if Android reports a signature conflict. Uninstalling removes that app's local data.

Local debug development still uses `assembleDebug` / `installDebug`. For a signed local release, set `RELEASE_STORE_FILE`, `RELEASE_STORE_PASSWORD`, `RELEASE_KEY_ALIAS`, and `RELEASE_KEY_PASSWORD` in your shell and run `gradle assembleRelease` using Gradle 8.2. Without `RELEASE_STORE_FILE`, local release builds are unsigned.

See the official [Android app signing documentation](https://developer.android.com/studio/publish/app-signing) for signing identities and APK update requirements.
