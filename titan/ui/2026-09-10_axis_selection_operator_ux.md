# 축 선택 화면(kadex_lobby) 운용자 친화 개편 — 코드 측

2026-09-10 / 진행중(C++ 완료, 빌드·WBP 작업 대기) / 엔터 커밋 함정 제거 + 입력값 ini 저장 + "이 PC의 IP" 표시 + 툴팁 도움말 + 실행 전 점검. 목표는 **처음 보는 사람이 설명서 없이 실행할 수 있게** 하는 것.

관련 문서: `packaging/kadex_0902_패키징_실행가이드.md`(§4가 이 화면을 설명), `level_new_kadex_0811/2026-09-01_scenario_run_modes_demo_fullsystem.md` §3.5(Host/Client/Solo/데모 체크박스 도입), `infra_architecture/architecture_decisions.md` §1.2(토폴로지), `ui/ingame_settings_input_system.md`(Settings 화면).

---

## 1. 목표

패키지를 실행하면 `kadex_lobby`의 축 선택 화면(`WBP_AxisSelection2`, 부모 `UAxisSelectionWidget`)이
먼저 뜬다. **캔버스 한 장에 EditableText 18개 + 버튼 4개 + 체크박스 1개**가 평면 나열돼 있고,
무엇을 눌러야 하는지는 화면이 아니라 실행가이드 §4가 설명하고 있었다.

이 화면을 만지는 사람은 **LIG 측 또는 우리 인력**(전시 전 세팅 담당)이고 관람객이 아니다.
그래서 목표는 기능을 숨기는 게 아니라 **화면 자체가 설명서 역할을 하게** 만드는 것이다.

---

## 2. ⚠️ 먼저 — Solo와 Demo는 별개 개념이다

**2026-09-10에 이 문서의 초판이 둘을 엮어서 서술했다가 사용자 지적으로 정정했다.** 다시 헷갈리지
않도록 근거와 함께 남긴다.

| | 무엇인가 | 무엇이 정하나 |
|---|---|---|
| **Solo** | 호스트/클라이언트 없이 이 PC 하나로 여는 것 | 맵 URL의 `?Listen` 유무 |
| **Demo / FullSystem** | 조준·사격을 **누가 쥐는가** | `AScenarioConfig::RunMode` / `?Demo=` / `-demo` |

- **Solo가 생긴 진짜 이유는 전시 데모가 아니라 축 선택이다.** `New_kadex_0811`의 GameMode가
  `DefaultAxisWhenUnspecified=UGV`라서, 자체방호축으로 들어가려면 원래 **다른 PC에서 Host를
  띄우거나** 콘솔에 `open New_kadex_0811?Axis=SelfDefense`를 쳐야만 했다. 그걸 버튼으로 옮긴 게
  `SoloButton`의 시초다.
  - 문서에도 흔적이 있다 — `2026-09-01_scenario_run_modes_demo_fullsystem.md` §6-2:
    "그냥 PIE로 New_kadex_0811을 직접 열면 여전히 UGV축이라 알림이 안 뜬다… 알림까지 보려면
    대기실 → '호스트 없이 시작'으로 가거나, 콘솔에 `StartSoloAxis SelfDefense 1`."
- **Demo는 시나리오 시스템이 RCWS까지 전부 쥐는 모드**다(자동사격 ARM + 자동 시작 +
  `UUGVRemoteControlSubsystem` 소켓 차단). FullSystem은 UGV가 LIG 원격통제기와, 자체방호가
  상위체계와 붙어야 시나리오대로 돈다.
- 둘이 한 덩어리로 읽히기 쉬운 이유: `StartSoloAxis(Axis, bDemoMode = true)`의 **기본값이
  true**다. 그건 "단독 실행은 보통 통제기가 없더라"는 편의 기본값일 뿐이다.

### 관련해서 코드로 확인한 사실 3가지 (2026-09-10)

1. **`architecture_decisions.md` §1.2가 이미 말하고 있다** — *"리슨서버는 클라이언트가 안 붙어도
   싱글플레이처럼 동작해서 한쪽 PC만 켜고 솔로 개발/테스트도 자연스럽게 됨."*
   → **설계상 "혼자 쓰기" = Host로 띄우고 아무도 안 붙이기.**
