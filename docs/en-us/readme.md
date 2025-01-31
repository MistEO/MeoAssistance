---
icon: ic:round-home
index: true
dir:
  order: 0
---

<!-- markdownlint-disable -->

::: center

![MAA Logo](/images/maa-logo_512x512.png =256x256)

<!-- markdownlint-restore -->
# MaaAssistantArknights

![C++](https://img.shields.io/badge/C++-20-%2300599C?logo=cplusplus)
![platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-blueviolet)
![license](https://img.shields.io/github/license/MaaAssistantArknights/MaaAssistantArknights) ![commit](https://img.shields.io/github/commit-activity/m/MaaAssistantArknights/MaaAssistantArknights?color=%23ff69b4)
![stars](https://img.shields.io/github/stars/MaaAssistantArknights/MaaAssistantArknights?style=social) ![GitHub all releases](https://img.shields.io/github/downloads/MaaAssistantArknights/MaaAssistantArknights/total?style=social)

MAA means MAA Assistant Arknights

An Arknights assistant

Based on image recognition, helps you to complete daily quests efficiently!

Development in progress  ✿✿ヽ(°▽°)ノ✿

:::

## Download and Install

Please read the [documentation](./manual/newbie.md) and then go to the [official website](https://maa.plus) or [Releases](https://github.com/MaaAssistantArknights/MaaAssistantArknights/releases) to download. Refer to [Getting Started](./manual/newbie.md) for installation instructions.

## Features

- Auto-battle, drop items recognition, auto-uploading to [Penguin Stats](https://penguin-stats.io/), [Yituliu](https://ark.yituliu.cn/).
- Auto-shifting in the Base, auto efficiency calculation of operators, optimal solution in a single room; also supports [customized schedule](./protocol/base-scheduling-schema.md).
- Auto-recruitment, support for expedited plan. Auto-uploading recruitment data to [Penguin Stats](https://penguin-stats.io/result/stage/recruit/recruit), [Yituliu](https://ark.yituliu.cn/survey/maarecruitdata).
- Supports manual identification of the public recruitment interface to facilitate selection of high-star public recruitment.
- Supports identifying operator lists, counting existing and unavailable operators and their potential, and displaying them in the open recruitment identification interface.
- Depot recognition and supports exporting to [Penguin Stats Planner](https://penguin-stats.cn/planner)、[Arknight Tools](https://arkntools.app/#/material)、[ARK-NIGHTS Operator Builds](https://ark-nights.com/settings).
- Visiting friends, collecting credits and purchasing items, collecting daily rewards, and completing daily quests in one click!
- Auto-battle for Integrated Strategy (I.S.) for collecting originium ingots and candles.
- Importing JSON task file for auto-battle! [Video](https://www.bilibili.com/video/BV1H841177Fk/)
- Support C, Python, Java, Rust, Golang, Java HTTP, Rust HTTP and other interfaces, easy to integrate and call, customize your MAA!

Talk is cheap. Show me the pictures!

```component Image4
{
  "imageList": [
    {
      "light": "images/en-us/readme/1-light.png",
      "dark": "images/en-us/readme/1-dark.png"
    },
    {
      "light": "images/en-us/readme/2-light.png",
      "dark": "images/en-us/readme/2-dark.png"
    },
    {
      "light": "images/en-us/readme/3-light.png",
      "dark": "images/en-us/readme/3-dark.png"
    },
    {
      "light": "images/en-us/readme/4-light.png",
      "dark": "images/en-us/readme/4-dark.png"
    }
  ]
}
```

## How to Use

### Basic Settings

Check [Beginner's Guide](./manual/newbie.md) and [User Manual](./manual/introduction/).
::: tip
For YostarEN, due to interface layout issues, the only supported resolution is `1920x1080`
:::

### Features

Please refer to the [User Manual](./manual/).

### Supports for overseas clients

At present, most of the functions of the International client (US client), Japanese client, Korean client, and traditional Chinese client have been supported. However, due to the small number of overseas users and the shortage of project personnel, many functions have not been fully tested, so please experience it yourself.  
If you encounter a bug or have a strong demand for a certain function, welcome to [Issues](https://github.com/MaaAssistantArknights/MaaAssistantArknights/issues) and [Discussions](https://github.com/MaaAssistantArknights/MaaAssistantArknights/discussions) to ask questions. And feel free to join us in building the MAA! Please refer to [Overseas Clients Adaptation](#overseas-clients-adaptation)

### CLI Support

MAA supports command line interface (CLI) operation, supports Linux, macOS and Windows, and can be used for automated scripts or on servers without graphical interfaces. Check [CLI Guide](./manual/cli/)

## Join us

### Associated Projects

**Currently, the project team is very short of front-end experts. If you have relevant experience, you are welcome to join us!**

- New Framework: [MaaFramework](https://github.com/MaaXYZ/MaaFramework)
- [Co-pilot (auto-battle) site](https://prts.plus) Frontend: [maa-copilot-frontend](https://github.com/MaaAssistantArknights/maa-copilot-frontend)
- [Co-pilot (auto-battle) site](https://prts.plus) Backend: [MaaBackendCenter](https://github.com/MaaAssistantArknights/MaaBackendCenter)
- [Website](https://www.maa.plus): [Frontend](https://github.com/MaaAssistantArknights/MaaAssistantArknights/tree/dev/website)
- Deep Learning: [MaaAI](https://github.com/MaaAssistantArknights/MaaAI)

### i18n

MAA supports multiple languages and uses Weblate for localization management. If you're multilingual, feel free to help us translate at [MAA Weblate](https://weblate.maa-org.net).

MAA uses Chinese (Simplified) as its first language, and all translation entries are in Chinese (Simplified).

[![Weblate](https://weblate.maa-org.net/widget/maa/wpf-gui/multi-auto.svg)](https://weblate.maa-org.net/engage/maa/)

### Development Related Information

#### Windows

Check [Development Guide](./develop/development.md).

#### Linux | MacOS

Check [Linux Compiling Tutorial](./develop/linux-tutorial.md)

#### API

- [C interface](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/dev/include/AsstCaller.h): [Integration Example](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/dev/src/Cpp/main.cpp)
- [Python interface](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/dev/src/Python/asst/asst.py): [Integration Example](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/dev/src/Python/sample.py)
- [Golang interface](https://github.com/MaaAssistantArknights/MaaAssistantArknights/tree/dev/src/Golang): [Integration Example](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/dev/src/Golang/maa/maa.go)
- [Dart interface](https://github.com/MaaAssistantArknights/MaaAssistantArknights/tree/dev/src/Dart)
- [Java interface](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/dev/src/Java/src/main/java/com/iguigui/maaj/easySample/MaaCore.java): [Integration Example](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/dev/src/Java/src/main/java/com/iguigui/maaj/easySample/MaaJavaSample.java)
- [Java HTTP interface](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/dev/src/Java/Readme.md)
- [Rust interface](https://github.com/MaaAssistantArknights/MaaAssistantArknights/tree/dev/src/Rust/src/maa_sys): [HTTP interface](https://github.com/MaaAssistantArknights/MaaAssistantArknights/tree/dev/src/Rust)
- [TypeScript interface](https://github.com/MaaAssistantArknights/MaaX/tree/main/packages/main/coreLoader)
- [Woolang interface](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/dev/src/Woolang/maa.wo)：[Integration Example](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/dev/src/Woolang/demo.wo)
- [Integration Documentation](./protocol/integration.md)
- [Callback Schema](./protocol/callback-schema.md)
- [Task Schema](./protocol/task-schema.md)
- [Copilot Schema](./protocol/copilot-schema.md)

#### Overseas Clients Adaptation

Please refer to [Overseas clients Adaptation](./develop/overseas-client-adaptation.md). For the functions already supported by the CN client, most of the overseas client's adaptation tasks only need screenshots and simple JSON modification.

#### For Novice Users of GitHub

Check [Introduction to Github Pull request Flow](./develop/development.md#introduction-to-github-pull-request-flow).

#### Issue Bot

Check [Issue Bot Usage](./develop/issue-bot-usage.md).

## Acknowledgements

### Open-source Libraries

- Image recognition: [opencv](https://github.com/opencv/opencv.git)
- ~~OCR: [chineseocr_lite](https://github.com/DayBreak-u/chineseocr_lite.git)~~
- OCR: [PaddleOCR](https://github.com/PaddlePaddle/PaddleOCR)
- ML Deployment: [FastDeploy](https://github.com/PaddlePaddle/FastDeploy)
- ML accelerator: [onnxruntime](https://github.com/microsoft/onnxruntime)
- ~~Item drop recognition: [Penguin Stats recognizer](https://github.com/penguin-statistics/recognizer)~~
- Map tile recognition: [Arknights-Tile-Pos](https://github.com/yuanyan3060/Arknights-Tile-Pos)
- C++ JSON library: [meojson](https://github.com/MistEO/meojson.git)
- C++ operator parser: [calculator](https://github.com/kimwalisch/calculator)
- ~~C++ Base64 encoding/decoding[cpp-base64](https://github.com/ReneNyffenegger/cpp-base64)~~
- C++ ZIP library: [zlib](https://github.com/madler/zlib)
- C++ Gzip library: [gzip-hpp](https://github.com/mapbox/gzip-hpp)
- Touch event producer for Android: [Minitouch](https://github.com/DeviceFarmer/minitouch)
- Touch event producer for Android: [MaaTouch](https://github.com/MaaAssistantArknights/MaaTouch)
- WPF MVVM framework: [Stylet](https://github.com/canton7/Stylet)
- WPF control library: [HandyControl](https://github.com/HandyOrg/HandyControl) -> [HandyControls](https://github.com/ghost1372/HandyControls)
- C# Log：[Serilog](https://github.com/serilog/serilog)
- C# JSON library: [Newtonsoft.Json](https://github.com/JamesNK/Newtonsoft.Json) & [System.Text.Json](https://github.com/dotnet/runtime)
- ~~Downloader: [aria2](https://github.com/aria2/aria2)~~

### Data Source

- ~~Recruitment data: [ArkTools](https://www.bigfun.cn/tools/aktools/hr)~~
- ~~Operators and infrastructure data: [PRTS Arknights Wiki (Chinese)](http://prts.wiki/)~~
- Stage data: [Penguin Stats](https://penguin-stats.io/)
- Gamedata and resources: [Arknights Game Resource](https://github.com/yuanyan3060/ArknightsGameResource)
- Gamedata: [Arknights Yostar Game Data](https://github.com/Kengxxiao/ArknightsGameData_YoStar)

### Contributors

Thanks to all friends who contribute to development/testing for making MAA better! (\*´▽｀)ノノ

[![Contributors](https://contributors-img.web.app/image?repo=MaaAssistantArknights/MaaAssistantArknights&max=105&columns=15)](https://github.com/MaaAssistantArknights/MaaAssistantArknights/graphs/contributors)

## Disclaimer

- This software is open source under the [GNU Affero General Public License v3.0 only](https://spdx.org/licenses/AGPL-3.0-only.html).
- The logo of this software is NOT granted rights under AGPL 3.0 License. [耗毛](https://weibo.com/u/3251357314) and Vie, the two artists and all developers of the software reserves all rights. The logo of the software shall not be used without authorization even if the project has an AGPL 3.0 License. Nor shall the logo be used for commercial purposes without authorization.
- The software is an open-source, free of charge software only for studying and communication purposes. There is no agreement or understanding between the developers of the software and the 3rd party person who uses this software as an assistant and charges you. In that case, the developers of the software are not responsible for the problems and consequences caused.

### DirectML Support Information

This software supports GPU acceleration, which relies on Microsoft's DirectML (Microsoft.AI.DirectML). For user convenience, an unmodified DirectML.dll file is included with the installation package.

#### About DirectML.dll

- Source: Official Microsoft
- License: Please refer to Microsoft's DirectML terms of use  
  [DirectML Official Documentation](https://learn.microsoft.com/en-us/windows/ai/directml/)

DirectML.dll is a standalone component provided by Microsoft. It is not part of this software's open-source section and is not subject to the AGPL-3.0 license.

If you do not require GPU support, you can safely delete the DLL file, and the core functionality of the software will remain unaffected.

## Advertisement

Discord Server: [Discord Invite](https://discord.gg/23DfZ9uA4V)  
User Group: [Telegram](https://t.me/+Mgc2Zngr-hs3ZjU1), [QQ Group](https://ota.maa.plus/MaaAssistantArknights/api/qqgroup/index.html)  
Copilot JSON Sharing: [prts.plus](https://prts.plus)  
Bilibili Livestreams: [MrEO Livestream](https://live.bilibili.com/2808861) Coding Sessions & [MAA-Official Livestream](https://live.bilibili.com/27548877) Gaming/Chats  
[Technical Discussion (QQ Group)](https://jq.qq.com/?_wv=1027&k=ypbzXcA2)  
[Dev Group(QQ Group)](https://jq.qq.com/?_wv=1027&k=JM9oCk3C)

Please click "Star" if you consider it helpful! Thank you for your support!
