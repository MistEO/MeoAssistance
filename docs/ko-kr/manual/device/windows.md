---
order: 1
icon: ri:windows-fill
---

# Windows 지원

아래의 목록은 무작위순이며, 성능순이 아닙니다.

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
        link: 'https://www.bluestacks.com/ko/',
        note: '완전히 호환됩니다. 에뮬레이터의 `설정` - `고급 설정`에서 `ADB 기능`을 켜야 합니다. Hyper-V와 호환되는 것으로 알려져 있습니다.\n\n- 느리고 번들로 제공되는 설치를 피하기 위해 [오프라인 설치 프로그램](https://support.bluestacks.com/hc/en-us/articles/4402611273485-BlueStacks-5-offline-installer)을 다운로드하는 것이 좋습니다; [Android 11](https://support.bluestacks.com/hc/en-us/articles/4402611273485-BlueStacks-5-offline-installer#:~:text=To%20install%20BlueStacks%205%20Android%2011) 버전 설치를 권장합니다; 제거하려면 공식 [제거 도구](https://support.bluestacks.com/hc/en-us/articles/360057724751-How-to-uninstall-BlueStacks-5-BlueStacks-X-and-BlueStacks-Services-completely-from-your-PC)를 사용하여 잔여물을 제거하세요.\n- adb 포트 번호가 계속 불규칙하게 변경되고 시작할 때마다 다르다면 컴퓨터에 [Hyper-V](https://support.bluestacks.com/hc/en-us/articles/4415238471053-System-requirements-for-BlueStacks-5-on-Hyper-V-enabled-Windows-10-and-11)가 활성화되어 있기 때문일 수 있습니다. MAA는 이제 BlueStacks 에뮬레이터 구성 파일 내에서 포트 번호를 자동으로 읽으려고 시도할 것입니다. 이 방법이 작동하지 않거나 다중 실행/둘 이상의 에뮬레이터 커널이 설치된 경우 [연결 (TODO)](../faq.html#블루스택-에뮬레이터가-매번-시작될-때마다-포트-번호가-다릅니다-hyper-v)을 참조하여 변경하세요. Hyper-V는 관리자로 실행되므로 에뮬레이터 자동 종료, 자동 연결 감지 등 adb와 관련 없는 작업도 MAA를 관리자로 실행해야 합니다.',
    },
    {
        name: 'MuMu 에뮬레이터 12',
        link: 'https://mumu.163.com/',
        note: '완전히 호환되며, [스크린샷 향상 모드](../connection.html#mumu-스크린샷-향상-모드)에 대한 추가 지원이 있습니다. Hyper-V와 호환되는 것으로 알려져 있습니다.\n\n- "완료 시 에뮬레이터 종료" 기능이 간혹 비정상적일 수 있습니다. 이런 경우 MuMu 공식 채널에 피드백을 주세요;\n- 둘 이상의 인스턴스를 열 때는 MuMu 12 다중 실행기의 ADB 버튼을 통해 해당 인스턴스의 포트 정보를 확인하고, MAA `설정` - `연결 설정`에서 연결 주소의 포트 번호를 해당 포트로 변경해야 합니다.',
    },
    {
        name: 'LDPlayer',
        link: 'https://kr.ldplayer.net/',
        note: '완전히 호환되며, [스크린샷 향상 모드](../connection.html#ld-스크린샷-향상-모드)에 대한 추가 지원이 있습니다. Hyper-V와 호환되는 것으로 알려져 있습니다.\n\n- LDPlayer 9 설치 프로그램은 설치 과정 중 자동으로 Hyper-V를 비활성화합니다. 관련 요구 사항이 있다면 주의하세요.',
    },
    {
        name: 'Nox',
        link: 'https://kr.bignox.com/',
        note: '완전히 호환되지만 테스트가 덜 되었습니다. Hyper-V와 호환되는 것으로 알려져 있습니다.',
    },
    {
        name: 'Memu',
        link: 'https://www.memuplay.com/ko/',
        note: '완전히 호환되지만 테스트가 덜 되었습니다.',
    },
]);

