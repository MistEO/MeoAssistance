---
order: 3
icon: mdi:plug
---

# Connection

## Automatic Detection

MAA can automatically detect and fill in the ADB path, connection address, and connection present through the **currently running single emulator**.

As of MAA v5.7.0, the supported emulators and connection addresses for detection are as follows:

- BlueStacks 5: `127.0.0.1:5555/5556/5565/5575/5585/5595/5554`
- MuMu Player 12: `127.0.0.1:16384/16416/16448/16480/16512/16544/16576`
- LDPlayer 9: `emulator-5554/5556/5558/5560`, `127.0.0.1:5555/5557/5559/5561`
- NoxPlayer: `127.0.0.1:62001/59865`
- Memu Play: `127.0.0.1:21503`

If detection fails, try launching MAA with UAC administrator privileges and retry detection. If it still fails, please refer to the manual setup below and verify whether the emulator and connection address are included in the list above.

## ADB Path

:::info Technical Details
Automatic detection uses the emulator's ADB, but sometimes problems occur with automatic detection, then manual settings are required.
`Replace ADB` is to download the ADB provided by Google and then replace it. If you set up Google's ADB yourself, you can do it once and for all.
:::

### Use the ADB provided by the emulator

Go to the emulator installation path. In Windows, when the emulator is running, right-click the process in the Task Manager and click `Open the location of the file`.

There should be an exe file with `adb` in its name in the top-level or lower directory. You can search and select it.

:::details Examples
`adb.exe` `HD-adb.exe` `adb_server.exe` `nox_adb.exe`
:::

### Use ADB provided by Google

