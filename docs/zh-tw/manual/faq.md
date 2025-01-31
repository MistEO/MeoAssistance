---
order: 4
icon: ph:question-fill
---

# 常見問題

如果是第一次使用 MAA，請閱讀[新手上路](./newbie.md)。

::: warning

若 MAA 在某次更新後無法運行，或者純粹是從 MAA 的報錯窗口來到這裡，那大概率是由於運行庫未更新導致的問題。  
出現次數最多的問題都是運行庫問題，而總是有人看不到文檔到處問，所以我們把置頂換成了這個。很氣。

請運行 MAA 目錄下的 `DependencySetup_依赖库安装.bat`，或者在終端中運行以下命令，

```sh
winget install "Microsoft.VCRedist.2015+.x64" --override "/repair /passive /norestart" --force --uninstall-previous --accept-package-agreements && winget install "Microsoft.DotNet.DesktopRuntime.8" --override "/repair /passive /norestart" --force --uninstall-previous --accept-package-agreements
```

或者手動下載並安裝以下<u>**兩個**</u>運行庫來解決問題。

- [Visual C++ 可再發行程序包](https://aka.ms/vs/17/release/vc_redist.x64.exe)
- [.NET 桌面運行時 8](https://aka.ms/dotnet/8.0/windowsdesktop-runtime-win-x64.exe)

:::

## 軟件無法運行/閃退/報錯

### 下載/安裝問題

- 完整 MAA 軟件壓縮包命名格式為 "MAA-`版本`-`平台`-`架構`.zip"，其餘均為無法單獨使用的“零部件”，請仔細閱讀。
  在大部分情況下，您需要使用 x64 架構的 MAA，即您需要下載 `MAA-*-win-x64.zip`，而非 `MAA-*-win-arm64.zip`。
- 如果在某次自動更新後發現功能缺失或無法使用，可能是自動更新過程中出現了問題。請重新下載並解壓完整安裝包。解壓後，將舊 `MAA` 文件夾中的 `config` 文件夾直接拖入新解壓後的 `MAA` 文件夾中。

### 運行庫問題

找到網頁右下角的向上 ↑ 箭頭，點一下它。

### 系統問題

- MAA 不支持 32 位操作系統，不支持 Windows 7 / 8 / 8.1。
- 以上運行庫安裝均需要依賴組件存儲服務（CBS、TrustedInstaller/TiWorker、WinSxS）。
  如果組件存儲服務被破壞，將不能正常安裝。

我們無法提供除重裝系統以外的修復建議，請避免使用未標明精簡項及精簡風險的“精簡版”系統，或者萬年前的舊版系統。

#### Windows N/KN

對於 Windows N/KN（歐洲/韓國），還需安裝[媒體功能包](https://support.microsoft.com/zh-tw/topic/c1c6fffa-d052-8338-7a79-a4bb980a700a)。

#### Windows 7

.NET 8 不支持 Windows 7 / 8 / 8.1 系統<sup>[源](https://github.com/dotnet/core/issues/7556)</sup>，所以 MAA 也同樣不再支持。最後一個可用的 .NET 8 版本為 [`v5.4.0-beta.1.d035.gd2e5001e7`](https://github.com/MaaAssistantArknights/MaaRelease/releases/tag/v5.4.0-beta.1.d035.gd2e5001e7)；最後一個可用的 .NET 4.8 版本為 [`v4.28.8`](https://github.com/MaaAssistantArknights/MaaAssistantArknights/releases/tag/v4.28.8)。尚未確定自行編譯的可行性。

對於 Windows 7，在安裝上文提到的兩個運行庫之前，還需檢查以下補丁是否已安裝：

  1. [Windows 7 Service Pack 1](https://support.microsoft.com/zh-cn/windows/b3da2c0f-cdb6-0572-8596-bab972897f61)
  2. SHA-2 代碼簽名修補程式：
     - KB4474419：[下載連結 1](https://catalog.s.download.windowsupdate.com/c/msdownload/update/software/secu/2019/09/windows6.1-kb4474419-v3-x64_b5614c6cea5cb4e198717789633dca16308ef79c.msu)、[下載連結 2](http://download.windowsupdate.com/c/msdownload/update/software/secu/2019/09/windows6.1-kb4474419-v3-x64_b5614c6cea5cb4e198717789633dca16308ef79c.msu)
     - KB4490628：[下載連結 1](https://catalog.s.download.windowsupdate.com/c/msdownload/update/software/secu/2019/03/windows6.1-kb4490628-x64_d3de52d6987f7c8bdc2c015dca69eac96047c76e.msu)、[下載連結 2](http://download.windowsupdate.com/c/msdownload/update/software/secu/2019/03/windows6.1-kb4490628-x64_d3de52d6987f7c8bdc2c015dca69eac96047c76e.msu)
  3. Platform Update for Windows 7（DXGI 1.2、Direct3D 11.1，KB2670838）：[下載連結 1](https://catalog.s.download.windowsupdate.com/msdownload/update/software/ftpk/2013/02/windows6.1-kb2670838-x64_9f667ff60e80b64cbed2774681302baeaf0fc6a6.msu)、[下載連結 2](http://download.windowsupdate.com/msdownload/update/software/ftpk/2013/02/windows6.1-kb2670838-x64_9f667ff60e80b64cbed2774681302baeaf0fc6a6.msu)

##### .NET 8 應用在 Windows 7 上運行異常的緩解措施 [#8238](https://github.com/MaaAssistantArknights/MaaAssistantArknights/issues/8238)

在 Windows 7 上運行 .NET 8 應用時，會出現內存佔用異常的問題，請參閱下文實施緩解措施。Windows 8/8.1 未經測試，若存在相同問題，請順手發個 Issue 提醒我們補充文檔。

1. 打開 `計算機`，右鍵空白處，點擊屬性，點擊左側 `高級系統設置`，點擊 `環境變量`。
2. 新建一個系統變量，變量名 `DOTNET_EnableWriteXorExecute`，變量值 `0`。
3. 重啟電腦。

## 連接錯誤

- 提示：請參考 [模擬器支援](./device/) 確定正在使用的模擬器通過了支援性測試。
- 若使用了遊戲加速器等軟體，請關閉軟體並 **重開電腦** 後再次嘗試。
- 請檢查您的解壓縮軟體 — 部分情況下使用如 `7z` 或 _其他小眾解壓縮軟體_ 會導致 `Minitouch` 相關檔案出錯

### 方式 1 : 確認 adb 及連接地址正確

- 確認 MAA `設定` - `連接設定` - `adb 路徑` 是否已自動填寫，若已填寫請忽略這步。若未填寫：
  - 找到模擬器安裝路徑，Windows 可在執行模擬器時在工作管理員中右鍵程式點擊 `開啟檔案位置`。
  - 頂層或下層目錄中大概會有一個 `adb.exe`（不一定就叫這個名字，可能叫 `nox_adb.exe`；`HD-adb.exe`；`adb_server.exe` 等等，總之是名字帶 `adb` 的 exe），尋找它，選擇它！
- 確認連接地址填寫正確。可在網上搜尋正在使用的模擬器 adb 偵錯地址是什麼，一般是類似 `127.0.0.1:5555` 這樣的格式（雷電模擬器除外）。

#### 常見安卓模擬器adb通訊埠

- 單開情況 / 多開時 首個模擬器

  針對模擬器單開情況，參考各個模擬器文件和網易遊戲高級遊戲開發工程師@趙青青的[博客](https://www.cnblogs.com/zhaoqingqing/p/15238464.html)，常見安卓模擬器的 adb 通訊埠如下：

    |模擬器|主模擬器預設通訊埠|
    |-|:-:|
    |網易 MuMu 模擬器 6/X|7555|
    |網易 MuMu 模擬器 12|16384|
    |夜神安卓模擬器|62001|
    |逍遙安卓模擬器|21503|
    |藍疊安卓模擬器|5555|
    |雷電安卓模擬器 9|5555 / emulator-5554|

    純數字的預設通訊埠可以直接使用 `127.0.0.1:[port]` 來連接，雷電模擬器進行了封裝，也可以使用 `emulator-5554` 進行連接。

    在 Windows 與 Mac 的 `設定` - `連接設定` - `連接地址` 配置中，如果有情況需要修改則可以參照上表。

- 多開情況

  - 夜神模擬器第一個設備通訊埠為 `62001` ，第二個通訊埠從 `62025` 開始遞增。
  - 網易 MuMu 模擬器 12 版本多開時 adb 通訊埠無規律，可以通過點擊 MuMu 多開器 12，啟動需要執行的模擬器，點擊右上角的 ADB 圖示，即可查看目前正在執行的模擬器 adb 通訊埠資訊。
  - 雷電模擬器從 9 版本開始，模擬器 adb 從本地通訊埠 `5555` 開始逐個遞增 2 ，比如第二個模擬器本地通訊埠為 `5557`。

### 方式 2 : 關閉冗餘 adb 程式

- 關閉 MAA 後查找 `工作管理員` - `詳細資料` 中有無名稱包含 `adb` 的程式（通常和上文中填寫的 `adb` 檔案同名），如有，結束它後重試連接。

### 方式 3 : 重開電腦

- 重開能解決 99% 的問題。（確信

### 方式 4 : 換模擬器

- 請參考 [模擬器支援](./device/)

## 連接正常，但是無操作

部分模擬器內建的 `adb` 版本過於老舊，不支援 `Minitouch` 相關操作。

請使用系統管理員身分打開 MAA，點擊 `MAA 設定` - `連接設定` - `強制替換 ADB`。（建議關閉模擬器、重開 MAA 後再操作，否則可能替換失敗）

模擬器更新後可能會重新覆蓋 adb 檔案。若更新後問題復現，請再次嘗試替換。

如果這樣也無法正常使用，可將 `連接設定` - `觸控模式` 從 `Minitouch` 切換到 `MaaTouch` 再次嘗試。由於 `Adb Input` 操作過於緩慢，請僅將其作為萬不得已的模式。

## 藍疊模擬器每次啟動通訊埠號都不一樣（Hyper-V）

打開 MAA，在 `設定` - `連接設定` 中設定 `連接配置` 為 `藍疊模擬器` ，隨後勾選 `自動檢測連接` 和 `每次重新檢測` （或是在主界面 `開始喚醒` 旁的設定中勾選這兩項）

通常情況下這樣設定就能連上了。如果沒能連上，可能是你安裝了多個模擬器核心或是該功能出現問題，可以根據下面的指導來進行額外設定

### 指定 `Bluestacks.Config.Keyword`

::: info 注意
如果你啟用了多開功能或是安裝了多個模擬器核心，那麼你需要進行額外設定來指定使用的模擬器編號
:::

在 `gui.json` 中添加 `Bluestacks.Config.Keyword` 欄位，內容為 `"bst.instance.模擬器編號.status.adb_port"`，模擬器編號可在模擬器路徑的 `BlueStacks_nxt\Engine` 中看到

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

### 指定 `Bluestacks.Config.Path`

::: info 注意
MAA 現在會嘗試從註冊表中讀取 `bluestacks.conf` 的儲存位置，當該功能失效或出錯時，你需要手動指定配置檔案路徑
:::

1. 在藍疊模擬器的數據目錄下找到 `bluestacks.conf` 這個檔案

    - 國際版預設路徑為 `C:\ProgramData\BlueStacks_nxt\bluestacks.conf`
    - 中國內地版預設路徑為 `C:\ProgramData\BlueStacks_nxt_cn\bluestacks.conf`

2. 如果是第一次使用，請開啟一次 MAA，會在 MAA 的 `config` 目錄下產生 `gui.json`。

3. **先關閉** MAA，**然後** 打開 `gui.json`，找到 `Configurations` 下的目前配置名欄位（可在 設定 - 切換配置 中查看，預設為 `Default`），在其中新增一個欄位 `Bluestacks.Config.Path`，填入 `bluestacks.conf` 的完整路徑。（注意斜槓要用轉義 `\\`）

::: details 範例
以 `C:\ProgramData\BlueStacks_nxt\bluestacks.conf` 為例

```json
{
    "Configurations": {
        "Default": {
            "Bluestacks.Config.Path":"C:\\ProgramData\\BlueStacks_nxt\\bluestacks.conf",
            // 其餘配置欄位，不要手動輸入修改
        }
    }
}
```

:::

## 連接正常，但是操作卡頓、異常或頻繁出錯

- 若使用了 `異形螢幕 UI 適配`，請將其調整為 0。
- 若正在遊玩非陸服用戶端，請先在 `設定` - `遊戲設定` - `用戶端類型` 中選擇用戶端版本。非陸服部分功能可能並非完全適配，請參考對應的外服使用文件。
- 若正在進行自動肉鴿，請參考 [文件](./introduction/integrated-strategy.md)，並在 `任務設定` - `自動肉鴿` - `肉鴿主題` 中正確選擇主題。
- `Adb Input` 觸控模式操作緩慢為正常情況，如需自動戰鬥等請嘗試切換其他模式。

### 提示截圖用時較長 / 過長

- MAA 目前支援 `RawByNc` 、 `RawWithGzip` 、 `Encode` 三種截圖方式，當執行任務平均截圖耗時 >400 / >800 時會輸出一次提示訊息（單次任務只會輸出一次）
- `設定 - 連線設定` 中會顯示近30次截圖耗時的 最小/平均/最大值，每10次截圖刷新
- 自動戰鬥類功能（如自動肉鴿）受截圖耗時影響較大
- 此項耗時與MAA無關，與電腦效能、目前佔用或模擬器相關，可嘗試清理後台/更換模擬器/升級電腦配置

## 管理員權限相關問題

MAA 理應無需以 Windows UAC 管理員權限運行即可實現所有功能。現與管理員權限有關的功能主要包括：

1. `自動檢測連接`：當目標模擬器以管理員身份運行時需要管理員權限。
2. `完成後關閉模擬器`：當目標模擬器以管理員身份運行時需要管理員權限。
3. `開機自動啟動 MAA`：無法在管理員身份下設置開機自啟。
4. 當 MAA 被錯誤解壓到需要管理員權限進行寫入的路徑時，例如 `C:\`、`C:\Program Files\`。

有報告稱關閉了 UAC 的系統存在“即使沒有右鍵選擇管理員運行也會以管理員權限啟動”的問題，建議開啟 UAC 以避免意料之外的提權行為。

## 檔案下載速度慢

請嘗試使用 [MAA 下載站](https://ota.maa.plus/MaaAssistantArknights/MaaRelease/releases/download/) 下載。

我們的小水管頻寬較低，且流量有限。雖然搭出來就是給大家用的，但也希望不要濫用，若 github 或其他鏡像站可正常下載，建議優先選擇。

## 下載到一半提示 “登錄” / “鑑權”

請使用 瀏覽器 / IDM / FDM 等正規下載器下載檔案，**不要用傻逼迅雷！**
