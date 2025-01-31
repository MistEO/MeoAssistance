---
order: 3
icon: mdi:plug
---

# 連接設定

## ADB 路徑

:::info 技術細節
自動檢測使用的是模擬器的 ADB，但有時自動檢測會出現問題，此時就需要手動設定。
`強制替換 ADB` 是下載 Google 提供的 ADB 後再進行替換，如果自己設定 Google 的 ADB 即可一勞永逸。
:::

### 使用模擬器提供的 ADB

前往模擬器安裝路徑，Windows 可在模擬器執行時，在工作管理員中右鍵程式點選 `開啟檔案位置`。

頂層或下層目錄中應該會有一個名字中帶有 `adb` 的 exe 檔案，可以使用搜索功能，然後選擇。

:::details 一些範例
`adb.exe` `HD-adb.exe` `adb_server.exe` `nox_adb.exe`
:::

### 使用 Google 提供的 ADB

[點選下載](https://dl.google.com/android/repository/platform-tools-latest-windows.zip) 後解壓縮，然後選擇其中的 `adb.exe`。

推薦直接解壓縮到 MAA 資料夾下，這樣可以直接在 ADB 路徑中填寫 `.\platform-tools\adb.exe`，也可以隨著 MAA 資料夾一起移動。

## 連接地址

::: tip
執行在本機的模擬器連接地址應該是 `127.0.0.1:<埠號>` 或 `emulator-<四位數字>`。
:::

### 獲取埠號

#### 模擬器相關文件及參考埠

- [Bluestacks 5](https://support.bluestacks.com/hc/zh-tw/articles/360061342631-%E5%A6%82%E4%BD%95%E5%B0%87%E6%82%A8%E7%9A%84%E6%87%89%E7%94%A8%E5%BE%9EBlueStacks-4%E8%BD%89%E7%A7%BB%E5%88%B0BlueStacks-5#%E2%80%9C2%E2%80%9D) `5555`
- [MuMu Pro](https://mumu.163.com/mac/function/20240126/40028_1134600.html) `16384`
- [MuMu 12](https://mumu.163.com/help/20230214/35047_1073151.html) `16384`
- [逍遙](https://bbs.xyaz.cn/forum.php?mod=viewthread&tid=365537) `21503`
- [夜神](https://support.yeshen.com/zh-CN/qt/ml) `62001`

其他模擬器可參考 [趙青青的部落格](https://www.cnblogs.com/zhaoqingqing/p/15238464.html)。

#### 獲取多開埠

- MuMu 12 多開器右上角可檢視正在執行的多開埠。
- Bluestacks 5 模擬器設定內可檢視當前的多開埠。
- _待補充_

::: details 備選方案

- 方案 1 : 使用 ADB 命令檢視模擬器埠

  1. 啟動**一個**模擬器，並確保沒有其他安卓裝置連接在此電腦上。
  2. 在存放有 ADB 可執行檔案的資料夾中啟動終端。
  3. 執行以下命令。

  ```sh
  # Windows 命令提示符
  adb devices
  # Windows PowerShell
  .\adb devices
  ```

  以下為輸出內容的例子：

  ```text
  List of devices attached
  127.0.0.1:<埠號>   device
  emulator-<四位數字>  device
  ```

  使用 `127.0.0.1:<埠>` 或 `emulator-<四位數字>` 作為連接地址。

- 方案 2 : 查詢已建立的 ADB 連接

  1. 執行方案 1。
  2. 按下 `Win+S` 開啟搜尋欄，輸入 `資源監視器` 並開啟。
  3. 切換到 `網路` 索引標籤，在 `接聽連接埠` 的名稱列中查詢模擬器程式名，如 `HD-Player.exe`。
  4. 記錄模擬器程式的所有接聽連接埠。
  5. 在 `TCP 連接` 的名稱列中查詢 `adb.exe`，在遠端連接埠列中與模擬器接聽連接埠一致的埠，即為模擬器除錯埠。

:::

### 藍疊模擬器 Hyper-V 每次啟動埠號都不一樣

在 `連接設定` 中設定 `連接配置` 為 `藍疊模擬器` ，隨後勾選 `自動檢測連接` 和 `每次重新檢測`。

通常情況下這樣就可以連接。如果無法連接，可能是存在多個模擬器核心，或出現了問題，請閱讀下文進行額外設定。

#### 指定 `Bluestacks.Config.Keyword`

::: info 注意
如果啟用了多開功能或安裝了多個模擬器核心，則需要進行額外設定來指定使用的模擬器編號
:::

在 `.\config\gui.json` 中搜索 `Bluestacks.Config.Keyword` 欄位，內容為 `"bst.instance.<模擬器編號>.status.adb_port"`，模擬器編號可在模擬器路徑的 `BlueStacks_nxt\Engine` 中看到

::: details 範例
Nougat64 核心：

```json
"Bluestacks.Config.Keyword":"bst.instance.Nougat64.status.adb_port",
```

Pie64_2 核心：（核心名稱後的數字代表這是一個多開核心）

```json
"Bluestacks.Config.Keyword": "bst.instance.Pie64_2.status.adb_port",
```

:::

#### 指定 `Bluestacks.Config.Path`

::: info 注意
MAA 現在會嘗試從登錄檔中讀取 `bluestacks.conf` 的儲存位置，當該功能無法工作時，則需要手動指定配置檔案路徑
:::

1. 在藍疊模擬器的資料目錄下找到 `bluestacks.conf` 這個檔案

   - 國際版預設路徑為 `C:\ProgramData\BlueStacks_nxt\bluestacks.conf`
   - 中國版預設路徑為 `C:\ProgramData\BlueStacks_nxt_cn\bluestacks.conf`

2. 如果是第一次使用，請執行一次 MAA，使 MAA 自動生成配置檔案。

3. **先關閉** MAA，**然後**開啟 `gui.json`，找到 `Configurations` 下的當前配置名欄位（可在 設定-切換配置 中檢視，預設為 `Default`），在其中搜索欄位 `Bluestacks.Config.Path`，填入 `bluestacks.conf` 的完整路徑。（注意斜槓要用轉義 `\\`）

::: details 範例
以 `C:\ProgramData\BlueStacks_nxt\bluestacks.conf` 為例

```json
{
  "Configurations": {
    "Default": {
      "Bluestacks.Config.Path": "C:\\ProgramData\\BlueStacks_nxt\\bluestacks.conf"
      // 其餘配置欄位，不要手動輸入修改
    }
  }
}
```

:::

## 連接配置

需選擇對應模擬器的配置，若列表中沒有則選擇通用配置。若通用配置不可用，請嘗試並選擇其他任一可用的配置。

具體區別可以閱讀[原始碼](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/dev/resource/config.json#L65)。

### MuMu 截圖增強模式

需使用中國版或方舟專版 MuMu 12 V4.0.0 及更新版本。國際版等暫不支援。<!-- V3.8.13 -->

1. `設定` - `連接設定`，勾選 `啟用 MuMu 截圖增強模式`，MAA 會在勾選開關時嘗試通過註冊表自動填寫安裝路徑。

2. `MuMu 安裝路徑` 填寫 `MuMuPlayer-12.0` 或 `YXArkNights-12.0` 資料夾的路徑，如 `C:\Program Files\Netease\MuMuPlayer-12.0`。

3. 若正在使用 MuMu 網絡橋接，則需勾選 `MuMu 網絡橋接模式` 後手動填寫 MuMu 多開器內對應模擬器的序號，如主多開為 `0`。

### 雷電截圖增強模式

需使用中國版或國際版雷電模擬器 9 V9.1.32 及更新版本。<!-- V9.0.78 -->

1. `設定` - `連接設定`，勾選 `啟用 LD 截圖增強模式`，MAA 會在勾選開關時嘗試通過註冊表自動填寫安裝路徑。

2. `LD 安裝路徑` 填寫 `LDPlayer9` 資料夾的路徑，如 `C:\leidian\LDPlayer9\`。

3. `例項編號` 填寫雷電多開器內對應模擬器的編號（ID），如主多開為 `0`。

## 觸控模式

1. [Minitouch](https://github.com/DeviceFarmer/minitouch)：使用 C 編寫的 Android 觸控事件器，操作 `evdev` 裝置，提供 Socket 介面供外部程式觸發觸控事件和手勢。從 Android 10 開始，Minitouch 在 SELinux 為 `Enforcing` 模式時不再可用。<sup>[源](https://github.com/DeviceFarmer/minitouch?tab=readme-ov-file#for-android-10-and-up)</sup>
2. [MaaTouch](https://github.com/MaaAssistantArknights/MaaTouch)：由 MAA 基於 Java 對 Minitouch 的重新實現，使用安卓原生的 `InputDevice`，並添加了額外特性。高版本 Android 可用性尚待測試。~~幫我們做做測試~~
3. Adb Input：直接呼叫 ADB 使用安卓的 `input` 命令進行觸控操作，相容性最強，速度最慢。

## ADB Lite

由 MAA 獨立實現的 ADB Client，使用 TCP 直接與 ADB Server 通訊。相較原版 ADB 可以避免不停開啟多個 ADB 程式，減少效能開銷，但部分截圖方式不可用。<sup>[PR](https://github.com/MaaAssistantArknights/MaaAssistantArknights/pull/3315)</sup>

推薦啟用，但具體優缺點尚待反饋。~~幫我們做做測試 x2~~

## MAA 和模擬器多開

::: info
若需要多開模擬器同時操作，可將 MAA 資料夾複製多份，使用 **不同的 MAA**、**同一個 adb.exe**、**不同的連接地址** 來進行連接。
:::

### 自動啟動多開模擬器

以[藍疊國際版](./device/windows.md)為例，介紹兩種自動啟動多開模擬器的方式。

#### 透過附加命令啟動

1. 啟動**單一**模擬器多開。
2. 開啟工作管理員，找到對應模擬器程式，轉到詳細資訊索引標籤，右鍵列首，點選 `選擇列`，勾選 `命令列`。
3. 在多出來的 `命令列` 列中找到 `...\Bluestacks_nxt\HD-Player.exe"` 後的內容。
4. 將找到的類似於 `--instance Nougat32` 的內容填寫到 `啟動設定` - `附加命令` 中。

::: note
操作結束後建議重新隱藏 `步驟 2` 中開啟的 `命令列` 列以防止卡頓
:::

::: details 範例

```text
多開1:
模擬器路徑: C:\Program Files\BlueStacks_nxt\HD-Player.exe
附加命令: --instance Nougat32 --cmd launchApp --package "com.hypergryph.arknights"
多開2:
模擬器路徑: C:\Program Files\BlueStacks_nxt\HD-Player.exe
附加命令: --instance Nougat32_1 --cmd launchApp --package "com.hypergryph.arknights.bilibili"
```

其中 `--cmd launchApp --package` 部分為啟動後自動執行指定包名應用，可自行更改。
:::

#### 透過模擬器的快捷方式啟動

部分模擬器支援建立應用快捷方式，可直接使用應用的快捷方式直接啟動模擬器並開啟明日方舟。

1. 開啟多開管理器，新增對應模擬器的快捷方式。
2. 將模擬器快捷方式的路徑填入 `啟動設定` - `模擬器路徑` 中

::: details 範例

```text
多開1:
模擬器路徑: C:\ProgramData\Microsoft\Windows\Start Menu\Programs\BlueStacks\多開1.lnk
多開2:
模擬器路徑: C:\ProgramData\Microsoft\Windows\Start Menu\Programs\BlueStacks\多開2-明日方舟.lnk
```

:::

若使用 `模擬器路徑` 進行多開操作，建議將 `啟動設定` - `附加命令` 置空。
