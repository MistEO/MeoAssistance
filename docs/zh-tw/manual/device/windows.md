---
order: 1
icon: ri:windows-fill
---

# Windows 模擬器

以下模擬器排序為隨機生成，排名不分先後。

<script setup>
import MarkdownIt from 'markdown-it'
import MarkdownItAnchor from 'markdown-it-anchor'

const shuffleArray = (array) => {
    for (let i = array.length - 1; i > 0; i--) {
        const j = Math.floor(Math.random() * (i + 1));
        [array[i], array[j]] = [array[j], array[i]];
    }
    return array;
}

const fullySupport = shuffleArray([
    {
        name: '藍疊模擬器 5 國際版',
        link: 'https://www.bluestacks.com/tw/index.html',
        note: '完美支援，需要在模擬器 `設定` - `進階` 中開啟 `Android除錯橋`。已知相容 Hyper-V。\n\n- 推薦下載 [離線安裝包](https://support.bluestacks.com/hc/zh-tw/articles/4402611273485-BlueStacks-5-%E9%9B%A2%E7%B7%9A%E5%AE%89%E8%A3%9D%E7%A8%8B%E5%BC%8F)，避免緩慢和捆綁安裝；推薦安裝 [Android 11](https://support.bluestacks.com/hc/zh-tw/articles/4402611273485-BlueStacks-5-%E9%9B%A2%E7%B7%9A%E5%AE%89%E8%A3%9D%E7%A8%8B%E5%BC%8F#:~:text=%E5%AE%89%E8%A3%9D%20BlueStacks%205%20%E7%9A%84%20Android%2011) 版本；解除安裝請使用官方提供的 [解除安裝工具](https://support.bluestacks.com/hc/zh-tw/articles/360057724751-%E5%A6%82%E4%BD%95%E5%BE%9E%E6%82%A8%E7%9A%84%E9%9B%BB%E8%85%A6%E4%B8%8A%E5%AE%8C%E5%85%A8%E7%A7%BB%E9%99%A4-BlueStacks-5-BlueStacks-X-%E5%92%8C-BlueStacks-%E6%9C%8D%E5%8B%99) 以清除殘留。\n- 若 ADB 埠號不斷的無規律變動，每次啟動都不相同，可能是因為您的電腦開啟了 [Hyper-V](https://support.bluestacks.com/hc/zh-tw/articles/4415238471053-BlueStacks-5-%E6%94%AF%E6%8F%B4-Hyper-V-%E7%9A%84-Windows-10-%E5%92%8C-11-%E4%B8%8A%E7%9A%84%E9%9B%BB%E8%85%A6%E8%A6%8F%E6%A0%BC%E9%9C%80%E6%B1%82)。MAA 現在會嘗試自動讀取藍疊模擬器配置檔案內的埠號，若該功能失效/你有多開需求/安裝了多個模擬器核心，請參考 [連線設定](../connection.html#藍疊模擬器-hyper-v-每次啟動埠號都不一樣) 做出修改。由於 Hyper-V 以管理員身份執行，如自動關閉模擬器、自動檢測連線等不涉及 ADB 的操作同樣需要以管理員身份執行 MAA。',
    },
    {
        name: 'MuMu 模擬器 12',
        link: 'https://mumu.163.com/',
        note: '完美支援，且額外支援[截圖增強模式](../connection.html#mumu-截圖增強模式)。已知相容 Hyper-V。\n\n- “完成後退出模擬器”功能可能偶現異常，如果遇到請向 MuMu 官方反饋。',
    },
    {
        name: '雷電模擬器',
        link: 'https://www.ldmnq.com/',
        note: '完美支援，且額外支援[截圖增強模式](../connection.html#雷電截圖增強模式)。已知相容 Hyper-V。\n\n- 雷電 9 安裝器在安裝過程中會自動靜默關閉 Hyper-V，若有相關需求請留意。',
    },
    {
        name: '夜神模擬器',
        link: 'https://www.yeshen.com/',
        note: '完美支援，但測試較少。已知相容 Hyper-V。',
    },
    {
        name: '逍遙模擬器',
        link: 'https://www.xyaz.cn/',
        note: '完美支援，但測試較少。',
    },
]);

