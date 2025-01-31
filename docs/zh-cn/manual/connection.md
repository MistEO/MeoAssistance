---
order: 3
icon: mdi:plug
---

# 连接设置

## 自动检测

MAA 可以通过当前**正在运行的单个模拟器**自动检测并填充 ADB 路径、连接地址和连接配置。

截止至 MAA v5.7.0 支持检测的模拟器及连接地址如下：

- BlueStacks 5：`127.0.0.1:5555/5556/5565/5575/5585/5595/5554`
- MuMu 模拟器 12：`127.0.0.1:16384/16416/16448/16480/16512/16544/16576`
- 雷电模拟器 9：`emulator-5554/5556/5558/5560`, `127.0.0.1:5555/5557/5559/5561`
- 夜神模拟器：`127.0.0.1:62001/59865`
- 逍遥模拟器：`127.0.0.1:21503`

若检测失败，请尝试使用 UAC 管理员权限启动 MAA 并再次检测。若仍失败，则请参考下文手动设置，并确认模拟器和连接地址是否包含在上述列表中。

## ADB 路径

:::info 技术细节
自动检测使用的是模拟器的 ADB，但有时自动检测会出现问题，此时就需要手动设置。
`强制替换 ADB` 是下载谷歌提供的 ADB 后再进行替换，如果自己设置谷歌的 ADB 即可一劳永逸。
:::

### 使用模拟器提供的 ADB

前往模拟器安装路径，Windows 可在模拟器运行时在任务管理器中右键进程点击 `打开文件所在的位置`。

顶层或下层目录中应该会有一个名字中带有 `adb` 的 exe 文件，可以使用搜索功能，然后选择。

:::details 一些例子
`adb.exe` `HD-adb.exe` `adb_server.exe` `nox_adb.exe`
:::

### 使用谷歌提供的 ADB

