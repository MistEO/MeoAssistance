---
order: 1
icon: ri:windows-fill
---

# Windows Emulator

:::warning
TODO: replace these Chinese emulators links with English ones
:::

The following emulators are displayed randomly in no particular order.

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
        name: 'Bluestacks 5',
        link: 'https://www.bluestacks.com/',
        note: 'Fully compatible. Need to turn on `Allow ADB connections` in the emulator `Settings` - `Engine Settings`. Known to be compatible with Hyper-V.\n\n- Recommended to download [Offline Installer](https://support.bluestacks.com/hc/en-us/articles/4402611273485-BlueStacks-5-offline-installer) to avoid slow and bundled installation; recommend installing [Android 11](https://support.bluestacks.com/hc/en-us/articles/4402611273485-BlueStacks-5-offline-installer#:~:text=To%20install%20BlueStacks%205%20Android%2011) version; to uninstall it, please use the official [Uninstall Tool](https://support.bluestacks.com/hc/en-us/articles/360057724751-How-to-uninstall-BlueStacks-5-BlueStacks-X-and-BlueStacks-Services-completely-from-your-PC) to get rid of the residue.\n- If the adb port number keeps changing irregularly and is different every time you start it, it may be because your computer has [Hyper-V](https://support.bluestacks.com/hc/en-us/articles/4415238471053-System-requirements-for-BlueStacks-5-on-Hyper-V-enabled-Windows-10-and-11) enabled. MAA will now try to automatically read the port number within the Blue Stacker emulator configuration file, if this does not work/you have a need to multi-open/have more than one emulator kernel installed, please refer to the [Connection (TODO)](../connection.html#the-port-number-of-bluestack-emulator-hyper-v-is-different-every-time-it-is-started) to make changes. Since Hyper-V runs as administrator, operations that don\'t involve adb such as auto-shutdown of the emulator, auto-detect connection, etc. also need to run MAA as administrator.',
    },
    {
        name: 'MuMu Emulator 12',
        link: 'https://mumu.163.com/',
        note: 'Fully compatible, with additional support for the [Screenshot Enhanced Mode](../connection.html#mumu-screenshot-enhanced-mode). Known to be compatible with Hyper-V.\n\n- The "Exit emulator when done" function may occasionally be abnormal, if you encounter it, please contact MuMu official for feedback.\n- You need to check the port information of the corresponding instance through the ADB button of MuMu 12 Multiple Opener when you open more than one instance, and change the port number of the connection address in MAA `Settings` - `Connection Settings` to the corresponding port.',
    },
    {
        name: 'LDPlayer',
        link: 'https://www.ldmnq.com/',
        note: 'Fully compatible, with additional support for the [Screenshot Enhanced Mode](../connection.html#ld-screenshot-enhanced-mode). Known to be compatible with Hyper-V.\n\n- LDPlayer 9 installer will automatically and silently disable Hyper-V during the installation process. Please pay attention if you have related needs.',
    },
    {
        name: 'Nox',
        link: 'https://www.yeshen.com/',
        note: 'Fully compatible, but less tested. Known to be compatible with Hyper-V.',
    },
    {
        name: 'Memu',
        link: 'https://www.xyaz.cn/',
        note: 'Fully compatible, but less tested.',
    },
]);

const partiallySupport = shuffleArray([
    {
        name: 'MuMu Emulator 6',
        link: 'https://mumu.163.com/update/win/',
        note: 'Support has been dropped since MAA v5.1.0 and NetEase has stopped maintaining it on 15-08-2023.\n\n- No longer support auto-detect connection, need to use generic connection configuration and manually configure adb path and connection address.\n- Need to run `Forced Replacement of ADB` in `Settings` - `Connection` to use efficient touch modes such as Minitouch, MaaTouch.\n- You need to run MAA with administrator privileges to use the "Exit Emulator When Done" function.\n- MuMu 6 default resolution is not supported, you need to change it to `1280x720`, `1920x1080` and other 16:9 ratio.\n- MuMu 6 multi-open uses the same adb port, so it can\'t support multi-open MuMu 6.',
    },
    {
        name: 'Windows Subsystem for Android™️',
        link: 'https://learn.microsoft.com/en-us/windows/android/wsa/',
        note: 'Support has been dropped since MAA v5.2.0 and will be discontinued by Microsoft on 05-03-2025.\n\n- Requires the use of [custom connection](../details.html#Custom connection) is required.\n- WSA 2204 or later (the version number is in the `About` page of the subsystem settings), select `Common Configuration` for the connection configuration.\n- WSA 2203 or older (the version number is at the top of the subsystem setup page), for the connection configuration select `WSA Older Versions`.\n- Since this software only supports 720p or higher `16:9` resolution better, please manually drag the window size as close to the 16:9 ratio as possible. (If your monitor is 16:9, you can press `F11` to go full screen).\n- Please try to make sure that Arknights is in the foreground and no other Android apps are running in the foreground at the same time, otherwise it may cause the game to pause or the task recognition error.\n- WSA\'s screenshots often somehow capture a white screen, resulting in recognition of abnormalities, or not recommended to use.',
    },
    {
        name: 'AVD',
        link: 'https://developer.android.com/studio/run/managing-avds',
        note: 'Theoretical support.\n\n- Starting from Android 10, Minitouch is no longer available when SELinux is in `Enforcing` mode, please switch to other touch modes, or switch SELinux **temporary** to `Permissive` mode.\n- AVD is made for debugging, it is more recommended to use other emulators designed for gaming.',
    },
    {
        name: 'Google Play Games (Developer)',
        link: 'https://developer.android.com/games/playgames/emulator?hl=zh-cn',
        note: 'Theoretical support. Hyper-V must be enabled, and a Google account must be logged in.\n\n- You need to use [custom connection](../connection.html) to connect, the ADB port is `6520`.\n- Due to the SELinux policy of Android 10 and later versions, Minitouch cannot work properly, please switch to other touch modes.\n- The first connection after each emulator startup will fail, you need to check `Attempt to kill and restart ADB process after connection failure`.',
    },
]);

const notSupport = shuffleArray([
    {
        name: 'Google Play Games',
        link: 'https://play.google.com/googleplaygames',
        note: 'Not supported, [Consumer Client](https://developer.android.com/games/playgames/pg-emulator#installing-game-consumer) cannot use ADB.',
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

## ✅ Fully supported

<ClientOnly><div v-html="fullySupportHtml"></div></ClientOnly>

## ⚠️ Partially supported

<ClientOnly><div v-html="partiallySupportHtml"></div></ClientOnly>

## 🚫 Unsupported

<ClientOnly><div v-html="notSupportHtml"></div></ClientOnly>