const partiallySupport = shuffleArray([
    {
        name: 'MuMu 模擬器 6',
        link: 'https://mumu.163.com/update/win/',
        note: '自 MAA v5.1.0 起放棄支援，網易已在 2023.8.15 停止維護。\n\n- 不再支援自動檢測連線，需使用通用連線配置，並手動配置 ADB 路徑和連線地址。\n- 需要在 `設定` - `連線設定` 中執行 `強制替換 ADB`，才能使用 Minitouch, MaaTouch 等高效的觸控模式。\n- 需要使用管理員許可權執行 MAA 才能使用“完成後退出模擬器”相關功能。\n- 不支援使用 MuMu 6 預設的幾個奇葩解析度，需要改成 `1280x720`，`1920x1080` 等 16:9 比例。\n- MuMu 6 多開使用的是同一個 ADB 埠，所以無法支援多開的 MuMu 6。',
    },
    {
        name: '適用於 Android™️ 的 Windows 子系統',
        link: 'https://docs.microsoft.com/zh-cn/windows/android/wsa/',
        note: '自 MAA v5.2.0 起放棄支援，微軟將在 2025.3.5 停止維護。\n\n- 需要使用 [自定義連線](../connection.html) 的方式來連線。\n- WSA 2204 或更高版本（版本號在子系統設定的 `關於` 頁面中），連線配置選擇 `通用配置`。\n- WSA 2203 或更老版本（版本號在子系統設定頁面的上方），連線配置選擇 `WSA 舊版本`。\n- 由於本軟體僅對 720p 以上 `16:9` 解析度支援較好，所以請手動拖動視窗大小，儘量貼近 16:9 比例。（如果你的顯示器是 16:9 的，可以直接按 `F11` 全屏）。\n- 任務執行過程中請儘量保證明日方舟在前台且無其他安卓應用同時在前台執行，否則可能導致遊戲暫停執行或任務識別錯誤。\n- WSA 的截圖經常莫名其妙截出來一個白螢幕，導致辨識異常，還是不推薦使用。',
    },
    {
        name: 'AVD',
        link: 'https://developer.android.com/studio/run/managing-avds',
        note: '理論支援。\n\n- 從 Android 10 開始，Minitouch 在 SELinux 為 `Enforcing` 模式時不再可用，請切換至其他觸控模式，或將 SELinux **臨時**切換為 `Permissive` 模式。\n- AVD 是為除錯而生的，更建議使用其他為遊戲而設計的模擬器。',
    },
    {
        name: 'Google Play 遊戲（開發者）',
        link: 'https://developer.android.com/games/playgames/emulator?hl=zh-cn',
        note: '理論支援。必須開啟 Hyper-V，且必須登入 Gooole 帳戶。\n\n- 需要使用 [自定義連線](../connection.html) 的方式來連線，ADB 埠為 `6520`。\n- 由於 Android 10 及更新版本的 SELinux 策略，Minitouch 無法正常工作，請切換到其他觸控模式。\n- 每次啟動模擬器後的首次連線都會失敗，需勾選 `連線失敗後嘗試關閉並重啟ADB程式`。',
    },
]);

const notSupport = shuffleArray([
    {
        name: 'Google Play 遊戲',
        link: 'https://play.google.com/googleplaygames',
        note: '不支援，[玩家客戶端](https://developer.android.com/games/playgames/pg-emulator?hl=zh-cn#installing-game-consumer)未開放 ADB 埠。',
    },
]);

const md = new MarkdownIt();
md.use(MarkdownItAnchor);

const fullySupportHtml = md.render(fullySupport.map(simulator => `
### ✅ ${simulator.link ? `[${simulator.name}](${simulator.link})` : simulator.name}
${simulator.note}
`).join(''));
const partiallySupportHtml = md.render(partiallySupport.map(simulator => `
### ⚠️ ${simulator.link ? `[${simulator.name}](${simulator.link})` : simulator.name}
${simulator.note}
`).join(''));
const notSupportHtml = md.render(notSupport.map(simulator => `
### 🚫 ${simulator.link ? `[${simulator.name}](${simulator.link})` : simulator.name}
${simulator.note}
`).join(''));
</script>

## ✅ 完美支援

<ClientOnly><div v-html="fullySupportHtml"></div></ClientOnly>

## ⚠️ 部分支援

<ClientOnly><div v-html="partiallySupportHtml"></div></ClientOnly>

## 🚫 不支援

<ClientOnly><div v-html="notSupportHtml"></div></ClientOnly>
