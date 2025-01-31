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

MAA 的意思是 MAA Assistant Arknights

一款明日方舟遊戲小助手

基於圖像辨識技術，一鍵完成全部日常任務！

絕讚更新中  ✿✿ヽ(°▽°)ノ✿

:::

## 下載與安裝

請閱讀[文檔](./manual/newbie.md)後前往 [官網](https://maa.plus) 或 [Releases](https://github.com/MaaAssistantArknights/MaaAssistantArknights/releases) 下載，並參考[新手上路](./manual/newbie.md)進行安裝。

## 亮點功能

- 刷理智，掉落辨識及上傳 [企鵝物流數據統計](https://penguin-stats.io/)，[一圖流](https://ark.yituliu.cn/)；
- 智能基建換班，自動計算幹員效率，單設施內最優解；同時也支援 [自定義排班](./protocol/base-scheduling-schema.md)；
- 自動公招，可選使用加急許可，一次全部刷完！公招數據上傳 [企鵝物流數據統計](https://penguin-stats.io/result/stage/recruit/recruit) ， [一圖流](https://ark.yituliu.cn/survey/maarecruitdata) ；
- 支援手動辨識公招介面，方便對高星公招做出選擇 ~~（你的這個高資回費出的是推王呢還是推王呢）~~
- 支援辨識幹員列表，統計已有和未有幹員（還能為手動辨識公招介面提供已有潛能數據提示）；
- 支援辨識養成材料，並可匯出至 [企鵝物流刷圖規劃器](https://penguin-stats.io/planner) 和 [明日方舟工具箱](https://arkntools.app/#/material) 計算缺少的養成材料；
- 訪問好友、收取信用及購物、領取日常獎勵等，一鍵全日常自動長草；
- 肉鴿全自動刷源石錠和蠟燭，自動辨識幹員及練度；
- 選擇作業 JSON 文件，自動抄作業， [影片範例](https://www.bilibili.com/video/BV1H841177Fk/) ；
- 倉庫辨識並支援匯出至 [企鵝物流刷圖規劃器](https://penguin-stats.io/planner) ， [明日方舟工具箱](https://arkntools.app/#/material) ， [ARK-NIGHTS 幹員培養表](https://ark-nights.com/settings) ；
- 支援 C, Python, Java, Rust, Golang, Java HTTP, Rust HTTP 等多種接口，方便集成調用，自定義你的 MAA！

話不多說，看圖！

```component Image4
{
  "imageList": [
    {
      "light": "images/zh-cn/readme/1-light.png",
      "dark": "images/zh-cn/readme/1-dark.png"
    },
    {
      "light": "images/zh-cn/readme/2-light.png",
      "dark": "images/zh-cn/readme/2-dark.png"
    },
    {
      "light": "images/zh-cn/readme/3-light.png",
      "dark": "images/zh-cn/readme/3-dark.png"
    },
    {
      "light": "images/zh-cn/readme/4-light.png",
      "dark": "images/zh-cn/readme/4-dark.png"
    }
  ]
}
```

## 使用說明

### 功能介紹

請參閱 [用戶手冊](./manual/)。

### 外服支援

目前國際服（美服）、日服、韓服、繁中服的絕大部分功能均已支援。但由於外服用戶較少及項目人手不足，很多功能並沒有進行全面的測試，所以請自行體驗。  
若您遇到了 Bug，或對某個功能有強需求，歡迎在 [Issues](https://github.com/MaaAssistantArknights/MaaAssistantArknights/issues) 和 [討論區](https://github.com/MaaAssistantArknights/MaaAssistantArknights/discussions) 催更；或加入我們一起建設 MAA！請參考 [外服適配教學](#外服適配)

## 主要關聯項目

**目前項目組非常缺前端大佬，若您有相關經驗，歡迎加入我們！**

- 全新框架：[MaaFramework](https://github.com/MaaAssistantArknights/MaaFramework)
- [作業站](https://prts.plus) 前端：[maa-copilot-frontend](https://github.com/MaaAssistantArknights/maa-copilot-frontend)
- [作業站](https://prts.plus) 後端：[MaaBackendCenter](https://github.com/MaaAssistantArknights/MaaBackendCenter)
- [官網](https://maa.plus) ： [前端](https://github.com/MaaAssistantArknights/MaaAssistantArknights/tree/dev/website)
- 深度學習：[MaaAI](https://github.com/MaaAssistantArknights/MaaAI)

### 多語言 (i18n)

MAA 支援多國語言，並使用 Weblate 進行在地化管理。如果您通曉多門語言，歡迎前往 [MAA Weblate](https://weblate.maa-org.net) 協助我們進行翻譯。

MAA 以中文（簡體）為第一語言，翻譯詞條皆以中文（簡體）為準。

[![Weblate](https://weblate.maa-org.net/widget/maa/wpf-gui/multi-auto.svg)](https://weblate.maa-org.net/engage/maa/)

## 致謝

### 開源庫

- 圖像辨識庫：[opencv](https://github.com/opencv/opencv.git)
- ~~文字辨識庫：[chineseocr_lite](https://github.com/DayBreak-u/chineseocr_lite.git)~~
- 文字辨識庫：[PaddleOCR](https://github.com/PaddlePaddle/PaddleOCR)
- 深度學習部署庫：[FastDeploy](https://github.com/PaddlePaddle/FastDeploy)
- 機器學習加速器：[onnxruntime](https://github.com/microsoft/onnxruntime)
- ~~關卡掉落辨識：[企鵝物流辨識](https://github.com/penguin-statistics/recognizer)~~
- 地圖格子辨識：[Arknights-Tile-Pos](https://github.com/yuanyan3060/Arknights-Tile-Pos)
- C++ JSON 庫：[meojson](https://github.com/MistEO/meojson.git)
- C++ 運算符解析器：[calculator](https://github.com/kimwalisch/calculator)
- ~~C++ base64 編解碼：[cpp-base64](https://github.com/ReneNyffenegger/cpp-base64)~~
- C++ 解壓壓縮庫：[zlib](https://github.com/madler/zlib)
- C++ Gzip 封裝：[gzip-hpp](https://github.com/mapbox/gzip-hpp)
- 安卓觸控事件器：[Minitouch](https://github.com/DeviceFarmer/minitouch)
- 安卓觸控事件器：[MaaTouch](https://github.com/MaaAssistantArknights/MaaTouch)
- WPF MVVM 框架：[Stylet](https://github.com/canton7/Stylet)
- WPF 控件庫：[HandyControl](https://github.com/HandyOrg/HandyControl)
- C# JSON 庫：[Newtonsoft.Json](https://github.com/JamesNK/Newtonsoft.Json)
- 下載器：[aria2](https://github.com/aria2/aria2)

### 數據源

- ~~公開招募數據：[明日方舟工具箱](https://www.bigfun.cn/tools/aktools/hr)~~
- ~~幹員及基建數據：[PRTS明日方舟中文WIKI](http://prts.wiki/)~~
- 關卡數據：[企鵝物流數據統計](https://penguin-stats.io/)
- 遊戲數據及資源：[明日方舟用戶端素材](https://github.com/yuanyan3060/ArknightsGameResource)
- 遊戲數據：[《明日方舟》Yostar遊戲數據](https://github.com/Kengxxiao/ArknightsGameData_YoStar)

### 貢獻 / 參與者

感謝所有參與到開發 / 測試中的朋友們，是大家的幫助讓 MAA 越來越好！ (\*´▽｀)ノノ

[![Contributors](https://contributors-img.web.app/image?repo=MaaAssistantArknights/MaaAssistantArknights&max=105&columns=15)](https://github.com/MaaAssistantArknights/MaaAssistantArknights/graphs/contributors)

## 開發相關

### Windows

1. 下載預構建的第三方庫

    ```cmd
    python maadeps-download.py
    ```

2. 使用 Visual Studio 2022 打開 `MAA.sln`，右鍵 `MaaWpfGui`，設為啟動項目
3. VS 上方配置選擇 `RelWithDebInfo`, `x64` （如果編譯 Release 包 或 ARM 平台，請忽略這步）
4. 右鍵 `MaaWpfGui` - 屬性 - 偵錯 - 啟用本地偵錯（這樣就能把斷點掛到 C++ Core 那邊了）
5. （可選）若準備提交 PR，建議啟用 [clang-format 支援](./develop/development.md#在-visual-studio-中啟用-clang-format)

### Linux | macOS

請參考 [Linux 編譯教學](./develop/linux-tutorial.md)

### API

- [C 接口](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/master/include/AsstCaller.h) ： [集成範例](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/master/src/Cpp/main.cpp)
- [Python 接口](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/master/src/Python/asst/asst.py) ： [集成範例](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/master/src/Python/sample.py)
- [Golang 接口](https://github.com/MaaAssistantArknights/MaaAssistantArknights/tree/master/src/Golang/) ： [集成範例](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/master/src/Golang/maa/maa.go)
- [Dart 接口](https://github.com/MaaAssistantArknights/MaaAssistantArknights/tree/master/src/Dart)
- [Java 接口](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/master/src/Java/src/main/java/com/iguigui/maaj/easySample/MaaCore.java) ： [集成範例](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/master/src/Java/src/main/java/com/iguigui/maaj/easySample/MaaJavaSample.java)
- [Java HTTP 接口](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/master/src/Java/Readme.md)
- [Rust 接口](https://github.com/MaaAssistantArknights/MaaAssistantArknights/tree/master/src/Rust/src/maa_sys) ： [HTTP 接口](https://github.com/MaaAssistantArknights/MaaAssistantArknights/tree/master/src/Rust)
- [TypeScript 接口](https://github.com/MaaAssistantArknights/MaaAsstElectronUI/tree/main/packages/main/coreLoader)
- [集成文件](./protocol/integration.md)
- [回呼訊息協議](./protocol/callback-schema.md)
- [任務流程協議](./protocol/task-schema.md)
- [自動抄作業協議](./protocol/copilot-schema.md)

### 外服適配

請參考 [外服適配教學](./develop/overseas-client-adaptation.md)，對於陸服已支援的功能，絕大部分的外服適配工作僅需要截圖 + 簡單的 JSON 修改即可。

### 想參與開發，但不太會用 Github?

[Github Pull Request 流程簡述](./develop/development.md#introduction-to-github-pull-request-flow)

### Issue bot

請參考 [Issue bot 使用方法](./develop/issue-bot-usage.md)

## 聲明

- 本軟體使用 [GNU Affero General Public License v3.0 only](https://spdx.org/licenses/AGPL-3.0-only.html) 開源。
- 本軟體 logo 並非使用 AGPL 3.0 協議開源，[耗毛](https://weibo.com/u/3251357314)、vie 兩位畫師及軟體全體開發者保留所有權利。不得以 AGPL 3.0 協議已授權為由在未經授權的情況下使用本軟體 logo，不得在未經授權的情況下將本軟體 logo 用於任何商業用途。
- 本軟體開源、免費，僅供學習交流使用。若您遇到商家使用本軟體進行代練並收費，可能是設備與時間等費用，產生的問題及後果與本軟體無關。

### DirectML 支援說明

本軟體支援 GPU 加速功能，但 GPU 加速依賴於 Microsoft 提供的 DirectML（Microsoft.AI.DirectML）。為了方便使用者，我們隨安裝包附帶了一個未修改的 DirectML.dll 文件。

#### 關於 DirectML.dll

- 來源：Microsoft 官方
- 許可證：請參考 Microsoft 的 DirectML 使用條款  
  [DirectML 官方文檔](https://learn.microsoft.com/en-us/windows/ai/directml/)

DirectML.dll 是 Microsoft 提供的獨立組件，不屬於本軟體的開源部分，也不受 AGPL-3.0 的約束。

如果您不需要 GPU 支援，可以安全地刪除該 DLL 文件，軟體核心功能仍然可以正常運行。

## 廣告

用戶交流 QQ 群：[MAA 使用 & 粥遊交流 QQ 群](https://ota.maa.plus/MaaAssistantArknights/api/qqgroup/index.html)  
Discord 伺服器：[Discord](https://discord.gg/23DfZ9uA4V)  
用戶交流 TG 群：[Telegram 群](https://t.me/+Mgc2Zngr-hs3ZjU1)  
自動戰鬥 JSON 作業分享：[prts.plus](https://prts.plus) 或 [抄作業.com](http://抄作業.com)  
Bilibili 直播間：[MrEO 直播間](https://live.bilibili.com/2808861) 直播敲代碼 & [MAA-Official 直播間](https://live.bilibili.com/27548877) 遊戲/雜談  

技術群（舟無關、禁水）：[內卷地獄！(QQ 群)](https://jq.qq.com/?_wv=1027&k=ypbzXcA2)  
開發者群：[QQ 群](https://jq.qq.com/?_wv=1027&k=JM9oCk3C)  

如果覺得軟體對你有幫助，幫忙點個 Star 吧！~（網頁最上方右上角的小星星），這就是對我們最大的支持了！
