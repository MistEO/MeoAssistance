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

MAAは、MAA Assistant Arknightsです。

アークナイツゲームアシスタント

画像認識技術に基づいて、ワンクリックですべてのデイリーリクエストを完了します！

絶賛開発中  ✿✿ヽ(°▽°)ノ✿

:::

## ダウンロードとインストール

[ドキュメント](./manual/newbie.md)を読んでから、[公式サイト](https://maa.plus)または[リリース](https://github.com/MaaAssistantArknights/MaaAssistantArknights/releases)にアクセスしてダウンロードしてください。[新規ユーザーガイド](./manual/newbie.md)を参考にインストールを行ってください。

## 機能一覧

- 自動作戦、ドロップ認識および [PenguinStats](https://penguin-stats.io/) と [Yituliu](https://ark.yituliu.cn/) へデータアップロード。
- 自動基地シフト、オペレーター効率計算、単一設備内に最適なソリューション；[カスタムシフト](./protocol/base-scheduling-schema.md)にも対応しています。
- 自動公開求人、緊急招集票を使う、使い切るのもサポート。[PenguinStats](https://penguin-stats.io/result/stage/recruit/recruit)と[Yituliu](https://ark.yituliu.cn/survey/maarecruitdata)へ公開求人データのアップロード。
- 高スターの公開求人を選択するのに便利な公開求人のパネルの手動認識をサポートします。~~（この上級エリートとCost回復はシージかシージか）~~
- 所持オペレーターを認識し、既存および未所有のオペレーターの記録サポート、公開求人を手動で設定するためのヒントを提供することもできます。（日本サーバーではオペレーターの名前部分のフォントサイズが異なるため認識にややブレがあります）
- 倉庫のアイテム認識機能 [Arkplanner](https://penguin-stats.io/planner)/[ARK-NIGHTS.com](https://ark-nights.com/settings)と[アークナイツ ツールボックス](https://arkntools.app/#/material)へ出力可能！使用方法はツール内文章を参照してください。
- 戦友訪問、FP収集、買い物、デイリーリワード収集、ワンクリックして全自動操作！
- 統合戦略自動作戦、オペレーターとレベルの自動認識、源石錐とキャンドルの自動収集、電気ケトルを獲得！
- 作業JSONファイルをインポートし、自動操作も可能！ [ビデオデモ](https://www.bilibili.com/video/BV1H841177Fk/)（現在JP未対応/中文）
- C、Python、Java、Rust、Golang、Java HTTP、Rust HTTPなどの多種多様なインターフェースに対応、統合や呼び出しが簡単で、自分好みにMAAをカスタマイズできます!

UIを見れば使い方もすぐ分かる！  

```component Image2
{
  "imageList": [
    {
      "light": "images/ja-jp/readme/1-light.png",
      "dark": "images/ja-jp/readme/1-light.png"
    },
    {
      "light": "images/ja-jp/readme/2-light.png",
      "dark": "images/ja-jp/readme/2-light.png"
    }
  ]
}
```

## マニュアル

### 機能紹介

[ユーザーマニュアル](./manual/)をご参照ください。

### サーバーに応じてサポートされる機能

現在、国際クライアント（英語クライアント）、日本語クライアント、韓国語クライアント、繁体字中国語クライアントのほとんどの機能がサポートされています。 ただし、海外ユーザーの少なさとプロジェクト要員不足により、十分に検証できていない機能も多いので、ぜひ体験してみてください。  
バグに遭遇した場合、または特定の機能に対する強い要望がある場合は、[Issues](https://github.com/MaaAssistantArknights/MaaAssistantArknights/issues) and [Discussions](https://github.com/MaaAssistantArknights/MaaAssistantArknights/discussions) で質問してください。 MAA の構築にご参加ください。[海外クライアント対応](#グローバル版を含む海外クライアントの対応について) を参照してください

### CLI支持

MAA はコマンドラインインタフェース（CLI）操作をサポートし、LinuxとmacOSをサポートし、自動スクリプトやグラフィックインタフェースのないサーバで使用することができる。[CLI使用ガイド](./manual/cli)を参照してください。

## 一緒に参加しよう

### 主な関連プロジェクト

**現在、プロジェクトチームにはフロントエンドの専門家が非常に不足しています。経験があれば、私たちに参加してください！**

- 新しいFramework: [MaaFramework](https://github.com/MaaAssistantArknights/MaaFramework)
- [作業シェアサイト](https://prts.plus)：[フロントエンド](https://github.com/MaaAssistantArknights/maa-copilot-frontend)
- バックエンド：[MaaBackendCenter](https://github.com/MaaAssistantArknights/MaaBackendCenter)
- [公式ウェブサイト](https://www.maa.plus): [フロントエンド](https://github.com/MaaAssistantArknights/MaaAssistantArknights/tree/dev/website)
- Deep Learning: [MaaAI](https://github.com/MaaAssistantArknights/MaaAI)

### 多言語 (i18n)

MAA は多言語をサポートし、Weblateを使用してローカライズ管理を行います。複数の言語に精通している場合は、[MAA Weblate](https://weblate.maa-org.net)で翻訳のお手伝いをしてください。

MAA は中国語（簡体字）を第一言語とし、翻訳見出しはすべて中国語（簡体字）を基準としています。

[![Weblate](https://weblate.maa-org.net/widget/maa/wpf-gui/multi-auto.svg)](https://weblate.maa-org.net/engage/maa/)

### Windows

1. ビルド済みのサードパーティ ライブラリをダウンロードします。

      ```cmd
      python maadeps-download.py
      ```

2. Visual Studio 2022 で `MAA.sln` を開き、`MaaWpfGui` を右クリックして、スタートアップ プロジェクトとして設定します。
3. VS 上記の設定で 'RelWithDebInfo' 'x64' を選択します （Release パッケージまたは ARM プラットフォームをコンパイルしている場合は、この手順を無視してください）
4. `MaaWpfGui` を右クリックし、[プロパティ] - [デバッグ] - [ローカル デバッグを有効にする] を選択します (これにより、C++ コアにブレークポイントを掛けることができます)。
5. (オプション) PR を送信する場合は、[clang-formatを有効にする](./develop/development.md#visual-studioでclang-formatを有効にする)を有効にすることをお勧めします。

### Linux | macOS

[Linuxチュートリアル](./develop/linux-tutorial.md)を参照してください。

### API

- [Cインターフェース](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/master/include/AsstCaller.h)：[統合例](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/master/src/Cpp/main.cpp)
- [Pythonインターフェース](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/master/src/Python/asst/asst.py)：[統合例](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/master/src/Python/sample.py)
- [Golangインターフェース](https://github.com/MaaAssistantArknights/MaaAssistantArknights/tree/master/src/Golang/)：[統合例](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/master/src/Golang/maa/maa.go)
- [Dartインターフェース](https://github.com/MaaAssistantArknights/MaaAssistantArknights/tree/master/src/Dart)
- [Javaインターフェース](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/master/src/Java/src/main/java/com/iguigui/maaj/easySample/MaaCore.java)：[統合例](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/master/src/Java/src/main/java/com/iguigui/maaj/easySample/MaaJavaSample.java)
- [Java HTTPインターフェース](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/master/src/Java/Readme.md)
- [Rustインターフェース](https://github.com/MaaAssistantArknights/MaaAssistantArknights/tree/master/src/Rust/src/maa_sys)：[HTTPインターフェース](https://github.com/MaaAssistantArknights/MaaAssistantArknights/tree/master/src/Rust)
- [TypeScriptインターフェース](https://github.com/MaaAssistantArknights/MaaAsstElectronUI/tree/main/packages/main/coreLoader)
- [Woolangインターフェース](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/master/src/Woolang/maa.wo)：[統合例](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/master/src/Woolang/demo.wo)
- [統合ドキュメント](./protocol/integration.md)
- [コールバックAPI](./protocol/callback-schema.md)
- [タスクAPI](./protocol/task-schema.md)
- [自動戦闘API](./protocol/copilot-schema.md)

### グローバル版を含む海外クライアントの対応について

[海外版クライアントの対応について](./develop/overseas-client-adaptation.md)をご覧ください。大陸版で既にサポートされている機能を他地域クライアントへ移植するための必要作業の大半は、簡単なJSONの修正と（作業に必要な）スクリーンショットの提出で済みます。。

### 開発に参加したいがGitHubの使い方がよくわかりません

[Githubプルリクエストのプロセス](./develop/development.md#introduction-to-github-pull-request-flow)

### Issue bot

詳細については[Issue bot 使用方法](./develop/issue-bot-usage.md)を参照してください。

## 謝辞

### オープンソースライブラリ

- 画像認識ライブラリ：[opencv](https://github.com/opencv/opencv.git)
- ~~テキスト認識ライブラリ：[chineseocr_lite](https://github.com/DayBreak-u/chineseocr_lite.git)~~
- テキスト認識ライブラリ：[PaddleOCR](https://github.com/PaddlePaddle/PaddleOCR)
- ML Deployment: [FastDeploy](https://github.com/PaddlePaddle/FastDeploy)
- ML accelerator: [onnxruntime](https://github.com/microsoft/onnxruntime)
- ~~ステージドロップ認識：[PenguinStats認識](https://github.com/penguin-statistics/recognizer)~~
- マップタイル認識：[Arknights-Tile-Pos](https://github.com/yuanyan3060/Arknights-Tile-Pos)
- C++ JSONライブラリ：[meojson](https://github.com/MistEO/meojson.git)
- C++ オペレーターパーサー：[calculator](https://github.com/kimwalisch/calculator)
- ~~C++ base64エンコードとデコード：[cpp-base64](https://github.com/ReneNyffenegger/cpp-base64)~~
- C++ 圧縮・解凍ライブラリ：[zlib](https://github.com/madler/zlib)
- C++ Gzipカプセル化ライブラリ：[gzip-hpp](https://github.com/mapbox/gzip-hpp)
- Android タッチ イベント: [Minitouch](https://github.com/DeviceFarmer/minitouch)
- Android タッチ イベント: [MaaTouch](https://github.com/MaaAssistantArknights/MaaTouch)
- WPF MVVWフレームワーク：[Stylet](https://github.com/canton7/Stylet)
- WPFコントロールライブラリ：[HandyControl](https://github.com/HandyOrg/HandyControl) -> [HandyControls](https://github.com/ghost1372/HandyControls)
- C# JSONライブラリ：[Newtonsoft.Json](https://github.com/JamesNK/Newtonsoft.Json) & [System.Text.Json](https://github.com/dotnet/runtime)
- ~~ダウンローダー：[aria2](https://github.com/aria2/aria2)~~

### データソース

- ~~公開求人データ：[アークナイツツール](https://www.bigfun.cn/tools/aktools/hr)~~
- ~~オペレーターおよび基地データ：[PRTSアークナイツ中国語WIKI](http://prts.wiki/)~~
- ステージデータ：[PenguinStatsデータ統計](https://penguin-stats.io/)
- ゲームのデータとリソース：[アークナイツのクライアント資料](https://github.com/yuanyan3060/ArknightsGameResource)
- ゲームデータ：[アークナイツYostarのゲームデータ](https://github.com/Kengxxiao/ArknightsGameData_YoStar)

### 貢献/協力者

MAAをより良くするために開発・テストに貢献してくれたすべての方々に感謝します！ (\*´▽｀)ノノ

[![Contributors](https://contributors-img.web.app/image?repo=MaaAssistantArknights/MaaAssistantArknights&max=105&columns=15)](https://github.com/MaaAssistantArknights/MaaAssistantArknights/graphs/contributors)

## 免責事項

- 本ソフトウェアは [GNU Affero General Public License v3.0 only](https://spdx.org/licenses/AGPL-3.0-only.html) オープンソースライセンスを使用しています。
- 本ソフトウェアのロゴはAGPL 3.0ライセンスを使用しており、オープンソースではなく、[耗毛](https://weibo.com/u/3251357314)とVie、2人のアーティストと開発者がすべての権利を留保しています。「AGPL 3.0ライセンスに基づいて許可されている」という理由で、許可がない場合に本ソフトウェアのロゴを無断で使用することは禁止しております。また、許可なく商業目的で本ソフトウェアのロゴを無断で使用することも禁止しております。
- 本ソフトウェアはオープンソースで無料であり、学習と研究のみの目的としています。販売者が本ソフトウェアの料金を請求する場合は、デバイスや時間の料金である可能性があります。発生した問題と結果は、本ソフトウェアとは一切関係ありません。

### DirectML サポート説明

本ソフトウェアは GPU 加速機能をサポートしていますが、GPU 加速は Microsoft 提供の DirectML（Microsoft.AI.DirectML）に依存しています。ユーザーの利便性を考慮し、未改変の DirectML.dll ファイルをインストールパッケージに同梱しています。

#### DirectML.dll について

- 出典：Microsoft 公式
- ライセンス：Microsoft の DirectML 利用規約を参照してください  
  [DirectML 公式ドキュメント](https://learn.microsoft.com/en-us/windows/ai/directml/)

DirectML.dll は Microsoft による独立したコンポーネントであり、本ソフトウェアのオープンソース部分には含まれません。また、AGPL-3.0 の制約も受けません。

GPU サポートが不要な場合、この DLL ファイルを安全に削除することができます。それでもソフトウェアの主要機能は正常に動作します。

## 広告

ユーザー研究グループQQグループ：[QQグループ](https://ota.maa.plus/MaaAssistantArknights/api/qqgroup/index.html)  
Discord サーバー: [Discord 招待リンク](https://discord.gg/23DfZ9uA4V)  
ユーザー研究グループTGグループ：[Telegram](https://t.me/+Mgc2Zngr-hs3ZjU1)  
自動作戦JSON作業シェア：[prts.plus](https://prts.plus)  
Bilibili ライブ配信: [MrEO ライブ配信](https://live.bilibili.com/2808861) コーディング & [MAA-Official ライブ配信](https://live.bilibili.com/27548877) ゲーム/雑談  

技術研究(アークナイツ無関係)：[インボリューション・ヘル！(QQグループ)](https://jq.qq.com/?_wv=1027&k=ypbzXcA2)  
開発者グループ：[QQグループ](https://jq.qq.com/?_wv=1027&k=JM9oCk3C)  

ソフトウェアが役立つと思うなら、Star（ページの右上隅にある星）をクリックしてください。私たちにとって最高のサポートです！