2. **반대로 `StartSoloAxis` 경로에는 클라이언트가 붙을 수 없다.** `UnrealEngine.cpp:16564`가
   `URL.HasOption(TEXT("Listen"))`일 때만 `World->Listen()`을 부른다 — `?Listen`이 없으면 net
   driver 자체가 안 생긴다. 누가 막은 게 아니라 그냥 그렇게 된다.
3. **자체방호 ↔ 상위체계 연동은 현재 코드에 없다.** 프로젝트 전체 소켓이
   `UGVRemoteControlSubsystem`의 UDP 2개뿐이다(grep 확인). Layer A(상위체계↔UGV)는
   `Network/UGVRemoteControlTypes.h:86-88`에 "LIG도 아직 미확정 / UGV SW는 구조상 원격통제기를
   거쳐야만 상위체계와 연결 가능"으로 남아 있다. 지금 구현에서 자체방호 PC가 외부와 주고받는 건
   **RTSP 송출뿐**이다.

### 코드에 있는 하드 제약은 하나뿐

```cpp
// UUGVRemoteControlSubsystem::ShouldBeActive()
NetMode != NM_Client  &&  PlayerAxis == UGV  &&  !IsDemoMode()
```

→ **통제기 연동이 필요하면 UGV축 프로세스가 서버여야 한다**(Host든 standalone이든).
`StartSoloAxis` 헤더의 "자체방호 Host 금지" ⚠️도 실은 *"자체방호가 호스트면 UGV PC가 클라가
되고, 그러면 통제기가 죽는다"*는 뜻이다. 자체방호가 호스트로 뜨는 것 자체가 금지는 아니다.

---

## 3. 화면 방향 (2026-09-10 확정)

**실제로 쓰는 경로는 둘뿐이다** — UGV 시뮬레이터(Host)와 이동형지휘소(Client). 이 둘을 각각
**필요한 입력 1칸 + 시작 버튼 1개를 가진 완결된 카드**로 만들고, 나머지는 전부 **고급 실행
옵션**으로 접는다.

```
┌──────────────────────────────────────────────────────────────┐
│ ① UGV 시뮬레이터              │ ② 이동형지휘소                │
│   통제기와 연동, 영상 송출     │   UGV PC에 접속               │
│   보통 이쪽을 먼저 실행        │   ①이 먼저 떠 있어야 함        │
│                               │                               │
│   통제기 IP [192.168.10.20]ⓘ  │   UGV PC IP [           ]ⓘ    │
│                               │                               │
│   이 PC의 IP  192.168.10.11   │                               │
│   통제기·이동형지휘소에         │                               │
│   이 주소를 알려주세요          │                               │
│                               │                               │
│   [ UGV 시뮬레이터 시작 ]      │   [ 이동형지휘소 시작 ]        │
├──────────────────────────────────────────────────────────────┤
│ ▸ 고급 실행 옵션                                              │
│    [ ] 데모 모드 ⓘ    [ 호스트 없이 이동형지휘소 실행 ] ⓘ      │
│    포트 4칸 · RTSP 해상도 12칸 · [ 실행 전 점검 ]              │
├──────────────────────────────────────────────────────────────┤
│ 요약 / 상태줄                          [설정]        [종료]    │
└──────────────────────────────────────────────────────────────┘
```

지금의 `WBP_AxisSelection2`와 구조가 비슷해지는 게 맞다 — **바뀌는 건 배치가 아니라 "화면이
말을 하는가"** 다. 필수 입력이 어느 것인지, 이 PC의 IP가 무엇인지, 각 칸이 무슨 뜻인지.

> 한때 이 자리에 "프리셋 카드 3~4장 + 시작 버튼 1개"를 넣었다가 걷어냈다. 실제 운용 경로가
> 둘뿐인데 나머지를 카드로 승격시키니 화면만 복잡해졌다.

---

## 4. 구현한 것 (C++만, `.uasset` 무변경)

### 4.1 엔터 커밋 함정 제거 — **WBP 변경 없이 즉시 효과**