[点击下载](https://dl.google.com/android/repository/platform-tools-latest-windows.zip)后解压，然后选择其中的 `adb.exe`。

推荐直接解压到 MAA 文件夹下，这样可以直接在 ADB 路径中填写 `.\platform-tools\adb.exe`，也可以随着 MAA 文件夹一起移动。

## 连接地址

::: tip
运行在本机的模拟器连接地址应该是 `127.0.0.1:<端口号>` 或 `emulator-<四位数字>`。
:::

### 模拟器相关文档及参考地址

- [Bluestacks 5](https://support.bluestacks.com/hc/zh-tw/articles/360061342631-%E5%A6%82%E4%BD%95%E5%B0%87%E6%82%A8%E7%9A%84%E6%87%89%E7%94%A8%E5%BE%9EBlueStacks-4%E8%BD%89%E7%A7%BB%E5%88%B0BlueStacks-5#%E2%80%9C2%E2%80%9D) `127.0.0.1:5555`
- [MuMu 模拟器 Pro](https://mumu.163.com/mac/function/20240126/40028_1134600.html) `127.0.0.1:16384`
- [MuMu 模拟器 12](https://mumu.163.com/help/20230214/35047_1073151.html) `127.0.0.1:16384`
- [雷电模拟器 9](https://help.ldmnq.com/docs/LD9adbserver) `emulator-5554`
- [夜神模拟器](https://support.yeshen.com/zh-CN/qt/ml) `127.0.0.1:62001`
- [逍遥模拟器](https://bbs.xyaz.cn/forum.php?mod=viewthread&tid=365537) `127.0.0.1:21503`

其他模拟器可参考 [赵青青的博客](https://www.cnblogs.com/zhaoqingqing/p/15238464.html)。

### 获取多开端口

- MuMu 12 多开器右上角可查看正在运行的多开端口。
- Bluestacks 5 模拟器设置内可查看当前的多开端口。
- _待补充_

::: details 备选方案

- 方案 1 : 使用 ADB 命令查看模拟器端口

  1. 启动**一个**模拟器，并确保没有其他安卓设备连接在此计算机上。
  2. 在存放有 ADB 可执行文件的文件夹中启动终端。
  3. 执行以下命令。

  ```sh
  # Windows 命令提示符
  adb devices
  # Windows PowerShell
  .\adb devices
  ```

  以下为输出内容的例子：

  ```text
  List of devices attached
  127.0.0.1:<端口号>   device
  emulator-<四位数字>  device
  ```

  使用 `127.0.0.1:<端口>` 或 `emulator-<四位数字>` 作为连接地址。

- 方案 2 : 查找已建立的 ADB 连接

  1. 执行方案 1。
  2. 按 `徽标键+S` 打开搜索栏，输入 `资源监视器` 并打开。
  3. 切换到 `网络` 选项卡，在 `侦听端口` 的名称列中查找模拟器进程名，如 `HD-Player.exe`。
  4. 记录模拟器进程的所有侦听端口。
  5. 在 `TCP 连接` 的名称列中查找 `adb.exe`，在远程端口列中与模拟器侦听端口一致的端口即为模拟器调试端口。

:::

### 蓝叠模拟器 Hyper-V 每次启动端口号都不一样

在 `连接设置` 中设置 `连接配置` 为 `蓝叠模拟器` ，随后勾选 `自动检测连接` 和 `每次重新检测`。

通常情况下这样就可以连接。如果无法连接，可能是存在多个模拟器核心或出现了问题，请阅读下文进行额外设置。

#### 指定 `Bluestacks.Config.Keyword`

::: info 注意
如果启用了多开功能或安装了多个模拟器核心，则需要进行额外设置来指定使用的模拟器编号
:::

在 `.\config\gui.json` 中搜索 `Bluestacks.Config.Keyword` 字段，内容为 `"bst.instance.<模拟器编号>.status.adb_port"`，模拟器编号可在模拟器路径的 `BlueStacks_nxt\Engine` 中看到

::: details 示例
Nougat64 核心：

```json
"Bluestacks.Config.Keyword":"bst.instance.Nougat64.status.adb_port",
```

Pie64_2 核心：（核心名称后的数字代表这是一个多开核心）

```json
"Bluestacks.Config.Keyword": "bst.instance.Pie64_2.status.adb_port",
```

:::

#### 指定 `Bluestacks.Config.Path`

::: info 注意
MAA 现在会尝试从注册表中读取 `bluestacks.conf` 的存储位置，当该功能无法工作时，则需要手动指定配置文件路径
:::

1. 在蓝叠模拟器的数据目录下找到 `bluestacks.conf` 这个文件

   - 国际版默认路径为 `C:\ProgramData\BlueStacks_nxt\bluestacks.conf`
   - 中国内地版默认路径为 `C:\ProgramData\BlueStacks_nxt_cn\bluestacks.conf`

2. 如果是第一次使用，请运行一次 MAA，使 MAA 自动生成配置文件。

3. **先关闭** MAA，**然后**打开 `gui.json`，找到 `Configurations` 下的当前配置名字段（可在 设置-切换配置 中查看，默认为 `Default`），在其中搜索字段 `Bluestacks.Config.Path`，填入 `bluestacks.conf` 的完整路径。（注意斜杠要用转义 `\\`）

::: details 示例
以 `C:\ProgramData\BlueStacks_nxt\bluestacks.conf` 为例

```json
{
  "Configurations": {
    "Default": {
      "Bluestacks.Config.Path": "C:\\ProgramData\\BlueStacks_nxt\\bluestacks.conf"
      // 其余配置字段，不要手动输入修改
    }
  }
}
```

:::

## 连接配置

需选择对应模拟器的配置，若列表中没有则选择通用配置。若通用配置不可用请尝试并选择其他任一可用的配置。

具体区别可以阅读[源码](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/dev/resource/config.json#L65)。

### MuMu 截图增强模式

需使用官版或方舟专版 MuMu 12 V4.0.0 及更新版本，国际版暂不支持。<!-- 官版 V3.8.13 支持初版截图增强 -->

1. `设置` - `连接设置`，勾选 `启用 MuMu 截图增强模式`，MAA 会在勾选开关时自动尝试通过注册表获取安装路径。

2. `MuMu 安装路径` 填写 `MuMuPlayer-12.0` 或 `YXArkNights-12.0` 文件夹的路径，如 `C:\Program Files\Netease\MuMuPlayer-12.0`。

3. 若正在使用 MuMu 网络桥接，则需勾选 `MuMu 网络桥接模式` 后手动填写 MuMu 多开器内对应模拟器的序号，如主多开为 `0`。

### 雷电截图增强模式

需使用官版或国际版雷电模拟器 9 V9.1.32 及更新版本。<!-- 官版 V9.0.78 支持初版截图增强 但存在高分辨率失效问题 V9.1.29 修复-->

1. `设置` - `连接设置`，勾选 `启用 LD 截图增强模式`，MAA 会在勾选开关时自动尝试通过注册表获取安装路径。

2. `LD 安装路径` 填写 `LDPlayer9` 文件夹的路径，如 `C:\leidian\LDPlayer9\`。

3. `实例编号` 填写雷电多开器内对应模拟器的编号（ID），如主多开为 `0`。

## 触控模式

1. [Minitouch](https://github.com/DeviceFarmer/minitouch)：使用 C 编写的 Android 触控事件器，操作 `evdev` 设备，提供 Socket 接口供外部程序触发触控事件和手势。从 Android 10 开始，Minitouch 在 SELinux 为 `Enforcing` 模式时不再可用。<sup>[源](https://github.com/DeviceFarmer/minitouch?tab=readme-ov-file#for-android-10-and-up)</sup>
2. [MaaTouch](https://github.com/MaaAssistantArknights/MaaTouch)：由 MAA 基于 Java 对 Minitouch 的重新实现，使用安卓原生的 `InputDevice`，并添加了额外特性。高版本 Android 可用性尚待测试。~~帮我们做做测试~~
3. Adb Input：直接调用 ADB 使用安卓的 `input` 命令进行触控操作，兼容性最强，速度最慢。

## ADB Lite

由 MAA 独立实现的 ADB Client，使用 TCP 直接与 ADB Server 通信。相较原版 ADB 可以避免不停开启多个 ADB 进程，减少性能开销，但部分截图方式不可用。<sup>[PR](https://github.com/MaaAssistantArknights/MaaAssistantArknights/pull/3315)</sup>

推荐启用，但具体优缺点尚待反馈。~~帮我们做做测试 x2~~

## MAA 和模拟器多开

::: info 如何实现
若需要多开模拟器同时操作，可将 MAA 文件夹复制多份，使用 **不同的 MAA**、**同一个 adb.exe**、**不同的连接地址** 来进行连接。
:::

### 自动启动多开模拟器

以[蓝叠国际版](./device/windows.md)为例，介绍两种自动启动多开模拟器的方式。

#### 通过附加命令启动

1. 启动**单一**模拟器多开。
2. 打开任务管理器，找到对应模拟器进程，转到详细信息选项卡，右键列首，点击 `选择列`，勾选 `命令行`。
3. 在多出来的 `命令行` 列中找到 `...\Bluestacks_nxt\HD-Player.exe"` 后的内容。
4. 将找到的类似于 `--instance Nougat32` 的内容填写到 `启动设置` - `附加命令` 中。

::: note
操作结束后建议重新隐藏 `步骤 2` 中打开的 `命令行` 列以防止卡顿
:::

::: details 示例

```text
多开1:
模拟器路径: C:\Program Files\BlueStacks_nxt\HD-Player.exe
附加命令: --instance Nougat32 --cmd launchApp --package "com.hypergryph.arknights"
多开2:
模拟器路径: C:\Program Files\BlueStacks_nxt\HD-Player.exe
附加命令: --instance Nougat32_1 --cmd launchApp --package "com.hypergryph.arknights.bilibili"
```

其中 `--cmd launchApp --package` 部分为启动后自动运行指定包名应用，可自行更改。
:::

#### 通过模拟器的快捷方式启动

部分模拟器支持创建应用快捷方式，可直接使用应用的快捷方式直接启动模拟器并打开明日方舟。

1. 打开多开管理器，新增对应模拟器的快捷方式。
2. 将模拟器快捷方式的路径填入 `启动设置` - `模拟器路径` 中

::: details 示例

```text
多开1:
模拟器路径: C:\ProgramData\Microsoft\Windows\Start Menu\Programs\BlueStacks\多开1.lnk
多开2:
模拟器路径: C:\ProgramData\Microsoft\Windows\Start Menu\Programs\BlueStacks\多开2-明日方舟.lnk
```

:::

若使用 `模拟器路径` 进行多开操作，建议将 `启动设置` - `附加命令` 置空。