[Click](https://dl.google.com/android/repository/platform-tools-latest-windows.zip) to download and unzip, then select `adb.exe`.

We are recommended to extract it directly to the MAA folder, so that you can directly fill in `.\platform-tools\adb.exe` in the ADB path, and move it with MAA.

## Connection Address

::: tip
The connection address of the emulator running on this computer should be `127.0.0.1:<port>` 或 `emulator-<four numbers>`。
:::

### Emulator documents and addressess for reference

:::warning
The following links are Chinese websites, cuz no English websites were found. Maybe.
:::

- [Bluestacks 5](https://support.bluestacks.com/hc/zh-tw/articles/360061342631-%E5%A6%82%E4%BD%95%E5%B0%87%E6%82%A8%E7%9A%84%E6%87%89%E7%94%A8%E5%BE%9EBlueStacks-4%E8%BD%89%E7%A7%BB%E5%88%B0BlueStacks-5#%E2%80%9C2%E2%80%9D) `127.0.0.1:5555`
- [MuMu Player Pro](https://mumu.163.com/mac/function/20240126/40028_1134600.html) `127.0.0.1:16384`
- [MuMu Player 12](https://mumu.163.com/help/20230214/35047_1073151.html) `127.0.0.1:16384`
- [LD Player 9](https://help.ldmnq.com/docs/LD9adbserver) `emulator-5554`
- [NoxPlayer](https://support.yeshen.com/zh-CN/qt/ml) `127.0.0.1:62001`
- [Memu Play](https://bbs.xyaz.cn/forum.php?mod=viewthread&tid=365537) `127.0.0.1:21503`

For other emulators, please check [Zhao Qingqing's blog](https://www.cnblogs.com/zhaoqingqing/p/15238464.html).

### Obtain Port Number

- MuMu 12: The running emulator ports can be found in the upper right corner of the MultiPlayer.
- Bluestacks 5: The current emulator ports can be viewed in the emulator settings.
- _TODO_

::: details Alternatives

- Method 1: Use the adb command to view the running port directly

  1. Launch **one** emulator and make sure no other Android devices are connected to this computer.
  2. Launch a terminal in the folder where the adb executable is.
  3. Execute the following command.

  ```sh
  # Windows CMD
  adb devices
  # Windows PowerShell
  .\adb devices
  ```

  An example of output:

  ```text
  List of devices attached
  127.0.0.1:<port>   device
  emulator-<four numbers>  device
  ```

  Use `127.0.0.1:<port>` or `emulator-<four numbers>` as the connection address.

- Method 2: Find established adb connections

  1. Do method 1.
  2. Press `Win+S` to open the search bar, type `Resource Monitor` and open it.
  3. Switch to the `Network` tab and look for the emulator process name in the name column of `Listening Port`, such as `HD-Player.exe`.
  4. Make a note of all listening ports for the emulator process.
  5. Find `adb.exe` in the name column of `TCP connection`. The port in the remote port column that is consistent with the emulator listening port is the emulator debugging port.

:::

### Automatically Start Multiple Emulators

- If you need to operate multiple emulators simultaneously, you can copy the MAA folder multiple times, and use **different MAAs**, **the same adb.exe**, and **different connection addresses** to connect.
- Taking `BlueStacks International Version` as an example, two ways to start multiple emulators are introduced.

  - Perform multiple operations by attaching commands to `HD-Player.exe`.

    1. Start the corresponding emulator separately.
    2. Open the Task Manager, find the corresponding emulator process, go to the Details tab, right-click the column above, click `Select Columns`, and check `Command Line`.
    3. In the newly added `Command Line` column, find the content after `"...\Bluestacks_nxt\HD-Player.exe"`.
    4. Fill in the found content, similar to `--instance Nougat32`, in `Startup Settings` - `Additional Commands`.

    Note: After the operation is completed, it is recommended to hide the `Command Line` column opened in `Step 2` to prevent freezing.

    - Example:

      ```bash
      Multi-instance 1:
      Emulator Path: C:\Program Files\BlueStacks_nxt\HD-Player.exe
      Additional Commands: --instance Nougat32 --cmd launchApp --package "com.hypergryph.arknights"
      Multi-instance 2:
      Emulator Path: C:\Program Files\BlueStacks_nxt\HD-Player.exe
      Additional Commands: --instance Nougat32_1 --cmd launchApp --package "com.hypergryph.arknights.bilibili"
      ```

      The `--cmd launchApp --package` part starts and automatically runs the specified package name application after startup, which can be changed as needed.

  - Perform multi-instance operation by using the shortcut of emulators or apps.

    1. Open the multi-instance manager and add the corresponding emulator's shortcut.
    2. Fill in the path of the emulator shortcut in `Startup Settings` - `Emulator Path`.

    Note: Some emulators support creating app shortcuts, which can directly launch the emulator and open Arknights with the app shortcut.

    - Example:

      ```bash
      Multi-instance 1:
      Emulator Path: C:\ProgramData\Microsoft\Windows\Start Menu\Programs\BlueStacks\Multi-instance 1.lnk
      Multi-instance 2:
      Emulator Path: C:\ProgramData\Microsoft\Windows\Start Menu\Programs\BlueStacks\Multi-instance 2 - Arknights.lnk
      ```

    If using `Emulator Path` for multi-instance operation, it is recommended to leave `Additional Commands` in `Startup Settings` empty.

### The port number of BlueStack emulator Hyper-V is different every time it is started

:::warning
TODO: replace these Chinese emulators links with English ones
:::

## Connection Present

You need to select the configuration of the emulator you are using. If it is not in the list, select General Mode. If General Mode is not available please try and select any of the other available presents.

For specific differences, you can read the [source code](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/dev/resource/config.json#L65).

### MuMu Screenshot Enhanced Mode

Only Chinese or Arknights Edition MuMu 12 V4.0.0 and later versions are supported. Global Edition is not supported at this time. <!-- Chinese V3.8.13 -->

1. Settings → Connection Settings, enable `Enable MuMu's screenshot enhancement mode`. MAA will attempt to automatically fill in the exec path through the registry when the switch is checked.

2. `MuMu12 Emulator exec path` Fill in the path to the `MuMuPlayer-12.0` or `YXArkNights-12.0` folder. e.g. `C:\Program Files\Netease\MuMuPlayer-12.0`.

3. If using MuMu network bridging, check `MuMu network bridging mode` and manually enter the serial number of the corresponding emulator in MuMu Multi-Instance Manager, such as `0` for the main instance.

### LD Screenshot Enhanced Mode

Only Chinese or Global LD Player 9 V9.1.32 and later versions are supported. <!-- Chinese V9.0.78 -->

1. Settings → Connection Settings, enable `Enable LD's screenshot enhancement mode`. MAA will attempt to automatically fill in the exec path through the registry when the switch is checked.

2. `LD Emulator exec path` Fill in the path to the `LDPlayer9` folder. e.g. `C:\leidian\LDPlayer9\`。

3. `Instance Number` Fill in the serial number (ID) of the corresponding emulator in LD Multiplayer. e.g. `0` for Main Multiplayer.

## Touch Mode

1. [Minitouch](https://github.com/DeviceFarmer/minitouch)：An Android touch eventer written in C uses `evdev` to provide a Socket interface for external programs to trigger touch events and gestures. Starting with Android 10, Minitouch no longer works when SELinux is `Enforcing`.<sup>[Source](https://github.com/DeviceFarmer/minitouch?tab=readme-ov-file#for-android-10-and-up)</sup>
2. [MaaTouch](https://github.com/MaaAssistantArknights/MaaTouch)：A Java-based reimplementation of Minitouch by MAA, which uses Android's `InputDevice`, and added additional features. Availability of higher versions of Android is yet to be tested. ~~Help us do some testing~~
3. Adb Input：Directly call ADB to use Android's `input` command to perform touch operations. It has the highest compatibility and the slowest speed.

## ADB Lite

An ADB Client independently implemented by MAA uses TCP to communicate directly with the ADB Server. Compared with the original ADB, it can avoid opening multiple ADB processes continuously and reduce performance overhead, but some screenshot methods are unavailable.<sup>[PR](https://github.com/MaaAssistantArknights/MaaAssistantArknights/pull/3315)</sup>

It is recommended to enable it, but the specific advantages and disadvantages need feedback. ~~Help us do some testing x2~~
