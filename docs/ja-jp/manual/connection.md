---
order: 3
icon: mdi:plug
---

# 接続設定

## ADB パス

:::info 技術的詳細
自動検出ではエミュレータの ADB を使用しますが、自動検出に問題が発生することがあります。その場合、手動で設定する必要があります。
`ADB の強制置換`は、Google が提供する ADB をダウンロードして置換することで、一度設定すれば解決します。
:::

### エミュレータ提供の ADB を使用する

エミュレータのインストールパスに移動し、Windows ではタスクマネージャでプロセスを右クリックし、「ファイルの場所を開く」を選択します。

最上位またはサブディレクトリに、`adb` を含む exe ファイルがあるはずです。検索機能を使用して選択します。

:::details いくつかの例
`adb.exe` `HD-adb.exe` `adb_server.exe` `nox_adb.exe`
:::

### Google 提供の ADB を使用する

[ここをクリックしてダウンロード](https://dl.google.com/android/repository/platform-tools-latest-windows.zip)し、解凍して、その中から `adb.exe` を選択します。

おすすめは、直接 `MAA` フォルダに解凍することです。そうすると、ADB パスに `.\platform-tools\adb.exe` を入力することができます。`MAA` フォルダと一緒に移動することもできます。

## 接続アドレス

:::tip
ローカルで実行しているエミュレータの接続アドレスは `127.0.0.1:<ポート番号>` または `emulator-<四桁の数字>` です。
:::

### ポート番号の取得

#### エミュレータ関連ドキュメントおよび参考ポート

- [Bluestacks 5](https://support.bluestacks.com/hc/zh-tw/articles/360061342631-%E5%A6%82%E4%BD%95%E5%B0%87%E6%82%A8%E7%9A%84%E6%87%89%E7%94%A8%E5%BE%9EBlueStacks-4%E8%BD%89%E7%A7%BB%E5%88%B0BlueStacks-5#%E2%80%9C2%E2%80%9D) `5555`
- [MuMu Pro](https://mumu.163.com/mac/function/20240126/40028_1134600.html) `16384`
- [MuMu 12](https://mumu.163.com/help/20230214/35047_1073151.html) `16384`
- [逍遥](https://bbs.xyaz.cn/forum.php?mod=viewthread&tid=365537) `21503`
- [夜神](https://support.yeshen.com/zh-CN/qt/ml) `62001`

他のエミュレータについては[Zhaoqingqing's Blog](https://www.cnblogs.com/zhaoqingqing/p/15238464.html)を参照してください。

#### 多重起動について

- MuMu 12 の多重起動マネージャーでは、実行中のポートを確認できます。
- Bluestacks 5 の設定内で現在の多重ポートを確認できます。
- _追加予定_

::: details 代替案

- オプション 1 : ADB コマンドを使用してエミュレータのポートを確認します

  1. **単一**のエミュレータを起動し、他に Android デバイスがこのコンピュータに接続されていないことを確認します。
  2. ADB 実行可能ファイルが格納されているフォルダでターミナルを起動します。
  3. 次のコマンドを実行します。

  ```sh
  # Windows コマンド プロンプト
  adb devices
  # Windows PowerShell
  .\adb devices
  ```

  以下は、出力例です

  ```text
  List of devices attached
  127.0.0.1:<ポート番号>   device
  emulator-<四桁の数字>  device
  ```

  使用 `127.0.0.1:<ポート>` または `emulator-<四桁の数字>` を接続アドレスとして使用します。

- 方法2：すでに確立されたADB接続を検索する

  1. 方法1を実行します。
  2. `Windowsキー+S` を押して検索バーを開き、「リソースモニター」を入力して開きます。
  3. `ネットワーク` タブに切り替えて、モニターするポート名であるシミュレータープロセス名（例：`HD-Player.exe`）を検索します。
  4. シミュレータープロセスのすべてのリッスンポートを記録します。
  5. `TCP接続` タブの中で、`adb.exe` を探し、リモートポート列でシミュレーターリッスンポートに一致するポートを探します。

:::

### 自動的に複数のエミュレーターを起動する

複数のエミュレーターを同時に操作する場合、MAAフォルダを複数コピーし、**異なるMAA**、**同じadb.exe**、**異なる接続アドレス** を使用して接続します。
**[ブルースタックス国際版](./device/windows.md)を例に**、2つの多重起動エミュレーターの開始方法を紹介します。

#### エミュレーターexeにコマンドを添付して複数起動する

1. **単一**のエミュレーターを複数開始します。
2. タスクマネージャーを開き、該当するエミュレータープロセスを見つけます。詳細情報タブに移動し、列ヘッダーで右クリックし、「列の選択」をクリックし、「コマンドライン」を選択します。
3. 追加された「コマンドライン」列で、「...\Bluestacks_nxt\HD-Player.exe」の後にある内容を見つけます。
4. 見つけた `--instance Nougat32` のような内容を「起動設定」 - 「追加コマンド」に記入します。

::: tip
操作が終了したら、「ステップ2」で開いた「コマンドライン」列を再度非表示にすることをお勧めします。これにより、パフォーマンスの低下を防ぎます。
:::

::: details 例

```text
多開1:
エミュレーターパス: C:\Program Files\BlueStacks_nxt\HD-Player.exe
追加コマンド: --instance Nougat32 --cmd launchApp --package "com.hypergryph.arknights"
多開2:
エミュレーターパス: C:\Program Files\BlueStacks_nxt\HD-Player.exe
追加コマンド: --instance Nougat32_1 --cmd launchApp --package "com.hypergryph.arknights.bilibili"
```

`--cmd launchApp --package` の部分は起動後に指定のパッケージ名のアプリを自動で起動するためのものであり、必要に応じて変更してください。
:::

#### シミュレーターやアプリを使用した複数起動の方法

1. マルチインスタンス管理ツールを開き、対応するシミュレーターのショートカットを追加します。
2. シミュレーターのショートカットのパスを「起動設定」-「シミュレーターのパス」に入力します。

::: tip
一部のシミュレーターはアプリケーションのショートカットを作成することができ、それを使用してシミュレーターを直接起動し、明日方舟を開くことができます。
:::

::: details 例

```text
多開1:
シミュレーターのパス: C:\ProgramData\Microsoft\Windows\Start Menu\Programs\BlueStacks\多開1.lnk
多開2:
シミュレーターのパス: C:\ProgramData\Microsoft\Windows\Start Menu\Programs\BlueStacks\多開2-明日方舟.lnk
```

:::

もし `シミュレータのパス` を使用して複数の起動操作を行う場合、`起動設定` - `追加コマンド` を空に設定することをお勧めします。

### BlueStacksシミュレータ Hyper-V 启動ごとにポート番号が異なります

`接続設定` で `接続構成` を `BlueStacksシミュレータ` に設定し、その後 `自動接続の検出` と `毎回再検出` をチェックします。

通常、これで接続できるはずです。接続できない場合、複数のシミュレータコアが存在するか、問題が発生している可能性がありますので、以下の追加設定をお読みください。

#### `Bluestacks.Config.Keyword` を指定

::: info 注意
複数の起動機能が有効になっている場合や複数のシミュレータコアがインストールされている場合は、追加の設定が必要です。
:::

`.\config\gui.json` 内で `Bluestacks.Config.Keyword` フィールドを検索し、内容を `"bst.instance.<シミュレータ番号>.status.adb_port"` に設定します。シミュレータ番号はシミュレータパスの `BlueStacks_nxt\Engine` 内で確認できます。

::: details 例
Nougat64 コア：

```json
"Bluestacks.Config.Keyword":"bst.instance.Nougat64.status.adb_port",
```

Pie64_2 コア：（コア名の後の数字は複数の起動コアを示します）

```json
"Bluestacks.Config.Keyword": "bst.instance.Pie64_2.status.adb_port",
```

:::

#### `Bluestacks.Config.Path` を指定

::: info 注意
MAA は現在 `bluestacks.conf` の保存場所をレジストリから読み取ろうとしますが、この機能が機能しない場合は、手動で設定ファイルのパスを指定する必要があります。
:::

1. ブルースタックスシミュレータのデータディレクトリ内にある `bluestacks.conf` ファイルを見つけます。

   - 国際版のデフォルトパスは `C:\ProgramData\BlueStacks_nxt\bluestacks.conf` です。
   - 中国本土版のデフォルトパスは `C:\ProgramData\BlueStacks_nxt_cn\bluestacks.conf` です。

2. 初めて使用する場合は、一度 MAA を実行して、MAA が設定ファイルを自動生成するようにします。

3. **MAA を閉じた後に**、`gui.json` を開き、`Configurations` の下の現在の設定名フィールド（設定-切り替え設定 で確認できます。デフォルトは `Default`）を検索し、`Bluestacks.Config.Path` フィールドを検索して、`bluestacks.conf` の完全なパスを入力します。（スラッシュはエスケープ `\\` を使用してください）

::: details 例
`C:\ProgramData\BlueStacks_nxt\bluestacks.conf` を使用しています。

```json
{
  "Configurations": {
    "Default": {
      "Bluestacks.Config.Path": "C:\\ProgramData\\BlueStacks_nxt\\bluestacks.conf"
      // 残りの構成フィールドは、手入力しないで修正します。
    }
  }
}
```

:::

## 接続設定

対応するエミュレーターの設定を選択してください。リストにない場合は汎用設定を選択してください。汎用設定が利用できない場合は他の利用可能な設定を試し、選択してください。

具体的な違いについては、[ソースコード](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/dev/resource/config.json#L65)をご覧ください。

### MuMu スクリーンショット強化モード

公式版 MuMu 12 V4.0.0 またはそれ以降のバージョンを使用し。方舟国際版などはサポートされていません。<!-- V3.8.13 -->

1. `設定` - `接続設定` で `MuMu のスクリーンショット拡張モードを有効にする` をチェックします。MAAは、スイッチをチェックすると、レジストリを通じてインストールパスを自動的に入力しようとします。

2. `MuMu12 エミュレーターパス` には `MuMuPlayer-12.0` フォルダのパスを入力してください。例: `C:\Program Files\Netease\MuMuPlayer-12.0`。

3. MuMu ネットワークブリッジを使用している場合は、`MuMu ネットワークブリッジモード` を選択し、MuMu マルチインスタンスマネージャ内の対応するエミュレータのシリアル番号を手動で入力する必要があります。例えば、メインインスタンスは `0` です。

### LDPlayer スクリーンショット強化モード

LDPlayerの正式版9 V9.1.32 以降を使用する必要があります。<!-- V9.0.78 -->

1. `設定` - `接続設定` で `LD のスクリーンショット拡張モードを有効にする` にチェックします。MAAは、スイッチをチェックすると、レジストリを通じてインストールパスを自動的に入力しようとします。

2. `LD エミュレータパス` には `LDPlayer9` フォルダのパスを入力してください。例： `C:\leidian\LDPlayer9\`。

3. `インスタンス番号` には LD マルチインスタンス内でのエミュレーターの番号を入力してください。主インスタンスの場合は `0` です。

## タッチモード

1. [Minitouch](https://github.com/DeviceFarmer/minitouch)：Android タッチイベントを操作するための C 言語で書かれたツールで、`evdev` デバイスを操作し、外部プログラムがタッチイベントとジェスチャーをトリガーできる Socket インターフェースを提供します。Android 10 以降、SELinux が `Enforcing` モードの場合、Minitouch は使用できなくなりました。<sup>[出典](https://github.com/DeviceFarmer/minitouch?tab=readme-ov-file#for-android-10-and-up)</sup>
2. [MaaTouch](https://github.com/MaaAssistantArknights/MaaTouch)：MAA による Java で Minitouch を再実装し、Android の `InputDevice` を使用し、追加の機能を付加しました。高バージョンの Android での利用可能性はまだテスト中です。~~テストを手伝ってください~~
3. Adb Input：直接 ADB を呼び出して、Android の `input` コマンドを使用してタッチ操作を行います。最も互換性があり、最も遅い速度です。

## ADB Lite

MAA によって独自に実装された ADB クライアントで、オリジナルの ADB よりも多重の ADB プロセスを開始せずに済み、パフォーマンスの低下を抑えることができますが、一部のスクリーンショット方法は使用できません。

推奨されますが、具体的な利点と欠点はまだフィードバックを得ていません。~~テストを手伝ってください x2~~
