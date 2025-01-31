---
order: 4
icon: ph:question-fill
---

# 자주 묻는 질문 (FAQ)

MAA를 처음 사용하는 경우 [초보자 가이드](./newbie.md)를 읽어보세요.

::: warning

MAA가 업데이트 후 실행되지 않거나 MAA의 오류 창을 통해 여기에 도달했다면, 이는 대부분 실행 라이브러리가 업데이트되지 않은 문제 때문입니다.  
가장 자주 발생하는 문제는 실행 라이브러리 문제이며, 많은 사람들이 문서를 읽지 않고 질문만 하기 때문에 공지 내용을 이걸로 교체했습니다.

MAA 디렉토리에서 `DependencySetup_依赖库安装.bat`를 실행하거나, 터미널에서 아래 명령을 실행하거나,

```sh
winget install "Microsoft.VCRedist.2015+.x64" --override "/repair /passive /norestart" --force --uninstall-previous --accept-package-agreements && winget install "Microsoft.DotNet.DesktopRuntime.8" --override "/repair /passive /norestart" --force --uninstall-previous --accept-package-agreements
```

아래<u>**두 개**</u>의 실행 라이브러리를 수동으로 다운로드하여 설치하여 문제를 해결하세요.

- [Visual C++ 재배포 가능 패키지](https://aka.ms/vs/17/release/vc_redist.x64.exe)
- [.NET 데스크톱 런타임 8](https://aka.ms/dotnet/8.0/windowsdesktop-runtime-win-x64.exe)

:::

## 소프트웨어 실행 불가능/크래쉬/오류 발생

### 다운로드/설치 문제

- 완전한 MAA 소프트웨어 패키지의 이름 형식은 "MAA-`버전`-`플랫폼`-`아키텍처`.zip"입니다. 다른 것들은 단독으로 사용할 수 없는 "구성 요소"이므로 주의 깊게 읽어보세요.
  대부분의 경우, x64 아키텍처의 MAA를 사용해야 합니다. 즉, `MAA-*-win-x64.zip`을 다운로드해야 하며, `MAA-*-win-arm64.zip`이 아닙니다.
- 자동 업데이트 후 기능이 누락되었거나 작동하지 않는 경우, 업데이트 과정에서 문제가 발생했을 수 있습니다. 전체 설치 패키지를 다시 다운로드하고 압축을 해제하세요. 압축 해제 후, 이전 `MAA` 폴더에 있는 `config` 폴더를 새로 압축 해제된 `MAA` 폴더로 직접 드래그하세요.

### 런타임 문제

웹 페이지 오른쪽 아래에 있는 위로 ↑ 화살표를 찾아 클릭하세요.

#### Windows N/KN 관련

Windows 8/8.1/10/11 N/KN(유럽/한국) 버전을 사용하는 경우, [미디어 기능 팩](https://support.microsoft.com/ko-kr/topic/c1c6fffa-d052-8338-7a79-a4bb980a700a)을 설치해야 합니다.

#### Windows 7 관련

.NET 8은 Windows 7 / 8 / 8.1 시스템을 지원하지 않으므로<sup>[출처](https://github.com/dotnet/core/issues/7556)</sup>, MAA도 더 이상 지원하지 않습니다. 마지막으로 사용 가능한 .NET 8 버전은 [`v5.4.0-beta.1.d035.gd2e5001e7`](https://github.com/MaaAssistantArknights/MaaRelease/releases/tag/v5.4.0-beta.1.d035.gd2e5001e7)입니다. 마지막으로 사용 가능한 .NET 4.8 버전은 [`v4.28.8`](https://github.com/MaaAssistantArknights/MaaAssistantArknights/releases/tag/v4.28.8)입니다. 자체 컴파일의 실현 가능성은 아직 확인되지 않았습니다.

Windows 7의 경우, 위에서 언급한 두 개의 런타임 라이브러리를 설치하기 전에 다음 패치가 설치되어 있는지 확인해야 합니다:

1. [Windows 7 Service Pack 1](https://support.microsoft.com/zh-cn/windows/b3da2c0f-cdb6-0572-8596-bab972897f61)
2. SHA-2 코드 서명 패치:
    - KB4474419：[다운로드 링크 1](https://catalog.s.download.windowsupdate.com/c/msdownload/update/software/secu/2019/09/windows6.1-kb4474419-v3-x64_b5614c6cea5cb4e198717789633dca16308ef79c.msu)、[다운로드 링크 2](http://download.windowsupdate.com/c/msdownload/update/software/secu/2019/09/windows6.1-kb4474419-v3-x64_b5614c6cea5cb4e198717789633dca16308ef79c.msu)
    - KB4490628：[다운로드 링크 1](https://catalog.s.download.windowsupdate.com/c/msdownload/update/software/secu/2019/03/windows6.1-kb4490628-x64_d3de52d6987f7c8bdc2c015dca69eac96047c76e.msu)、[다운로드 링크 2](http://download.windowsupdate.com/c/msdownload/update/software/secu/2019/03/windows6.1-kb4490628-x64_d3de52d6987f7c8bdc2c015dca69eac96047c76e.msu)
3. Platform Update for Windows 7（DXGI 1.2、Direct3D 11.1,KB2670838）：[다운로드 링크 1](https://catalog.s.download.windowsupdate.com/msdownload/update/software/ftpk/2013/02/windows6.1-kb2670838-x64_9f667ff60e80b64cbed2774681302baeaf0fc6a6.msu)、[다운로드 링크 2](http://download.windowsupdate.com/msdownload/update/software/ftpk/2013/02/windows6.1-kb2670838-x64_9f667ff60e80b64cbed2774681302baeaf0fc6a6.msu)

##### .NET 8 애플리케이션이 Windows 7에서 비정상적으로 실행되는 문제 완화 조치 [#8238](https://github.com/MaaAssistantArknights/MaaAssistantArknights/issues/8238)

Windows 7에서 .NET 8 애플리케이션을 실행할 때 메모리 사용량이 비정상적으로 증가하는 문제가 있습니다. 아래의 완화 조치를 참조하십시오. Windows 8/8.1은 테스트되지 않았습니다. 동일한 문제가 있는 경우, 문서를 업데이트할 수 있도록 Issue를 제출해 주십시오.

1. `컴퓨터`를 열고 빈 공간을 마우스 오른쪽 버튼으로 클릭하여 속성을 클릭하고, 왼쪽의 `고급 시스템 설정`을 클릭한 다음 `환경 변수`를 클릭합니다.
2. 새 시스템 변수를 만들고, 변수 이름을 `DOTNET_EnableWriteXorExecute`, 변수 값을 `0`으로 설정합니다.
3. 컴퓨터를 재시작합니다.

#### 공식 통합 패키지 (확실함)

::: info 참고
이 작업은 약 10GB 정도의 디스크 공간을 소비하므로 다른 방법들을 모두 시도한 후에 마지막으로 시도하세요.
:::

[Microsoft C++ 빌드도구](https://visualstudio.microsoft.com/zh-hans/visual-cpp-build-tools/)를 설치하여 완전한 개발 환경을 설정하세요 (닷넷 및 C++ 개발 환경만 설치하면 됩니다).

### 가능성 3: 시스템 구성 요소 문제

상기된 런타임 설치는 구성 요소 저장소(CBS, TrustedInstaller/TiWorker, WinSxS)에 의존합니다. 구성 요소 저장소가 손상되면 정상적으로 설치할 수 없습니다.

운영 체제를 다시 설치하는 것 이외에 다른 수정 방법을 제공할 수 없으며, 확장 기능이나 리스크가 표시되지 않은 "정리판" 운영 체제를 사용하는 것을 피하세요.

## 연결 오류

::: tip
사용 중인 에뮬레이터가 지원하는지, 확인하려면 다음 문서를 참조하세요. [Windows 지원 목록](./device/windows.md)
:::

### adb 및 연결 주소 확인

- MAA의 `설정` - `연결 설정` - `adb 경로`가 채워졌는지 확인하세요. 이미 채워져 있다면 이 단계를 건너뛰세요.

::: details 미 입력시

- 에뮬레이터의 설치 경로를 찾으세요. Windows에서는 작업 관리자에서 에뮬레이터를 실행한 후 프로세스를 마우스 오른쪽 클릭하여 `파일 위치 열기`를 선택할 수 있습니다.
- 최상위 또는 하위 폴더에는 일반적으로 adb.exe가 있을 것입니다. 해당 파일을 선택하세요! (이름이 정확히 일치하지 않을 수 있습니다. 예를 들어 `nox_adb.exe`, `HD-adb.exe`, `adb_server.exe` 등이 있을 수 있습니다. 어쨌든 adb가 들어간 이름의 exe 파일입니다)
:::

- 연결 주소가 올바르게 입력되었는지 확인하세요. 사용 중인 에뮬레이터의 adb 디버깅 주소를 인터넷에서 검색하세요. 일반적으로 127.0.0.1:5555와 같은 형식입니다. (단, LDPlayer는 예외입니다)

#### 에뮬레이터의 연결 포트

각 에뮬레이터의 포트들은 다음과 같습니다:

::: details 연결 포트

| 에뮬레이터 |     포트 기본값      |
| ---------- | :------------------: |
| MuMu 6/X   |         7555         |
| MuMu 12    |        16384         |
| NoxPlayer  |        62001         |
| Memu       |        21503         |
| BlueStacks |         5555         |
| LDPlayer 9 | 5555 / emulator-5554 |

:::

이 외의 에뮬레이터를 사용할 경우에는, 다음 블로그를 참조하세요. [Zhao Qingqing의 블로그](https://www.cnblogs.com/zhaoqingqing/p/15238464.html)。

### 기존 adb 프로세스 종료

MAA를 종료한 후 작업 관리자에서 `세부 정보`에서 `adb`라는 이름을 포함한 프로세스가 있는지 확인하세요.있다면 종료한 후 다시 연결을 시도하세요.
 (일반적으로 앞서 입력한 adb 파일과 동일한 이름을 갖습니다)

### 여러 adb를 올바르게 사용하기

adb 버전이 다를 경우, 새로 시작된 프로세스가 이전 프로세스를 종료합니다. 따라서 Android Studio, Alas, 휴대폰 도우미와 같은 여러 개의 adb를 동시에 실행할 때 버전이 같은지 확인하세요.

### 게임 가속기 피하기

일부 가속기는 가속을 시작하거나 중지한 후 MAA, ADB 및 에뮬레이터를 다시 연결해야 합니다.

### 컴퓨터 다시 시작

재부팅은 97%의 문제를 해결합니다. (확신합니다)

### 에뮬레이터 변경

[Windows 지원 목록](./device/windows.md)을 참고하세요.

## 연결은 정상이지만, 조작이 안됨

일부 에뮬레이터들의 기본 `adb` 버전이 너무 오래되어 `Minitouch` 관련 작업을 지원하지 않습니다.

관리자 권한으로 MAA를 실행하고, `MAA 설정` - `연결 설정` - `강제 ADB 교체`를 클릭하세요. (에뮬레이터를 닫고 MAA를 다시 시작한 후 작업을 권장합니다. 그렇지 않으면 실패할 수 있습니다)

에뮬레이터를 업데이트하면 adb 파일이 다시 덮어씌워질 수 있습니다. 업데이트 후 문제가 그대로라면 다시 시도하세요.

이러한 조치로도 문제가 해결되지 않는 경우, `연결 설정` - `터치 모드`를 `Minitouch`에서 `MaaTouch`로 변경하여 다시 시도하세요. `Adb Input` 작업은 최후의 수단으로 사용해야 합니다.

## 블루스택 에뮬레이터가 매번 시작될 때마다 포트 번호가 다릅니다. (Hyper-V)

MAA를 열고 `설정` - `연결 설정`에서 `연결 구성`을 `블루스택`으로 설정한 후 `자동 연결 검색`과 `매번 다시 검색`을 선택하세요. (또는 메인 화면의 시작 버튼 옆의 설정에서 이 두 항목을 선택하세요)

일반적으로 이렇게 하면 연결됩니다. 연결되지 않으면 여러 에뮬레이터 코어가 있는지 또는 문제가 있는지 확인하려면 아래 추가 설정을 참조하세요.

### `Bluestacks.Config.Keyword` 지정

::: info 참고
멀티 코어 기능을 사용하거나 여러 에뮬레이터 코어를 설치한 경우 사용할 에뮬레이터의 번호를 지정해야 합니다.
:::

`.\config\gui.json`에서 `Bluestacks.Config.Keyword` 필드를 검색하고 내용을 `"bst.instance.<에뮬레이터 번호>.status.adb_port"`로 설정합니다. 에뮬레이터 번호는 에뮬레이터 경로인 `BlueStacks_nxt\Engine`에서 확인할 수 있습니다.

::: details 예시
Nougat64 코어:

```json
"Bluestacks.Config.Keyword":"bst.instance.Nougat64.status.adb_port",
```

Pie64_2 코어: (코어 이름 뒤의 숫자는 멀티 오픈 코어를 나타냅니다)

```json
"Bluestacks.Config.Keyword": "bst.instance.Pie64_2.status.adb_port",
```

:::

### `Bluestacks.Config.Path` 지정

::: info 참고
MAA는 이제 레지스트리에서 `bluestacks.conf`의 위치를 읽으려고 시도합니다. 이 기능이 작동하지 않으면 설정 파일 경로를 수동으로 지정해야 합니다.
:::

1. 블루스택 시뮬레이터 데이터 디렉터리에서 `bluestacks.conf` 파일을 찾으세요.

   - 국제 버전의 기본 경로는 `C:\ProgramData\BlueStacks_nxt\bluestacks.conf`입니다.
   - 중국 본토 버전의 기본 경로는 `C:\ProgramData\BlueStacks_nxt_cn\bluestacks.conf`입니다.

2. 처음 사용하는 경우 MAA를 실행하여 MAA가 설정 파일을 자동으로 생성하도록 하세요.

3. **먼저** MAA를 닫고 **그 다음에** `gui.json`을 열고 현재 설정 이름(설정-설정 변경에서 확인할 수 있습니다. 기본적으로 Default) 아래의 `Configurations`에서 `Bluestacks.Config.Path` 필드를 검색하여 `bluestacks.conf` 파일의 전체 경로를 입력하세요. (슬래시는 `\\`로 이스케이프해야 합니다.)

::: details 예시
`C:\ProgramData\BlueStacks_nxt\bluestacks.conf`를 기준으로합니다.

```json
{
  "Configurations": {
    "Default": {
      "Bluestacks.Config.Path": "C:\\ProgramData\\BlueStacks_nxt\\bluestacks.conf"
      // 다른 설정 필드, 수동으로 입력하지 마세요.
    }
  }
}
```

:::

## 연결은 정상이지만 작업이 버벅거리거나 이상 또는 빈번한 오류가 발생합니다

- `UI 위치 조절`을 사용하는 경우 0으로 설정하세요.
- 국가 별 클라이언트를 플레이하는 경우 먼저 `설정` - `게임 설정` - `클라이언트 유형`에서 `클라이언트 버전`을 선택하세요. 해외 클라이언트의 경우 일부 기능이 완전히 호환되지 않을 수 있으므로 해당 해외 클라이언트 사용 설명서를 참조하세요.
- 통합전략 기능을 사용할 경우, [사용자 설명서](./introduction/integrated-strategy.md)를 참조하고 `통합전략` - `테마`에서 올바른 테마를 선택하세요
- `Adb Input` 터치 모드 작업이 느린 것은 정상입니다. Copilot 기능이 필요한 경우 다른 모드로 전환해 보세요.

### 스크린샷 촬영이 오래 걸리거나 오랜 시간이 걸립니다

- MAA는 현재 `RawByNc`, `RawWithGzip`, `Encode` 세 가지 방법으로 스크린샷을 지원하며 평균 스크린샷 시간이 400/800보다 클 때 한 번의 메시지를 출력합니다. (한 번의 작업에서 한 번만 메시지를 출력합니다)
- `설정` - `연결 설정`에서 최근 30회 스크린샷 시간의 최소/평균/최대값을 표시하며 10회마다 스크린샷이 새로 고침됩니다.
- Copilot(자동 전투) 기능은 스크린샷 시간에 크게 영향을 받습니다.
- 이 시간은 MAA와 관련이 없으며 컴퓨터 성능, 현재 사용률 또는 시뮬레이터에 따라 달라집니다. 백그라운드를 정리하거나 시뮬레이터를 변경하거나 컴퓨터 구성을 업그레이드하는 등의 조치를 취해보세요.

## 관리자 권한 관련 문제

MAA는 Windows UAC 관리자 권한 없이 모든 기능을 실행할 수 있어야 합니다. 현재 관리자 권한과 관련된 기능은 다음과 같습니다:

1. `자동 연결 감지`：목표 에뮬레이터가 관리자 권한으로 실행될 때 관리자 권한이 필요합니다.
2. `작업 완료 후 에뮬레이터 종료`：목표 에뮬레이터가 관리자 권한으로 실행될 때 관리자 권한이 필요합니다.
3. `부팅 시 MAA 자동 시작`：관리자 권한으로 실행할 경우 부팅 시 자동 시작을 설정할 수 없습니다.
4. MAA가 관리자 권한이 필요한 경로에 잘못 압축된 경우, 예: `C:\` 또는 `C:\Program Files\`와 같은 경로.

UAC가 비활성화된 시스템에서는 "우클릭하여 관리자 권한으로 실행을 선택하지 않아도 관리자 권한으로 실행되는" 문제가 발생할 수 있다는 보고가 있습니다. 예상치 못한 권한 상승을 방지하기 위해 UAC를 활성화하는 것이 좋습니다.

## 파일 다운로드 속도가 느립니다

[MAA 다운로드 사이트](https://ota.maa.plus/MaaAssistantArknights/MaaRelease/releases/download/)에서 다운로드 하세요.

우리의 서버는 대역폭이 낮고 트래픽이 제한됩니다. 사용하기 위해 만들어졌지만 너무 남용하지 마세요. GitHub 또는 다른 미러 사이트에서 정상적으로 다운로드할 수 있는 경우 거기에서 먼저 시도해보세요.

## 중간에 다운로드 중 "로그인"/"인증" 메시지가 표시됩니다

브라우저 / IDM / FDM 등의 다운로드 관리자를 사용하여 파일을 다운로드하세요. **위쳐를 사용하지 마세요!**
