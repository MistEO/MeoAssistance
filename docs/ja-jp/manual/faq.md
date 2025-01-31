---
order: 4
icon: ph:question-fill
---

# よくある質問

初めて MAA を使用する場合は、[新規ユーザーガイド](./newbie.md)をお読みください。

::: warning

MAAがアップデート後に動作しなくなった場合、またはエラーメッセージウィンドウを経由してここにたどり着いた場合、それは大体ランタイムが更新されていないことが原因です。  
最も多い問題はランタイム関連であり、それにもかかわらず多くの人がドキュメントを読まずに質問するため、トップメッセージをこれに変更しました。とてもイライラします。

MAAディレクトリ内の `DependencySetup_依赖库安装.bat` を実行するか、以下のコマンドを端末で実行するか、

```sh
winget install "Microsoft.VCRedist.2015+.x64" --override "/repair /passive /norestart" --force --uninstall-previous --accept-package-agreements && winget install "Microsoft.DotNet.DesktopRuntime.8" --override "/repair /passive /norestart" --force --uninstall-previous --accept-package-agreements
```

以下の<u>**2つ**</u>のランタイムライブラリを手動でダウンロードしてインストールして問題を解決してください。

- [Visual C++ 再頒布可能パッケージ](https://aka.ms/vs/17/release/vc_redist.x64.exe)
- [.NET デスクトップランタイム 8](https://aka.ms/dotnet/8.0/windowsdesktop-runtime-win-x64.exe)

:::

## ソフトウェアが動作しない/クラッシュする/エラーが出る

### ダウンロード/インストールの問題

- 完全な MAA ソフトウェアパッケージの命名形式は "MAA-`バージョン`-`プラットフォーム`-`アーキテクチャ`.zip" です。他のものは単独で使用できない「コンポーネント」ですので、よくお読みください。
  ほとんどの場合、x64 アーキテクチャの MAA を使用する必要があります。つまり、`MAA-*-win-x64.zip` をダウンロードする必要があり、`MAA-*-win-arm64.zip` ではありません。
- 自動更新後に機能が欠落している、または使用できない場合は、更新プロセス中に問題が発生した可能性があります。完全なインストールパッケージを再ダウンロードして解凍してください。解凍後、古い `MAA` フォルダから `config` フォルダを新しく解凍した `MAA` フォルダに直接ドラッグしてください。

### ランタイムの問題

ウェブページの右下にある上向きの ↑ 矢印を見つけてクリックしてください。

### システムの問題

- MAA は 32 ビットのオペレーティングシステムをサポートしておらず、Windows 7 / 8 / 8.1 をサポートしていません。
- 上記のランタイムのインストールには、すべてコンポーネントストアサービス（CBS、TrustedInstaller/TiWorker、WinSxS）が必要です。
  コンポーネントストアサービスが破損している場合、正常にインストールできません。

システムの再インストール以外の修復提案を提供することはできませんので、縮小項目および縮小リスクが明記されていない「縮小版」システムや、古いバージョンのシステムを使用しないでください。

#### Windows N/KN

Windows N/KN（ヨーロッパ/韓国）の場合、[メディア機能パック](https://support.microsoft.com/ja-jp/topic/c1c6fffa-d052-8338-7a79-a4bb980a700a)をインストールする必要があります。

#### Windows 7 関連

.NET 8 は Windows 7 / 8 / 8.1 システムをサポートしていないため<sup>[ソース](https://github.com/dotnet/core/issues/7556)</sup>、MAA も同様にサポートしていません。最後に利用可能な .NET 8 バージョンは [`v5.4.0-beta.1.d035.gd2e5001e7`](https://github.com/MaaAssistantArknights/MaaRelease/releases/tag/v5.4.0-beta.1.d035.gd2e5001e7) です。最後に利用可能な .NET 4.8 バージョンは [`v4.28.8`](https://github.com/MaaAssistantArknights/MaaAssistantArknights/releases/tag/v4.28.8) です。自己コンパイルの実現可能性はまだ確認されていません。

Windows 7 の場合、上記の 2 つのランタイムライブラリをインストールする前に、以下のパッチがインストールされているかどうかを確認する必要があります：

1. [Windows 7 Service Pack 1](https://support.microsoft.com/ja-jp/windows/windows-7-service-pack-1-sp1-%E3%82%92%E3%82%A4%E3%83%B3%E3%82%B9%E3%83%88%E3%83%BC%E3%83%AB%E3%81%99%E3%82%8B-b3da2c0f-cdb6-0572-8596-bab972897f61)
2. SHA-2 コード署名：
    - KB4474419：[ダウンロードリンク1](https://catalog.s.download.windowsupdate.com/c/msdownload/update/software/secu/2019/09/windows6.1-kb4474419-v3-x64_b5614c6cea5cb4e198717789633dca16308ef79c.msu)、[ダウンロードリンク2](http://download.windowsupdate.com/c/msdownload/update/software/secu/2019/09/windows6.1-kb4474419-v3-x64_b5614c6cea5cb4e198717789633dca16308ef79c.msu)
    - KB4490628：[ダウンロードリンク1](https://catalog.s.download.windowsupdate.com/c/msdownload/update/software/secu/2019/03/windows6.1-kb4490628-x64_d3de52d6987f7c8bdc2c015dca69eac96047c76e.msu)、[ダウンロードリンク2](http://download.windowsupdate.com/c/msdownload/update/software/secu/2019/03/windows6.1-kb4490628-x64_d3de52d6987f7c8bdc2c015dca69eac96047c76e.msu)
3. Platform Update for Windows 7（DXGI 1.2、Direct3D 11.1，KB2670838）：[ダウンロードリンク1](https://catalog.s.download.windowsupdate.com/msdownload/update/software/ftpk/2013/02/windows6.1-kb2670838-x64_9f667ff60e80b64cbed2774681302baeaf0fc6a6.msu)、[ダウンロードリンク2](http://download.windowsupdate.com/msdownload/update/software/ftpk/2013/02/windows6.1-kb2670838-x64_9f667ff60e80b64cbed2774681302baeaf0fc6a6.msu)

##### .NET 8 アプリケーションが Windows 7 で異常に動作する場合の緩和策 [#8238](https://github.com/MaaAssistantArknights/MaaAssistantArknights/issues/8238)

Windows 7 で .NET 8 アプリケーションを実行すると、メモリ使用量が異常になる問題があります。以下の緩和策を参照してください。Windows 8/8.1 はテストされていません。同じ問題がある場合は、ドキュメントを更新するために Issue を提出してください。

1. `コンピュータ` を開き、空白部分を右クリックしてプロパティをクリックし、左側の `システムの詳細設定` をクリックし、`環境変数` をクリックします。
2. 新しいシステム変数を作成し、変数名を `DOTNET_EnableWriteXorExecute`、変数値を `0` に設定します。
3. コンピュータを再起動します。

#### 公式統合パック（確信）

::: info 注意
この操作はディスク容量を10GBほど消費しますので、他の可能性を確認した上で使用してください。
:::

- [Microsoft C++ Builder](https://visualstudio.microsoft.com/ja/visual-cpp-build-tools/) をインストールし、完全な開発環境を構成します（.NET と C++ の開発環境のみをインストールする必要がある）。

### 可能性3：システムコンポーネントの問題

- 記のランタイム・ライブラリのインストールはすべて、コンポーネントストレージサービス（CBS、TrustedInstaller/TiWorker、WinSxS）に依存しています。 コンポーネントストレージサービスが破損している場合、インストールは正しく動作しません。

- システムの再インストール以外の修正はお勧めできませんので、いわゆる「軽量/ライト」システムの使用は避けてください。

## 接続エラー

::: tip
[エミュレータのサポート](./device/) を参照して、エミュレータがサポートされていることを確認してください。
:::

### adbと接続アドレスが正しいことを確認する

- MAA `設定` - `接続設定` - `adbパス` が自動的に入力されていることを確認してください。入力されている場合は、この手順を無視してください。

::: details 未記入の場合は

- エミュレータのインストールパスを見つける。Windowsはエミュレータの実行中にタスクマネージャーでプロセスを右クリックし、 `ファイルの場所を開く` をクリックします。
- トップまたはボトムのディレクトリに高確率で `adb.exe` が存在する可能性があります（必ずしもこの名前で呼ばれているとは限りません。`nox_adb.exe`、`HD-adb.exe`、`adb_server.exe` などと呼ばれる場合があります。とにかく名前に `adb` が含まれるexeです） 。それを選択してください。
:::

- 接続アドレスが正しく入力されていることを確認してください。使用しているエミュレータのadbアドレスをインターネットで検索できます。通常は `127.0.0.1:5555` のような形式です（LDPlayerを除く）。

#### 一般的なAndroidエミュレータのadbポート

関連資料およびリファレンス adbポート：

- [Bluestacks 5](https://support.bluestacks.com/hc/zh-tw/articles/360061342631-%E5%A6%82%E4%BD%95%E5%B0%87%E6%82%A8%E7%9A%84%E6%87%89%E7%94%A8%E5%BE%9EBlueStacks-4%E8%BD%89%E7%A7%BB%E5%88%B0BlueStacks-5#%E2%80%9C2%E2%80%9D) `5555`
- [MuMu Pro](https://mumu.163.com/mac/function/20240126/40028_1134600.html) `16384`
- [MuMu 12](https://mumu.163.com/help/20230214/35047_1073151.html) `16384`
- [Nemu](https://bbs.xyaz.cn/forum.php?mod=viewthread&tid=365537) `21503`
- [NOX](https://support.yeshen.com/zh-CN/qt/ml) `62001`

他のエミュレーターは[@赵青青のブログ](https://www.cnblogs.com/zhaoqingqing/p/15238464.html)を参照してください

### 既存のadbプロセスを閉じる

- MAAを閉じた後、`タスクマネージャー`-`詳細`で`adb`という名前のプロセス（通常、上記で入力した`adb`ファイルと同じ名前）を探し、もしあれば終了して接続を再試行してください。

### スマホアシスタントをオフにする

Android Phone Assistant は adb を使用してスマホと通信しますが、この種のソフトウェアは MAA の動作を妨げる可能性がありますので、スマホアシスタントと既存の adb プロセスを閉じて、再試行してください。

### ゲームアクセラレータを回避する

ゲームアクセラレータなどのソフトウェアを使用と使用の停止している場合は、MAAやADBやエミュレータを終了し、PCを再起動してから再試行してください。

### コンピューターを再起動して試す

再起動すると、問題の97%が解決します（確信

### エミュレーターを変更する

[エミュレータのサポート](./device/) を参照してください。

## 接続に成功した後、固まり、全く操作できなくなる

一部のエミュレータの `adb` のバージョンが古すぎて、 `Minitouch` に対応していない場合があります。

管理者権限でMAAを開き、`設定` - `接続設定` - `ADB強制置き換え`をクリックしてください。（エミュレータを終了し、MAAを再起動してから行うことをお勧めします、そうしないと、交換が失敗する可能性があります。）

エミュレータを更新すると、ADBのバージョンがリセットされます。アップデート後に問題が再発する場合は、再度ファイルを置き換えてみてください。

それでもうまくいかない場合は、`接続設定` - `タッチモード` の `Minitouch` を `MaaTouch` に切り替えて再度使用してください。 `Adb Input` は非常に動作が遅いため最終手段としてご使用ください。

## Blue Stackエミュレータが起動するたびにポート番号が異なる（Hyper-V）

MAAを起動し `設定` - `接続設定` で `接続構成` に `BlueStacks` を選んで、 `接続自動認識` と `毎回再検出する` にチェックを入れます。或いは `ウェイクアップ` に設定する。

通常、これにより接続が設定されます。 接続できない場合は、複数のエミュレータコアをインストールしているか、機能に問題がある可能性がありますので、以下の手順に従って追加設定を行ってください。

### `Bluestacks.Config.Keyword` を指定する

::: info 注意
マルチコアを有効にしている場合、または複数のエミュレータコアをインストールしている場合は、使用するエミュレータ番号を指定するための追加設定が必要です。
:::

`gui.json` に `Bluestacks.Config.Keyword`フィールドを探し、`"bst.instance.emulator_number.status.adb_port"`というコンテンツを追加します。シミュレーター番号は、シミュレーター パスの `BlueStacks_nxt\Engine` で確認できます。

::: details 示例
Nougat64コア：

```json
"Bluestacks.Config.Keyword":"bst.instance.Nougat64.status.adb_port",
```

Pie64_2コア：（コア名の後の数字は、これがマルチコアであることを意味する）

```json
"Bluestacks.Config.Keyword": "bst.instance.Pie64_2.status.adb_port",
```

:::

### `Bluestacks.Config.Path` を指定する

::: info 注意
MAA はレジストリから `bluestacks.conf` の保存場所を読み取ろうとするようになり、機能が失敗したりエラーが発生したりした場合は、構成ファイルのパスを手動で指定する必要があります
:::

1. Bluestacksエミュレータのデータディレクトリにある`bluestacks.conf`ファイルを探します

    - グローバル版のデフォルトパス `C:\ProgramData\BlueStacks_nxt\bluestacks.conf`
    - 中国版のデフォルトパス `C:\ProgramData\BlueStacks_nxt_cn\bluestacks.conf`

2. 初めて MAA を使用する場合は、一度 MAA を起動してください。MAAの `config` ディレクトリに `gui.json` が生成されます。

3. **最初に** MAAを **終了してから** `gui.json` を開きます、 `Configurations` の下の現在の構成名フィールドを見つけます（設定 - 構成を切り替える で表示でき、デフォルトは `Default` です）、その中のフィールド `Bluestacks.Config.Path` を検索しますし、 `bluestacks.conf` へのフルパスを記入します。（注意として `\\`をエスケープに使用しています）

::: details 示例
`C:\ProgramData\BlueStacks_nxt\bluestacks.conf` を記入例として使用します

```json
{
  "Configurations": {
    "Default": {
      "Bluestacks.Config.Path":"C:\\ProgramData\\BlueStacks_nxt\\bluestacks.conf",
      // 残りの設定フィールドを手動で入力して変更しないでください
    }
  }
}
```

:::

## 接続に成功し、数回クリックすることができ、その後スタックまたはタスクエラーになる

- `UI調整`を使用している場合は、0に調整してください。
- CN以外のクライアントをお使いの場合は、`設定` - `対象クライアント` - `クライアントバージョン` の選択でご確認ください。また、非CNクライアントでは全ての機能がサポートされているわけではありませんので、各国語のドキュメントを参照してください。
- 自動ローグを実行している場合、[詳細説明](./introduction/integrated-strategy.md)を参照し、`スタート` - `自動ローグ` - `主題`で正しいテーマを選択してください。
- `Adb Input` のタッチモードで動作が遅いのは正常です。オートバトルなどが必要な場合は他のモードに切り替えてみてください。

### スクリーンショットに時間がかかる/長すぎるというメッセージが表示される

- MAA は現在、「RawByNc」、「RawWithGzip」、「Encode」の 3 つのスクリーンショット方法をサポートしています。 タスク実行の平均スクリーンショット時間が >400 / >800 の場合、プロンプト メッセージが出力されます (単一タスクは 1 回のみ出力されます)。
- 「設定 - 接続設定」には、過去 30 枚のスクリーンショットにかかった最小/平均/最大時間が表示され、スクリーンショット 10 枚ごとに更新されます。
- 自動戦闘機能（自動肉バトなど）はスクリーンショットの撮影時間に大きく影響されます。
- この時間の消費はMAAとは関係ありませんが、コンピューターのパフォーマンス、現在の使用状況、またはエミュレーターに関連しています。バックグラウンドプロセスをクリーンアップしたり、エミュレーターを変更したり、コンピューターの構成をアップグレードすることができます。

## 管理者権限に関する問題

MAAはWindows UAC管理者権限なしで、すべての機能を実行できるはずです。現在、管理者権限に関係する機能は以下の通りです：

1. `接続の自動検出`：ターゲットエミュレーターが管理者権限で実行されている場合、管理者権限が必要です。
2. `終了後にエミュレーターを閉じる`：ターゲットエミュレーターが管理者権限で実行されている場合、管理者権限が必要です。
3. `起動時にMAAを自動的に開始`：管理者権限で起動する場合、起動時の自動起動設定はできません。
4. MAAが誤って管理者権限が必要なパスに解凍されている場合、例えば `C:\`、`C:\Program Files\` など。

UACを無効にしたシステムでは、「右クリックして管理者として実行を選択しなくても、管理者権限で起動される」という問題が報告されています。意図しない権限昇格を避けるために、UACを有効にすることをお勧めします。

## ダウンロード速度が遅い、ミラーサイトにアクセスできない

[MAA ダウンロードステーション](https://ota.maa.plus/MaaAssistantArknights/MaaRelease/releases/download/) を使ってダウンロードしてください。
我々の提供サーバーでは帯域幅が狭く、トラフィックも限られています。 githubや他のミラーサイトからダウンロードできる場合は、そちらを選択することをお勧めします。

## ダウンロードが中途半端になり "ログイン"/"認証 "のプロンプトが出る

- ファイルをダウンロードするには、ブラウザ/IDM/FDM やその他の通常のダウンローダーを使用してください。**Thunderを使うな！**