`HandleHostClicked` / `HandleClientClicked` / `HandleSoloClicked` 진입부에서
`CommitAllInputFields()`를 호출한다. 화면의 모든 텍스트 필드를 그 자리에서 다시 읽으므로,
타이핑만 하고 버튼을 눌러도 그 값으로 시작된다.

**2026-09-10 실측 확인**: Host를 누르면 `ApplyNetworkFieldsToSubsystem` 로그가 2줄 나온다 —
버튼 클릭으로 포커스가 빠지며 발생한 커밋 1 + 이 강제 커밋 1.

> 참고: 버튼이 포커스를 가져가면 예전 코드에서도 커밋이 먼저 일어났다(Slate는 마우스 다운에
> 포커스를 옮기고 `OnClicked`는 마우스 업). 이 변경의 의미는 **그 포커스 동작에 대한 의존을
> 없앤 것**이다 — 버튼 `Is Focusable`을 끄거나 입력 경로가 달라져도 값이 들어간다.

### 4.2 입력값 ini 저장 — 역시 WBP 변경 없음

`ApplyNetworkFieldsToSubsystem` / `ApplyResolutionFieldsToSubsystem` 끝에서 `SaveConfig()`.
두 서브시스템 다 `UCLASS(Config = Game)`인데 **`SaveConfig()` 호출이 프로젝트 전체에 0건**이었다
(grep 확인). 저장 위치는 `Saved/Config/<Platform>/Game.ini` —
`UGVRemoteControlSubsystem.h` 2026-08-15 주석이 "손으로 오버라이드하라"고 안내하던 그 파일이다.

### 4.3 "이 PC의 IP" — 어댑터 열거가 아니라 라우팅 테이블에 묻는다

UDP 소켓을 만들어 상대 IP로 `Connect()`하고(**UDP라 패킷은 한 바이트도 안 나간다**)
`FSocket::GetAddress()`를 읽는다(내부는 `getsockname`, `SocketsBSD.cpp:404`). OS가 그 목적지로
나갈 때 실제로 쓸 출발지 IP가 그대로 나오므로 NIC이 몇 개든 **커널이 답한다.**
윈도우/리눅스 동일 코드다(`FSocketSubsystemWindows`도 BSD 계열).

기준 목적지는 **통제기 IP → 실패하면 접속 서버 IP** 순으로 시도한다. 두 카드가 동시에 화면에
있어서 "지금 고른 것"으로 정할 수가 없고, UGV PC에서는 앞쪽이 / 이동형지휘소 PC에서는 뒤쪽이
실제로 채워져 있기 때문이다.

#### 왜 `GetLocalAdapterAddresses()`가 주 수단이면 안 되나 (엔진 5.8 소스 확인)

| | Windows (`SocketSubsystemWindows.cpp:135`) | Linux (`SocketSubsystemUnix.cpp:110`) |
|---|---|---|
| 방식 | `GetAdaptersAddresses()` | `getifaddrs()` |
| 필터 | 이더넷/무선 + Up + DNS-eligible만 | `IFF_UP && !IFF_LOOPBACK` 전부 |
| Hyper-V / WSL / VMware vEthernet | **나옴** | — |
| docker0 / virbr0 | — | **나옴** |
| **Tailscale / VPN (TUN)** | **안 나옴**(`IF_TYPE_TUNNEL`) | **나옴** |
| IPv6 | 포함(`fe80::` 섞임) | 포함 |

**플랫폼별로 결과가 다르고**, 특히 윈도우에서 Tailscale 주소가 안 나오는 건 이 프로젝트에 그대로
걸린다. 그래서 어댑터 열거는 **폴백**으로만 쓴다 — IPv4만 남기고 `127.` / `169.254.`를 버린 뒤
같은 `/24`를 맨 위로 올려 후보 목록으로 보여준다. 라우팅으로 답이 나온 경우에도 나머지 후보를
`LocalIPDetailText`에 같이 적는다("왜 172.17.0.1이 뜨지?"를 화면에서 바로 판단하게).

`-multihome`이 지정돼 있으면 엔진이 그 주소만 쓰므로 라우팅보다 먼저 그 값을 쓴다.