const partiallySupport = shuffleArray([
{
        name: 'MuMu 에뮬레이터',
        link: 'https://www.mumuplayer.com/',
        note: 'MAA v5.1.0 이후로 지원이 중단되었으며 넷이즈는 2023-08-15에 유지보수를 중단했습니다.\n\n- 더 이상 자동 연결 감지를 지원하지 않으며, 일반 연결 구성을 사용하여 adb 경로와 연결 주소를 수동으로 구성해야 합니다;\n- Minitouch, MaaTouch와 같은 효율적인 터치 모드를 사용하려면 `설정` - `연결`에서 `ADB 강제 교체`를 실행해야 합니다;\n- "완료 시 에뮬레이터 종료" 기능을 사용하려면 관리자 권한으로 MAA를 실행해야 합니다;\n- MuMu 6의 기본 해상도는 지원되지 않으며, `1280x720`, `1920x1080` 등 16:9 비율로 변경해야 합니다;\n- MuMu 6 다중 실행은 동일한 adb 포트를 사용하므로 MuMu 6의 다중 실행을 지원할 수 없습니다.',
    },
    {
        name: 'Windows Subsystem for Android™️',
        link: 'https://learn.microsoft.com/ko-kr/windows/android/wsa/',
        note: 'MAA v5.2.0 이후로 지원이 중단되었으며 Microsoft는 2025-03-05에 중단할 예정입니다.\n\n- WSA 2204 이상 (버전 번호는 서브시스템 설정의 `정보` 페이지에 있음), 연결 구성에서 `일반 구성`을 선택하세요;\n- WSA 2203 이하 (버전 번호는 서브시스템 설정 페이지 상단에 있음), 연결 구성에서 `WSA 이전 버전`을 선택하세요;\n- 이 소프트웨어는 720p 이상 `16:9` 해상도만 잘 지원하므로, 창 크기를 수동으로 16:9 비율에 최대한 가깝게 조정해 주세요. (모니터가 16:9인 경우 `F11`을 눌러 전체 화면으로 전환할 수 있습니다);\n- 아크나이츠가 포그라운드에 있고 다른 안드로이드 앱이 동시에 포그라운드에서 실행되지 않도록 해주세요. 그렇지 않으면 게임이 일시 중지되거나 작업 인식 오류가 발생할 수 있습니다;\n- WSA의 스크린샷은 종종 어떤 이유로 하얀 화면을 캡처하여 인식 오류를 일으키므로 사용을 권장하지 않습니다.',
    },
    {
        name: 'AVD',
        link: 'https://developer.android.com/studio/run/managing-avds',
        note: '이론적으로 지원됩니다.\n\n- Android 10부터 SELinux가 `Enforcing` 모드일 때 Minitouch를 사용할 수 없으므로 다른 터치 모드로 전환하거나 SELinux를 **임시로** `Permissive` 모드로 전환하세요.\n- AVD는 디버깅을 위해 만들어졌으므로 게임용으로 설계된 다른 에뮬레이터를 사용하는 것이 더 좋습니다.',
    },
    {
        name: 'Google Play Games (개발자용)',
        link: 'https://developer.android.com/games/playgames/emulator?hl=ko-kr',
        note: '이론적으로 지원됩니다. Hyper-V가 활성화되어 있어야 하며 Google 계정에 로그인해야 합니다.\n\n- [사용자 정의 연결](../connection.html)을 사용하여 연결해야 하며 ADB 포트는 `6520`입니다.\n- Android 10 이상 버전의 SELinux 정책으로 인해 Minitouch가 제대로 작동하지 않으므로 다른 터치 모드로 전환하세요.\n- 에뮬레이터를 시작한 후 첫 번째 연결은 실패하므로 `연결 실패 후 ADB 프로세스 종료 및 재시작 시도`를 체크해야 합니다.',
    },
]);

const notSupport = shuffleArray([
    {
        name: 'Google Play Games',
        link: 'https://play.google.com/googleplaygames',
        note: '호환되지 않습니다, [Consumer Client](https://developer.android.com/games/playgames/pg-emulator#installing-game-consumer)\'의 ADB 포트를 사용할 수 없습니다.\n\n단, KR의 경우 [PlayBridge](https://github.com/ACK72/PlayBridge)를 이용해 사용이 가능하지만, 공식 지원이 아니므로 서비스가 불안정할 수 있습니다.',
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

## ✅ 완벽한 지원

<ClientOnly><div v-html="fullySupportHtml"></div></ClientOnly>

## ⚠️ 부분 지원

<ClientOnly><div v-html="partiallySupportHtml"></div></ClientOnly>

## 🚫 미지원

<ClientOnly><div v-html="notSupportHtml"></div></ClientOnly>
