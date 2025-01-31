---
order: 1
icon: ri:windows-fill
---

# Windows 模拟器

以下模拟器排序为随机生成，排名不分先后。

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
        name: '蓝叠模拟器 5',
        link: 'https://www.bluestacks.cn/',
        note: '完美支持。需要在模拟器 `设置` - `引擎设置` 中打开 `允许ADB连接`。',
    },
    {
        name: '蓝叠模拟器 5 国际版',
        link: 'https://www.bluestacks.com/tw/index.html',
        note: '完美支持，需要在模拟器 `设定` - `进阶` 中打开 `Android调试桥`。已知兼容 Hyper-V。\n\n- 推荐下载 [离线安装包](https://support.bluestacks.com/hc/zh-tw/articles/4402611273485-BlueStacks-5-%E9%9B%A2%E7%B7%9A%E5%AE%89%E8%A3%9D%E7%A8%8B%E5%BC%8F)，避免缓慢和捆绑安装；推荐安装 [Android 11](https://support.bluestacks.com/hc/zh-tw/articles/4402611273485-BlueStacks-5-%E9%9B%A2%E7%B7%9A%E5%AE%89%E8%A3%9D%E7%A8%8B%E5%BC%8F#:~:text=%E5%AE%89%E8%A3%9D%20BlueStacks%205%20%E7%9A%84%20Android%2011) 版本；卸载请使用官方提供的 [卸载工具](https://support.bluestacks.com/hc/zh-tw/articles/360057724751-%E5%A6%82%E4%BD%95%E5%BE%9E%E6%82%A8%E7%9A%84%E9%9B%BB%E8%85%A6%E4%B8%8A%E5%AE%8C%E5%85%A8%E7%A7%BB%E9%99%A4-BlueStacks-5-BlueStacks-X-%E5%92%8C-BlueStacks-%E6%9C%8D%E5%8B%99) 以清除残留。\n- 若 ADB 端口号不断的无规律变动，每次启动都不相同，可能是因为您的电脑开启了 [Hyper-V](https://support.bluestacks.com/hc/zh-tw/articles/4415238471053-BlueStacks-5-%E6%94%AF%E6%8F%B4-Hyper-V-%E7%9A%84-Windows-10-%E5%92%8C-11-%E4%B8%8A%E7%9A%84%E9%9B%BB%E8%85%A6%E8%A6%8F%E6%A0%BC%E9%9C%80%E6%B1%82)。MAA 现在会尝试自动读取蓝叠模拟器配置文件内的端口号，若该功能失效/你有多开需求/安装了多个模拟器核心，请参考 [连接设置](../connection.html#蓝叠模拟器-hyper-v-每次启动端口号都不一样) 做出修改。由于 Hyper-V 以管理员身份运行，如自动关闭模拟器、自动检测连接等不涉及 ADB 的操作同样需要以管理员身份运行 MAA。',
    },
    {
        name: 'MuMu 模拟器 12',
        link: 'https://mumu.163.com/',
        note: '完美支持，且额外支持[截图增强模式](../connection.html#mumu-截图增强模式)。已知兼容 Hyper-V。\n\n- “完成后退出模拟器”功能可能偶现异常，如果遇到请向 MuMu 官方反馈。',
    },
    {
        name: '雷电模拟器',
        link: 'https://www.ldmnq.com/',
        note: '完美支持，且额外支持[截图增强模式](../connection.html#雷电截图增强模式)。已知兼容 Hyper-V。\n\n- 雷电 9 安装器在安装过程中会自动静默关闭 Hyper-V，若有相关需求请留意。',
    },
    {
        name: '夜神模拟器',
        link: 'https://www.yeshen.com/',
        note: '完美支持，但测试较少。已知兼容 Hyper-V。',
    },
    {
        name: '逍遥模拟器',
        link: 'https://www.xyaz.cn/',
        note: '完美支持，但测试较少。',
    },
]);

const partiallySupport = shuffleArray([
    {
        name: 'MuMu 模拟器 6',
        link: 'https://mumu.163.com/update/win/',
        note: '自 MAA v5.1.0 起放弃支持，网易已在 2023.8.15 停止维护。\n\n- 不再支持自动检测连接，需使用通用连接配置，并手动配置 ADB 路径和连接地址。\n- 需要在 `设置` - `连接设置` 中运行 `强制替换 ADB`，才能使用 Minitouch, MaaTouch 等高效的触控模式。\n- 需要使用管理员权限运行 MAA 才能使用“完成后退出模拟器”相关功能。\n- 不支持使用 MuMu 6 默认的几个奇葩分辨率，需要改成 `1280x720`、`1920x1080` 等 16:9 比例。\n- MuMu 6 多开使用的是同一个 ADB 端口，所以无法支持多开的 MuMu 6。',
    },
    {
        name: '适用于 Android™️ 的 Windows 子系统',
        link: 'https://docs.microsoft.com/zh-cn/windows/android/wsa/',
        note: '自 MAA v5.2.0 起放弃支持，微软将在 2025.3.5 停止维护。\n\n- 需要使用 [自定义连接](../connection.html) 的方式来连接。\n- WSA 2204 或更高版本（版本号在子系统设置的 `关于` 页面中），连接配置选择 `通用配置`。\n- WSA 2203 或更老版本（版本号在子系统设置页面的上方），连接配置选择 `WSA 旧版本`。\n- 由于本软件仅对 720p 以上 `16:9` 分辨率支持较好，所以请手动拖动窗口大小，尽量贴近 16:9 比例。（如果你的显示器是 16:9 的，可以直接按 `F11` 全屏）。\n- 任务运行过程中请尽量保证明日方舟在前台且无其他安卓应用同时在前台运行，否则可能导致游戏暂停运行或任务识别错误。\n- WSA 的截图经常莫名其妙截出来一个白屏，导致识别异常，还是不推荐使用。',
    },
    {
        name: 'AVD',
        link: 'https://developer.android.com/studio/run/managing-avds',
        note: '理论支持。\n\n- 从 Android 10 开始，Minitouch 在 SELinux 为 `Enforcing` 模式时不再可用，请切换至其他触控模式，或将 SELinux **临时**切换为 `Permissive` 模式。\n- AVD 是为调试而生的，更建议使用其他为游戏而设计的模拟器。',
    },
    {
        name: 'Google Play 游戏（开发者）',
        link: 'https://developer.android.com/games/playgames/emulator?hl=zh-cn',
        note: '理论支持。必须开启 Hyper-V，且必须登录谷歌账户。\n\n- 需要使用 [自定义连接](../connection.html) 的方式来连接，ADB 端口为 `6520`。\n- 由于 Android 10 及更新版本的 SELinux 策略，Minitouch 无法正常工作，请切换到其他触控模式。\n- 每次启动模拟器后的首次连接都会失败，需勾选 `连接失败后尝试关闭并重启ADB进程`。',
    },
]);

const notSupport = shuffleArray([
    {
        name: 'Google Play 游戏',
        link: 'https://play.google.com/googleplaygames',
        note: '不支持，[玩家客户端](https://developer.android.com/games/playgames/pg-emulator?hl=zh-cn#installing-game-consumer)无法连接 ADB。',
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

## ✅ 完美支持

<ClientOnly><div v-html="fullySupportHtml"></div></ClientOnly>

## ⚠️ 部分支持

<ClientOnly><div v-html="partiallySupportHtml"></div></ClientOnly>

## 🚫 不支持

<ClientOnly><div v-html="notSupportHtml"></div></ClientOnly>
