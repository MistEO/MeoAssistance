<!-- markdownlint-disable -->

<div align="center">

<img alt="LOGO" src="./docs/.vuepress/public/images/maa-logo_512x512.png" width="256" height="256" />

# MaaAssistantArknights

<br>
<div>
    <img alt="C++" src="https://img.shields.io/badge/C++-20-%2300599C?logo=cplusplus">
</div>
<div>
    <img alt="platform" src="https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-blueviolet">
</div>
<div>
    <img alt="license" src="https://img.shields.io/github/license/MaaAssistantArknights/MaaAssistantArknights">
    <img alt="commit" src="https://img.shields.io/github/commit-activity/m/MaaAssistantArknights/MaaAssistantArknights?color=%23ff69b4">
</div>
<div>
    <img alt="stars" src="https://img.shields.io/github/stars/MaaAssistantArknights/MaaAssistantArknights?style=social">
    <img alt="GitHub all releases" src="https://img.shields.io/github/downloads/MaaAssistantArknights/MaaAssistantArknights/total?style=social">
</div>
<br>

<!-- markdownlint-restore -->

[简体中文](https://maa.plus/docs/zh-cn/) | [繁體中文](https://maa.plus/docs/zh-tw/) | [English](https://maa.plus/docs/en-us/) | [日本語](https://maa.plus/docs/ja-jp/) | [한국어](https://maa.plus/docs/ko-kr/)

MAA 的意思是 MAA Assistant Arknights

一款明日方舟游戏小助手

基于图像识别技术，一键完成全部日常任务！

绝赞更新中  ✿✿ヽ(°▽°)ノ✿

</div>

## 下载与安装

请阅读[文档](https://maa.plus/docs/zh-cn/manual/newbie.html)后前往 [官网](https://maa.plus) 或 [Releases](https://github.com/MaaAssistantArknights/MaaAssistantArknights/releases) 下载，并参考[新手上路](https://maa.plus/docs/zh-cn/manual/newbie.html)进行安装。

## 亮点功能

- 刷理智，掉落识别及上传 [企鹅物流](https://penguin-stats.cn/)，[一图流](https://ark.yituliu.cn/)
- 智能基建换班，自动计算干员效率，单设施内最优解；同时也支持 [自定义排班](https://maa.plus/docs/zh-cn/protocol/base-scheduling-schema.html)
- 自动公招，可选使用加急许可，一次全部刷完！公招数据自动上传 [企鹅物流](https://penguin-stats.cn/result/stage/recruit/recruit)，[一图流](https://ark.yituliu.cn/survey/maarecruitdata)
- 支持手动识别公招界面，方便对高星公招做出选择 ~~（你的这个高姿回费出的是推王呢还是推王呢）~~
- 支持识别干员列表，统计已有和未有干员及潜能，并在公招识别显示
- 支持识别养成材料，并导出至 [企鹅物流刷图规划](https://penguin-stats.cn/planner)、[明日方舟工具箱](https://arkntools.app/#/material)、[ARK-NIGHTS 干员培养表](https://ark-nights.com/settings)
- 访问好友、收取信用及购物、领取日常奖励等，一键全日常自动长草
- 肉鸽全自动刷源石锭和等级，自动烧水和凹直升，智能识别干员及练度
- 选择作业 JSON 文件，自动抄作业， [视频演示](https://www.bilibili.com/video/BV1H841177Fk/)
- 支持 C, Python, Java, Rust, Golang, Java HTTP, Rust HTTP 等多种接口，方便集成调用，自定义你的 MAA！

<!-- markdownlint-disable -->

<details><summary>话不多说，看图！</summary>

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="./docs/.vuepress/public/images/zh-cn/readme/1-dark.png">
  <img alt="zh1" src="./docs/.vuepress/public/images/zh-cn/readme/1-light.png">
</picture>
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="./docs/.vuepress/public/images/zh-cn/readme/2-dark.png">
  <img alt="zh2" src="./docs/.vuepress/public/images/zh-cn/readme/2-light.png">
</picture>
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="./docs/.vuepress/public/images/zh-cn/readme/3-dark.png">
  <img alt="zh3" src="./docs/.vuepress/public/images/zh-cn/readme/3-light.png">
</picture>
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="./docs/.vuepress/public/images/zh-cn/readme/4-dark.png">
  <img alt="zh4" src="./docs/.vuepress/public/images/zh-cn/readme/4-light.png">
</picture>

</details>

<!-- markdownlint-restore -->

## 使用说明

### 功能介绍

请参阅 [用户手册](https://maa.plus/docs/zh-cn/manual/)。

### 外服支持

目前国际服（美服）、日服、韩服、繁中服的绝大部分功能均已支持。但由于外服用户较少及项目人手不足，很多功能并没有进行全面的测试，所以请自行体验。  
若您遇到了 Bug，或对某个功能有强需求，欢迎在 [Issues](https://github.com/MaaAssistantArknights/MaaAssistantArknights/issues) 和 [讨论区](https://github.com/MaaAssistantArknights/MaaAssistantArknights/discussions) 催更；或加入我们一起建设 MAA！请参阅 [外服适配教程](https://maa.plus/docs/zh-cn/develop/overseas-client-adaptation.html)

### CLI 支持

MAA 支持命令行界面（CLI）操作，支持 Linux，macOS 和 Windows，可用于自动化脚本或在无图形界面的服务器上使用。请参阅 [CLI 使用指南](https://maa.plus/docs/zh-cn/manual/cli/)

## 加入我们

### 主要关联项目

**目前项目组非常缺前端大佬，若您有相关经验，欢迎加入我们！**

- 全新框架：[MaaFramework](https://github.com/MaaXYZ/MaaFramework)
- [作业站](https://prts.plus) 前端：[maa-copilot-frontend](https://github.com/MaaAssistantArknights/maa-copilot-frontend)
- [作业站](https://prts.plus) 后端：[MaaBackendCenter](https://github.com/MaaAssistantArknights/MaaBackendCenter)
- [官网](https://maa.plus)：[前端](website)
- 深度学习：[MaaAI](https://github.com/MaaAssistantArknights/MaaAI)

### 多语言 (i18n)

MAA 支持多国语言，并使用 Weblate 进行本地化管理。如果您通晓多门语言，欢迎前往 [MAA Weblate](https://weblate.maa-org.net) 帮助我们进行翻译。

MAA 以中文（简体）为第一语言，翻译词条均以中文（简体）为准。

[![Weblate](https://weblate.maa-org.net/widget/maa/wpf-gui/multi-auto.svg)](https://weblate.maa-org.net/engage/maa/)

### 参与开发

#### Windows

请参阅 [开始开发](https://maa.plus/docs/zh-cn/develop/development.html)。

#### Linux | macOS

请参阅 [Linux 编译教程](https://maa.plus/docs/zh-cn/develop/linux-tutorial.html)。

#### API

- [C 接口](include/AsstCaller.h)：[集成示例](src/Cpp/main.cpp)
- [Python 接口](src/Python/asst/asst.py)：[集成示例](src/Python/sample.py)
- [Golang 接口](src/Golang)：[集成示例](src/Golang/maa/maa.go)
- [Dart 接口](src/Dart)
- [Java 接口](src/Java/src/main/java/com/iguigui/maaj/easySample/MaaCore.java)：[集成示例](src/Java/src/main/java/com/iguigui/maaj/easySample/MaaJavaSample.java)
- [Java HTTP 接口](src/Java/Readme.md)
- [Rust 接口](src/Rust/src/maa_sys)：[HTTP 接口](src/Rust)
- [TypeScript 接口](https://github.com/MaaAssistantArknights/MaaX/tree/main/packages/main/coreLoader)
- [Woolang 接口](src/Woolang/maa.wo)：[集成示例](src/Woolang/demo.wo)
- [集成文档](https://maa.plus/docs/zh-cn/protocol/integration.html)
- [回调消息协议](https://maa.plus/docs/zh-cn/protocol/callback-schema.html)
- [任务流程协议](https://maa.plus/docs/zh-cn/protocol/task-schema.html)
- [自动抄作业协议](https://maa.plus/docs/zh-cn/protocol/copilot-schema.html)

#### 外服适配

请参阅 [外服适配教程](https://maa.plus/docs/zh-cn/develop/overseas-client-adaptation.html)，对于国服已支持的功能，绝大部分的外服适配工作仅需要截图 + 简单的 JSON 修改即可。

#### 想参与开发，但不太会用 GitHub?

[GitHub Pull Request 流程简述](https://maa.plus/docs/zh-cn/develop/development.html#github-pull-request-流程简述)

#### Issue bot

请参阅 [Issue Bot 使用方法](https://maa.plus/docs/zh-cn/develop/issue-bot-usage.html)

## 致谢

### 开源库

- 图像识别库：[opencv](https://github.com/opencv/opencv.git)
- ~~文字识别库：[chineseocr_lite](https://github.com/DayBreak-u/chineseocr_lite.git)~~
- 文字识别库：[PaddleOCR](https://github.com/PaddlePaddle/PaddleOCR)
- 深度学习部署库：[FastDeploy](https://github.com/PaddlePaddle/FastDeploy)
- 机器学习加速器：[onnxruntime](https://github.com/microsoft/onnxruntime)
- ~~关卡掉落识别：[企鹅物流识别](https://github.com/penguin-statistics/recognizer)~~
- 地图格子识别：[Arknights-Tile-Pos](https://github.com/yuanyan3060/Arknights-Tile-Pos)
- C++ JSON 库：[meojson](https://github.com/MistEO/meojson.git)
- C++ 运算符解析器：[calculator](https://github.com/kimwalisch/calculator)
- ~~C++ base64 编解码：[cpp-base64](https://github.com/ReneNyffenegger/cpp-base64)~~
- C++ 解压压缩库：[zlib](https://github.com/madler/zlib)
- C++ Gzip 封装：[gzip-hpp](https://github.com/mapbox/gzip-hpp)
- 安卓触控事件器：[Minitouch](https://github.com/DeviceFarmer/minitouch)
- 安卓触控事件器：[MaaTouch](https://github.com/MaaAssistantArknights/MaaTouch)
- WPF MVVM 框架：[Stylet](https://github.com/canton7/Stylet)
- WPF 控件库：[HandyControl](https://github.com/HandyOrg/HandyControl) -> [HandyControls](https://github.com/ghost1372/HandyControls)
- C# 日志：[Serilog](https://github.com/serilog/serilog)
- C# JSON 库：[Newtonsoft.Json](https://github.com/JamesNK/Newtonsoft.Json) & [System.Text.Json](https://github.com/dotnet/runtime)
- ~~下载器：[aria2](https://github.com/aria2/aria2)~~

### 数据源

- ~~公开招募数据：[明日方舟工具箱](https://www.bigfun.cn/tools/aktools/hr)~~
- ~~干员及基建数据：[PRTS Wiki](http://prts.wiki/)~~
- 关卡数据：[企鹅物流数据统计](https://penguin-stats.cn/)
- 游戏数据及资源：[明日方舟客户端素材](https://github.com/yuanyan3060/ArknightsGameResource)
- 游戏数据：[《明日方舟》Yostar游戏数据](https://github.com/Kengxxiao/ArknightsGameData_YoStar)

### 贡献/参与者

感谢所有参与到开发/测试中的朋友们，是大家的帮助让 MAA 越来越好！ (\*´▽｀)ノノ

[![Contributors](https://contributors-img.web.app/image?repo=MaaAssistantArknights/MaaAssistantArknights&max=105&columns=15)](https://github.com/MaaAssistantArknights/MaaAssistantArknights/graphs/contributors)

## 声明

- 本软件使用 [GNU Affero General Public License v3.0 only](https://spdx.org/licenses/AGPL-3.0-only.html) 开源。
- 本软件 logo 并非使用 AGPL 3.0 协议开源，[耗毛](https://weibo.com/u/3251357314)、vie 两位画师及软件全体开发者保留所有权利。不得以 AGPL 3.0 协议已授权为由在未经授权的情况下使用本软件 logo，不得在未经授权的情况下将本软件 logo 用于任何用途。
- 本软件开源、免费，仅供学习交流使用。若您遇到商家使用本软件进行代练并收费，可能由于设备或时间等原因，产生的任何问题及后果与本软件无关。

### DirectML 支持说明

本软件支持 GPU 加速功能，但 GPU 加速依赖于 Microsoft 提供的 DirectML（Microsoft.AI.DirectML）。为了方便用户，我们随安装包附带了一个未修改的 DirectML.dll 文件。

#### 关于 DirectML.dll

- 来源：Microsoft 官方
- 许可证：请参考 Microsoft 的 DirectML 使用条款
  [DirectML 官方文档](https://learn.microsoft.com/en-us/windows/ai/directml/)

DirectML.dll 是 Microsoft 提供的独立组件，不属于本软件的开源部分，也不受 AGPL-3.0 的约束。

如果您不需要 GPU 支持，可以安全地删除该 DLL 文件，软件核心功能仍然可以正常运行。

## 广告

用户交流 QQ 群：[MAA 使用 & 粥游交流 QQ 群](https://ota.maa.plus/MaaAssistantArknights/api/qqgroup/index.html)  
Discord 服务器: [邀请链接](https://discord.gg/23DfZ9uA4V)  
用户交流 TG 群：[Telegram 群](https://t.me/+Mgc2Zngr-hs3ZjU1)  
自动战斗 JSON 作业分享：[prts.plus](https://prts.plus)  
Bilibili 直播间：[MrEO 直播间](https://live.bilibili.com/2808861) 直播敲代码 & [MAA-Official 直播间](https://live.bilibili.com/27548877) 游戏/杂谈  

技术群（舟无关、禁水）：[内卷地狱！(QQ 群)](https://jq.qq.com/?_wv=1027&k=ypbzXcA2)  
开发者群：[QQ 群](https://jq.qq.com/?_wv=1027&k=JM9oCk3C)  

如果觉得软件对你有帮助，帮忙点个 Star 吧！~（网页最上方右上角的小星星），这就是对我们最大的支持了！