> ⚠️ **라우팅만 본다 — 방화벽은 못 본다.** ufw 8000/8001/8554(실행가이드 §1-4)는 여전히 별개다.
> 코드 주석과 화면 문구 양쪽에 이 경고를 박아뒀다.

### 4.4 툴팁 도움말 — 코드가 건다

`ApplyHelpTooltips()`가 `NativeConstruct`에서 바인딩된 모든 입력칸/버튼에
`UWidget::SetToolTipText`로 도움말을 심는다.

- WBP에서 18개 칸에 일일이 타이핑할 필요가 없다.
- **문구가 "그 값이 무슨 뜻인지 아는 곳"에 산다** — 포트 기본값이나 계약이 바뀌면 같이 눈에 든다.
- 로비는 이미 마우스 커서가 떠 있다(`titan_examplePlayerController::BeginPlay`의 `bShowMouseCursor`).
- 끄고 싶으면 클래스 디폴트의 `bApplyBuiltInTooltips = false`.
- 눈에 보이는 "ⓘ" 아이콘을 원하면 WBP에 Image를 놓고 같은 문구를 툴팁으로 넣으면 된다.

특히 `SoloButton`과 `DemoModeCheckBox` 툴팁에는 **§2의 구분("네트워크 구성" vs "누가 조준·사격을
쥐는가")을 명시**했다. 이 화면에서 가장 오해하기 쉬운 지점이라서다.

### 4.5 시작 전 검증

`ValidateForLaunch(Action, OutError)`. **확실히 실패할 입력만 막는다** — 포트 1~65535 밖,
수신 포트 2개 충돌(둘째 바인드가 반드시 실패하는데 `StartSockets`는 경고만 남기고 조용히
비활성으로 남는다), Client인데 접속 IP가 비었거나 공백 포함.

RC IP가 IPv4 리터럴이 아닌 경우는 **막지 않는다**(호스트명일 수 있다). 대신 IP 표시가
"추정값"이라고 알려준다.

### 4.6 실행 전 점검 버튼

`PreflightButton` → `StatusText`에 여러 줄 보고: 이 PC의 IP + 근거 / **입력된 상대 주소 전부**로
가는 경로 유무 / UDP 수신 포트 2개 / TCP 8554 / **"방화벽과 상대편 상태는 확인하지 않습니다"**.

> UDP 8000/8001은 "UGV 시뮬레이터 시작"으로 들어갈 때만 열린다(`ShouldBeActive`). 결과를 감추는
> 대신 그 사실을 한 줄로 같이 적는다.
>
> 포트 프로브는 일부러 `AsReusable()`을 **안 붙인다.** 실제 수신 소켓은
> `FUdpSocketBuilder().AsReusable()`로 열리지만, 재사용 플래그를 켜면 윈도우에서는 이미 점유된
> 포트에도 바인드가 성공해 점검이 무의미해진다. 실제보다 엄격한 쪽이고 경고 용도로는 맞다.

### 4.7 그 외

- `AdvancedPanel` 접기/펼치기(`AdvancedToggleButton` + `AdvancedToggleLabel` — 화살표는 코드가 바꿈).
  **토글 버튼을 배치 안 하면 패널은 항상 보인다** — 안 그러면 지금 WBP에서 포트/해상도에 영영
  접근할 수 없다.
- `SummaryText` — "지금 이 설정으로 각 버튼을 누르면 무슨 일이 일어나는가". 버튼 라벨만으로는 알
  수 없는 실행 모드를 앞세운다.
- `QuitButton` — 2단 확인(3초 안에 한 번 더). `UConfirmDialogWidget`을 안 쓴 건 WBP 클래스 참조를
  하나 더 요구해서 "이 화면은 WBP 하나로 완성"이라는 계약이 깨지기 때문.
- `EditableText_ip`에 커밋 핸들러 추가(지금까지 없었다 — Client를 누르는 순간에만 읽혔음).

---

## 5. 바뀐 파일

| 파일 | 내용 |
|---|---|
| `UI/AxisSelectionWidget.h` | 헤더 상단에 위 배경 전부 기록(특히 §2의 Solo/Demo 구분), 위젯 이름 계약 9개 추가, 내부 `EAxisLaunchAction` |
| `UI/AxisSelectionWidget.cpp` | `AxisSelectionNet` 네임스페이스(라우팅/어댑터/포트 프로브), `ApplyHelpTooltips`, 기존 3개 핸들러에 커밋+검증 삽입, `SaveConfig()` 2곳 |

C++만 손댔다. `.uasset`은 하나도 안 건드렸다.

---

## 6. WBP에 배치할 위젯 이름 (사용자 작업)

**주 경로 2개는 이미 있던 이름 그대로다** — 지금 WBP도 그대로 동작한다.

| 이름 | 타입 | 상태 |
|---|---|---|
| `HostButton` + `RCIPText` | Button + Editable Text | 기존 |
| `ClientButton` + `EditableText_ip` | Button + Editable Text | 기존 |
| `SoloButton`, `DemoModeCheckBox`, 포트 4칸, 해상도 12칸 | — | 기존 (고급으로 이동) |
| `SettingsButton` | Button | 기존 |
| **`LocalIPText` / `LocalIPDetailText`** | Text Block | 신규 |
| **`SummaryText` / `StatusText`** | Text Block (**Auto Wrap 필수**) | 신규 |
| **`AdvancedPanel`** | 아무 컨테이너 | 신규 |
| **`AdvancedToggleButton` + `AdvancedToggleLabel`** | Button + Text Block | 신규 |
| **`PreflightButton`** | Button | 신규 |
| **`QuitButton`** | Button | 신규 |

클래스 디폴트: `GameSettingsWidgetClass` = `WBP_GameSettings`,
`bAdvancedPanelExpandedByDefault`, `bApplyBuiltInTooltips`(기본 켬).

### ⚠️ 함정 3개 (전부 이 화면에서 실제로 터졌던 것)

1. **바인딩되는 위젯은 전부 같은 WidgetTree 안에.** `BindWidgetOptional`은 그 WBP 자신의 트리만
   훑는다 — "라벨+입력칸"을 자식 UserWidget으로 빼면 안에 있는 `RCIPText`가 영영 안 붙는다.
2. **텍스트 입력은 `Editable Text`만 (`Editable Text Box` 금지).** 이 엔진 버전에서
   `BindWidgetOptional` + `EditableTextBox`가 `Internal Compiler Error`로 100% 재현된다
   (`AxisSelectionWidget.h` 2026-08-20 문단). 부작용: `Editable Text`는 배경 브러시가 없어서
   맨 글씨로 보인다 — **입력칸처럼 보이려면 각각을 `Border`로 감싸야 한다.**
3. **부모 클래스를 반드시 `AxisSelectionWidget`으로.** 부모가 `UserWidget`으로 남아 있으면 이름이
   다 맞아도 **모든 바인딩이 조용히 null**이 된다 — 2026-08-20 UGV→RC 무응답 버그의 진짜 원인.

---

## 7. 남은 것

1. **빌드 + PIE 확인**(사용자). 로그에서 볼 것:
   - `[AxisSelectionWidget] NativeConstruct — Preflight=… AdvToggle=… AdvPanel=… Quit=… …`
   - 종료 후 `Saved/Config/Windows/Game.ini`에 `RCIP=`가 남았는지
2. **WBP 작업**(사용자) — §6.
3. **실행가이드 §4 갱신 — 보류 중.** 엔터 ⚠️ 두 개가 새 빌드에서는 사실이 아니게 되지만, 그
   문서는 "2026-09-04 / 완료(실측 검증)" 상태로 납품 대상 패키지를 설명한다.
   **빌드·실측이 끝난 뒤에 고칠 것** — 검증 안 된 동작을 고객 문서에 먼저 쓰면 안 된다.
4. `guide/`에 축 선택 화면 문서가 없다. WBP까지 끝나 화면이 확정되면 그때 신설 검토.
5. **보류**: 고급 옵션을 `WBP_GameSettings`의 비어 있는 Network 탭으로 이사 — 운용자용 화면이라
   로비에서 바로 만지는 편이 낫다는 판단(2026-09-10 사용자 확정).
