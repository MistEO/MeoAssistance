---
order: 3
icon: mdi:plug
---

# 연결 설정

## ADB 경로

:::info 기술적 정보
자동 감지는 에뮬레이터의 ADB를 사용하지만, 자동 감지에 문제가 있을 경우 수동 설정이 필요합니다.
`ADB 강제 교체`는 구글이 제공하는 ADB를 다운로드한 후 교체하는 것으로, 구글 ADB를 설정하면 한 번만 설정해두면 됩니다.
:::

### 에뮬레이터에서 제공하는 ADB 사용

에뮬레이터 설치 경로로 이동합니다. Windows에서는 에뮬레이터 실행 중 작업 관리자를 열어 프로세스를 오른쪽 클릭하고 `파일 위치 열기`를 선택합니다.

상위 또는 하위 디렉토리에서 `adb`가 포함된 exe 파일을 찾을 수 있습니다. 검색 기능을 사용하여 파일을 찾고 선택하세요.

:::details 예시
`adb.exe` `HD-adb.exe` `adb_server.exe` `nox_adb.exe`
:::

### 구글이 제공하는 ADB 사용

[여기서](https://dl.google.com/android/repository/platform-tools-latest-windows.zip) 다운로드 후 압축을 풀고, `adb.exe`를 선택합니다.

MAA 폴더에 직접 압축을 푸는 것을 권장합니다. 그러면 ADB 경로에 `.\platform-tools\adb.exe`를 입력할 수 있으며, MAA 폴더와 함께 이동할 수 있습니다.

## 연결 주소

::: tip
로컬에서 실행되는 에뮬레이터의 연결 주소는 `127.0.0.1:<포트 번호>` 또는 `emulator-<네 자리 숫자>`이어야 합니다.
:::

### 포트 번호 입력

#### 에뮬레이터 관련 문서 및 참조 포트

- [Bluestacks 5](https://support.bluestacks.com/hc/zh-tw/articles/360061342631-%E5%A6%82%E4%BD%95%E5%B0%87%E6%82%A8%E7%9A%84%E6%87%89%E7%94%A8%E5%BE%9EBlueStacks-4%E8%BD%89%E7%A7%BB%E5%88%B0BlueStacks-5#%E2%80%9C2%E2%80%9D) `5555`
- [MuMu Pro](https://mumu.163.com/mac/function/20240126/40028_1134600.html) `16384`
- [MuMu 12](https://mumu.163.com/help/20230214/35047_1073151.html) `16384`
- [逍遥(Xiaoyao)](https://bbs.xyaz.cn/forum.php?mod=viewthread&tid=365537) `21503`
- [夜神(Yeshen)](https://support.yeshen.com/zh-CN/qt/ml) `62001`

기타 에뮬레이터는 [赵青青의 블로그](https://www.cnblogs.com/zhaoqingqing/p/15238464.html)를 참고하세요.

#### 멀티 인스턴스에 관하여

- MuMu 12 의 멀티 인스턴스 관리자 오른쪽 상단에서 실행 중인 인스턴스의 포트를 확인할 수 있습니다.
- Bluestacks 5 에뮬레이터 설정 내에서 현재 멀티 인스턴스 포트를 확인할 수 있습니다.
- _추가 예정_

::: details 대체 방법

- 방법 1: adb 명령어로 에뮬레이터 포트 확인

    1. **하나의 에뮬레이터**를 실행하고, 다른 안드로이드 장치가 이 컴퓨터에 연결되어 있지 않은지 확인합니다.
    2. adb 실행 파일이 있는 폴더에서 명령어 창을 엽니다.
    3. 다음 명령어를 실행합니다.

    ```sh
    # Windows 명령 프롬프트
    adb devices
    # Windows PowerShell
    .\adb devices
    ```

    다음은 출력 예시입니다:

    ```text
    List of devices attached
    127.0.0.1:<포트 번호>   device
    emulator-<네 자리 숫자>  device
    ```

    `127.0.0.1:<포트>` 또는 `emulator-<네 자리 숫자>`를 연결 주소로 사용합니다.

- 방법 2: 기존 adb 연결 찾기

    1. 방법 1을 시도합니다.
    2. `윈도우 키 + S`를 눌러 검색 창을 열고, `리소스 모니터`를 입력한 후 실행합니다.
    3. `네트워크` 탭으로 전환하고, 수신 대기 포트의 이름 열에서 에뮬레이터 프로세스명을 찾습니다. 예: `HD-Player.exe`.
    4. 에뮬레이터 프로세스의 모든 수신 대기 포트를 기록합니다.
    5. `TCP 연결`의 목록에서 `adb.exe`를 찾아, 원격 포트 열에서 에뮬레이터 수신 대기 포트와 일치하는 포트를 에뮬레이터 디버깅 포트로 사용합니다.

:::

### Bluestacks 에뮬레이터 Hyper-V 포트 번호 변경

`연결 설정`에서 `연결 프리셋`을 `Bluestacks 에뮬레이터`로 설정한 후, `연결 자동 감지`와 `매번 재감지`를 선택합니다.

일반적으로 이렇게 설정하면 연결됩니다. 연결되지 않는 경우, 여러 에뮬레이터 코어가 존재하거나 문제가 발생한 것입니다. 아래의 추가 설정을 읽어보세요.

#### `Bluestacks.Config.Keyword` 지정

::: info 주의
멀티 인스턴스 기능을 활성화했거나 여러 에뮬레이터 코어를 설치한 경우, 사용 중인 에뮬레이터 번호를 지정해야 합니다.
:::

`.\config\gui.json`에서 `Bluestacks.Config.Keyword` 필드를 검색합니다. 내용은 `"bst.instance.<에뮬레이터 번호>.status.adb_port"`입니다. 에뮬레이터 번호는 에뮬레이터 경로의 `BlueStacks_nxt\Engine`에서 확인할 수 있습니다.

::: details 예시
Nougat64 코어：

```json
"Bluestacks.Config.Keyword":"bst.instance.Nougat64.status.adb_port",
```

Pie64_2 코어：（코어 이름 뒤의 숫자는 멀티 인스턴스 코어를 나타냅니다）

```json
"Bluestacks.Config.Keyword": "bst.instance.Pie64_2.status.adb_port",
```

:::

#### `Bluestacks.Config.Path` 지정

::: info 주의
MAA는 이제 레지스트리에서 `bluestacks.conf`의 저장 위치를 읽어오려고 시도합니다. 이 기능이 작동하지 않을 경우 수동으로 구성 파일 경로를 지정해야 합니다.
:::

1. 블루스택 데이터 디렉토리에서 `bluestacks.conf` 파일을 찾습니다.

    - 글로벌 버전 `C:\ProgramData\BlueStacks_nxt\bluestacks.conf`
    - 중국 내륙 버전 `C:\ProgramData\BlueStacks_nxt_cn\bluestacks.conf`

2. 처음 사용하는 경우, MAA를 한 번 실행하여 MAA가 자동으로 구성 파일을 생성하게 합니다.

3. **MAA를 종료**한 후, `gui.json`을 열어 `Configurations` 아래의 현재 구성 이름 필드를 찾습니다(설정-구성 전환에서 확인할 수 있으며, 기본값은 `Default`입니다). 그 안에서 `Bluestacks.Config.Path` 필드를 찾아 `bluestacks.conf`의 전체 경로를 입력합니다. (슬래시를 이스케이프 `\\`해서 사용해야 합니다.)

::: details 예시
`C:\ProgramData\BlueStacks_nxt\bluestacks.conf` 경로 예시

```json
{
    "Configurations": {
        "Default": {
            "Bluestacks.Config.Path": "C:\\ProgramData\\BlueStacks_nxt\\bluestacks.conf"
            // 다른 구성 필드, 수동으로 입력하지 마세요.
        }
    }
}
```

:::

## 연결 구성

해당 에뮬레이터의 구성을 선택해야 합니다. 목록에 없으면 일반 구성을 선택하세요. 일반 구성이 작동하지 않으면 다른 사용 가능한 구성을 시도하세요.

자세한 차이점은 [소스 코드](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/dev/resource/config.json#L65)를 참조하세요.

### MuMu 스크린샷 강화 모드

중국 공식판 또는 Ark Edition MuMu 12 V4.0.0 및 이후 버전만 지원합니다. 글로벌 에디션은 현재 지원되지 않습니다. <!-- V3.8.13 -->

1. 설정 → 연결 설정에서 `MuMu 스크린샷 향상 모드 활성화`를 체크합니다. 체크할 경우 MAA는 레지스트리를 통해 설치 경로를 자동으로 입력하려고 시도합니다.

2. MuMu 에뮬레이터 경로에 `MuMuPlayer-12.0` 또는 `YXArkNights-12.0` 폴더의 경로를 입력합니다. 예: `C:\Program Files\Netease\MuMuPlayer-12.0`.

3. MuMu 네트워크 브리징을 사용 중인 경우 `MuMu 네트워크 브리징 모드`를 선택한 후 MuMu 멀티 인스턴스 관리자에서 해당 에뮬레이터의 일련 번호를 수동으로 입력해야 합니다. 예를 들어, 주 인스턴스는 `0`입니다.

### LD 스크린샷 향상 모드

현재 중국판 LD Player 9 V9.1.32 이상 버전만 지원됩니다. 명일방주 에디션은 현재 지원되지 않습니다. <!-- V9.0.78 -->

1. 설정 → 연결 설정에서 `LD의 스크린샷 향상 모드 활성화`를 켭니다. 체크할 경우 MAA는 레지스트리를 통해 설치 경로를 자동으로 입력하려고 시도합니다.

2. `LD 에뮬레이터 실행 경로`에 `LDPlayer9` 폴더의 경로를 입력합니다. 예: `C:\leidian\LDPlayer9\`

3. `인스턴스 번호`에 LD 멀티플레이어에서 해당 에뮬레이터의 일련번호(ID)를 입력합니다. 예: 메인 멀티플레이어의 경우 `0`

## 터치 모드

1. [Minitouch](https://github.com/DeviceFarmer/minitouch): C로 작성된 Android 터치 이벤트 핸들러로, 외부 프로그램이 터치 이벤트와 제스처를 트리거할 수 있는 소켓 인터페이스를 제공합니다. Android 10부터는 SELinux가 `Enforcing` 모드일 때 Minitouch가 더 이상 사용되지 않습니다.
2. [MaaTouch](https://github.com/MaaAssistantArknights/MaaTouch): MAA가 Java 기반으로 Minitouch를 재구현한 것입니다. 높은 버전의 Android에서도 사용 가능성이 테스트 중입니다.
3. Adb Input: ADB 명령어를 직접 호출하여 터치 작업을 수행하며, 호환성이 가장 좋지만, 속도는 가장 느립니다.

## ADB Lite

MAA가 독립적으로 구현한 ADB 클라이언트로, 원본 ADB와 비교했을 때 여러 ADB 프로세스를 계속해서 실행하지 않아도 되지만, 일부 스크린샷 방식은 사용할 수 없습니다.

활성화하는 것을 권장하지만, 구체적인 장단점은 피드백이 필요합니다. ~~테스트를 도와주세요~~

## MAA와 에뮬레이터의 멀티 인스턴스

::: info
멀티 인스터스를 사용해 여러 에뮬레이터를 동시에 작동해야 하는 경우, MAA 폴더를 여러 번 복사한 후 **다른 MAA**, **같은 adb.exe**, **다른 연결 주소**를 사용하여 연결할 수 있습니다.
:::

### 멀티 인스턴스 에뮬레이터의 자동 시작

[BlueStacks의 글로벌 버전](./device/windows.md)을 예로 들어, 여러 에뮬레이터를 시작하는 두 가지 방법을 소개합니다.

#### 추가 명령어를 통해 시작

1. 하나의 에뮬레이터를
2. 작업 관리자를 열고, 해당 에뮬레이터 프로세스를 찾아 `세부 정보` 탭으로 이동한 후, 상단의 열을 우클릭하고 `열 선택`을 클릭하여 `명령줄`을 체크합니다
3. 새로 추가된 `명령줄` 열에서 `"...\Bluestacks_nxt\HD-Player.exe"` 이후의 내용을 찾습니다.
4. 찾은 내용을 `--instance Nougat32`와 같이 `시작 설정` - `추가 명령어`에 입력합니다.

::: note 참고
작업이 완료된 후, `2단계`에서 열었던 `명령줄` 열을 숨겨 프리징을 방지하는 것이 좋습니다.
:::

::: details 예시

```text
멀티 인스턴스 1:
에뮬레이터 경로: C:\Program Files\BlueStacks_nxt\HD-Player.exe
추가 명령어: --instance Nougat32 --cmd launchApp --package "com.hypergryph.arknights"
멀티 인스턴스 2:
에뮬레이터 경로: C:\Program Files\BlueStacks_nxt\HD-Player.exe
추가 명령어: --instance Nougat32_1 --cmd launchApp --package "com.hypergryph.arknights.bilibili"
```

`--cmd launchApp --package` 부분은 시작 후 지정된 패키지 이름의 애플리케이션을 자동으로 실행하며, 필요에 따라 변경할 수 있습니다.
:::

### 에뮬레이터의 바로가기를 통해 시작

일부 에뮬레이터는 앱 바로가기를 생성할 수 있으며, 이를 통해 에뮬레이터를 직접 시작하고 앱 바로가기로 명일방주를 실행할 수 있습니다.

1. 멀티 인스턴스 관리자를 열고 해당 에뮬레이터의 바로가기를 추가합니다.
2. `시작 설정` - `에뮬레이터 경로`에 에뮬레이터 바로가기의 경로를 입력합니다.

::: details 예시

```text
멀티 인스턴스 1:
에뮬레이터 경로: C:\ProgramData\Microsoft\Windows\Start Menu\Programs\BlueStacks\Multi-instance 1.lnk
멀티 인스턴스 2:
에뮬레이터 경로: C:\ProgramData\Microsoft\Windows\Start Menu\Programs\BlueStacks\Multi-instance 2 - Arknights.lnk
```

:::

멀티 인스턴스 작업에 `에뮬레이터 경로`를 사용할 경우, `시작 설정`의 `추가 명령어`는 비워 두는 것이 좋습니다.
