# 교전 오디오 — 보이스 예산 · 총성 MetaSound 분리 · 런타임 감쇠 통일

2026-09-21 / 완료(PIE 실청취 검증됨) / `L_SoldierScenario` 35명 교전에서 총성이 뚝뚝 끊기던 원인은 "관리 시스템 누락"이 아니라 발사량 ×4 × 보이스 상한 32였음. MaxChannels 64 + 총성을 크랙/꼬리 MetaSound로 분리(Concurrency 2겹) + C++ 런타임 감쇠를 NaturalSound+LPF로 통일 + SoldierLab 피격 이펙트 표 복원 + 1인칭 사수 강조를 감쇠 애셋만으로 해결. 추가로 음속 지연 `USoldierAudioLibrary`(§7b)까지 빌드·BP 배선·PIE 청취 검증 완료(PIE 종료 크래시 1건 수정).

대상 레벨: `Content/SoldierLab/Levels/L_SoldierScenario` (SoldierLab `BP_Soldier_Hostile` 15 /
`BP_Soldier_Friendly` 20 + UGV `BP_UGV_0901_C_1` + 트럭 `BP_TitanTruck_C_1`).
P4 submit은 안 함(작업 트리 상태).

---

## 1. 증상과 사용자 가설

교전이 붙으면 총소리가 **뚝뚝 끊기고 이상하게** 들림. 사용자 가설은 "기존 사운드 관리
시스템에서 새 아군/적군(SoldierLab)이 빠진 게 아닌가".

## 2. 조사 — "사운드 관리 시스템"은 총성에 원래 없었다

프로젝트에서 사운드를 "관리"하는 장치는 두 곳뿐이었다:

| 장치 | 위치 | 대상 |
|---|---|---|
| 공유 `USoundConcurrency` | `UFootstepAudioComponent` (`Soldiers/FootstepAudioComponent.cpp:112`) | 구 병사 **발소리**만 |
| Priority 90 + 틱 셀프힐(08-13) | 엔진 루프 `MS_UGVEngine` / `MS_UAVEngine` | 차량 엔진 루프만 |

**총성에는 관리 장치가 애초에 없었다.** 새 SoldierLab 병사는 발소리를 GASP
`AC_FoleyEvents`(오너당 1, 12m 감쇠)로 내므로 발소리 쪽은 문제없음.

구 `BP_EnemyRifle` / `BP_AR4Rifle_old`와 새 `/Game/SoldierLab/Weapons/Blueprints/BP_AR4Rifle`의
`Shoot` 배선도 **완전히 같았다**: `SpawnSoundAttached(assault_rifle_gunshot_01, WeaponMesh,
"Muzzle")`, Concurrency 핀 None. 웨이브는 1.5s 스테레오, `bOverrideConcurrency=false`,
Priority 1, 감쇠 `SA_Weapon`(30m 풀볼륨 / 1.5km 폴오프 NaturalSound -60dB / LPF 2m→50m
20k→500Hz). 즉 "새 병사가 빠졌다"가 아니라 **구 병사와 똑같이 쏘는데 훨씬 많이 쏜다**가
차이였다.

### 2.1 진짜 원인 = 발사량 ×4 + 보이스 상한 32

| | 구 시스템 | 신 SoldierLab |
|---|---|---|
| 근거 | `EnemyCombatComponent.h:514`, `AllyFormationComponent.h:416` | `SoldierEngagement.h:416~422`, `BP_AR4Rifle fireRate 0.12` |
| 버스트 | 3~4발 | 2~5발 @ 8.3발/s |
| 버스트 간격 | 2.5~5s | 0.5s |
| **1인당 발사율** | **≈ 1발/s** | **≈ 3.8발/s** |
| 35명 합계 | ≈ 35발/s | **100발/s 이상** |
| 동시 재생 중인 1.5s 총성 | ≈ 50 | **100~200** |

보이스 상한은 **32**: `DefaultEngine.ini`에 `AudioMaxChannels=0`이라 엔진 기본
`FAudioQualitySettings::MaxChannels(32)`가 쓰이고 있었다(로그 `AudioDevice MaxSources: 32`).

엔진은 매 프레임 `FAudioDevice::GetSortedActiveWaveInstances`(`AudioDevice.cpp:4255~4290`)에서
활성 웨이브를 **볼륨 × 우선순위**로 정렬해 상위 32개만 남기고 나머지는 정지시킨다. 총성은
전부 Priority 1 · 풀볼륨이라 **동률** → 새 총성이 날 때마다 기존 총성 중 아무거나 잘린다.
그리고 원샷은 가상화(virtualization) 대상이 아니라서(`AudioVirtualLoop.cpp:110` `!IsLooping()`)
잘리면 나중에 이어지지도 않고 그냥 **뚝** 끊긴다. 같은 샘플 하나(`gunshot_01`)만 100개
겹치니 위상 간섭으로 소리가 "이상하게" 들리는 것도 한몫.

---

## 3. 수정 1 — 보이스 예산 32 → 64

`Config/DefaultEngine.ini`:

```ini
[/Script/Engine.AudioSettings]
+QualityLevels=(DisplayName="Default",MaxChannels=64)
```

(주석 포함해서 추가.) ini 배열 `+`는 기본값 배열을 통째로 교체한다(`Obj.cpp:3095`).
**에디터 재시작 필요.** 검증: 로그 `AudioDevice MaxSources: 64`.

64로 올려도 초당 100발 × 1.5s = 150개는 여전히 못 담는다 — 예산은 완화일 뿐이고, 진짜
해결은 아래 수정 2(총성 하나가 차지하는 슬롯 자체를 줄이기)다.

---

## 4. 수정 2 — 총성을 크랙/꼬리 MetaSound로 분리 (사용자가 에디터에서 제작, 세션이 스펙 제공)

### 4.1 설계 이유

- 발사음 샘플은 "크랙(짧은 파열음)"과 "꼬리(4.75s 여운)"가 한 파일에 붙어 있었다. **꼬리를
  발마다 통째로 틀면 16슬롯이 4발 만에 찬다.**
- 그래서 매 발 나는 **크랙**과 총 1정당 1~2개만 유지하는 **꼬리**를 분리하고, 꼬리는
  Concurrency 2겹(오너 제한 + 전역 상한)으로 제어.
- "최신 N개만 남기기" 방식은 기각: 초당 100발이면 슬롯이 N/100초마다 재활용돼 여운이 남지
  않는다.

### 4.2 만든 에셋 — `/Game/SoldierLab/Effects/Audio/MetaSounds/`

| 에셋 | 그래프 | 출력/감쇠 | Concurrency |
|---|---|---|---|
| `MSS_RifleCrack` | Random Get(gunshot_01/02/03, NoRepeats) → Wave Player Mono(Pitch ±1 세미톤 랜덤) → Out Mono, On Finished 연결 | Mono / `SA_Weapon`(SoldierLab 것, §7) | Override ✔ **Max 16**, StopFarthestThenOldest, Steal Release 0.05 |
| `MSS_RifleCrack_Enemy` | 위 복제, 피치 **-2.5~-0.5** | 동일 | 동일 |
| `MSS_RifleTail` | `UE.Attenuation` 인터페이스로 `Distance` 입력. Random Get(tail_01/02) → Wave Player Mono(피치 -1.5~0.5) → Mono Mixer(Gain = Map Range Clamped(Distance 0~20000 → 0.4~1.0)) → Out Mono, On Finished(4.75s 끝까지) | 동일 | Override ✘, **Concurrency Set** = `SC_RifleTail_Owner` + `SC_RifleTail_Global` |

- `MSS_RifleCrack_Enemy`의 피치 하향은 **적군 톤 구분용 임시** — 실제 AK 샘플이 생기면 배열만
  교체하면 된다.
- `MSS_RifleTail`의 Mixer Gain은 **사용자가 실청취 후 ×5로 올림**(꼬리 샘플 자체의 레벨이
  낮았음).

Sound Concurrency 에셋 — `/Game/SoldierLab/Effects/Audio/SoundConcurrency/`:

| 에셋 | 값 | 비고 |
|---|---|---|
| `SC_RifleTail_Owner` | **Max 2, Limit To Owner ✔, Stop Oldest, Steal Release 0.25** | 처음엔 Max 1 / PreventNew였는데 **단발 1초 간격 사격에서 첫 발 이후 꼬리가 4.75s 동안 안 나와서** 변경. 버스트에서는 마지막 발 꼬리만 온전히 울리는 결과 |
| `SC_RifleTail_Global` | Max 8, Stop Farthest Then Prevent New, Steal Release 0.3 | 전장 전체 꼬리 상한 |

### 4.3 `BP_AR4Rifle` 배선 (MCP로 편집)

새 함수 `PlayShotSounds`: `OwningCharacter`의 `USoldierIdentityComponent.Faction`으로 Switch —
Hostile → `MSS_RifleCrack_Enemy`, Friendly/Neutral → `MSS_RifleCrack`, 세 갈래 모두 뒤에
`MSS_RifleTail`. 전부 `SpawnSoundAttached(WeaponMesh, "Muzzle")`.

`Shoot` 이벤트의 기존 `SpawnSoundAttached(gunshot_01)` 노드를 삭제하고 그 자리에 호출 삽입:
`Branch(탄약>0) → PlayShotSounds → 머즐 Niagara`. 컴파일 경고 0.

---

## 5. 수정 3 — 피격음 / 휘즈 / RCWS 발사음 감쇠

사용자 보고: "멀리서 총성은 먹먹한데 **피격음·휘즈가 오히려 더 잘 들린다**, RCWS 발사음은
감쇠가 빠진 것 같다."

### 5.1 원인 — 런타임 `USoundAttenuation`은 두 값만 넣으면 Linear · LPF 없음

`ASoldierProjectile` / `ARCWSProjectile` / `URCWSFireControlComponent`는 애셋 감쇠 대신
**런타임 생성 `USoundAttenuation`**에 반경/폴오프 두 값만 넣는다(`guide/rcws_fire_control_dev_guide.md`
§12.1의 07-14 설계). 그러면 나머지는 엔진 기본값 = **Linear 폴오프, LPF 없음**
(`Attenuation.cpp:18`, `SoundAttenuation.cpp:18`).

반면 총성은 애셋 `SA_Weapon`(NaturalSound -60dB + LPF)을 쓰니 원거리에서 **역전**이 난다:

| 소리 | 감쇠 | 300m에서 |
|---|---|---|
| 총성 (`SA_Weapon`) | NaturalSound + LPF | -11dB, 500Hz 이하만 |
| SoldierLab 피격음 (`BP_RifleProjectile` CDO 10m / 1,500m 리니어) | Linear, LPF 없음 | **볼륨 81%, 또렷** |
| RCWS 발사음 (인스턴스 30m / 2,500m 리니어) | Linear, LPF 없음 | **볼륨 89%** |

SoldierLab 피격음의 1000/150000은 RCWS 시절 C++ 기본값(`RCWSProjectile.h:215`)이 이식 때
따라온 것 — titan 쪽 `BP_RifleProjectile`은 08-13에 300/3000으로 고쳤는데, SoldierLab 쪽은
C++ 이식 후 0부터 재제작이라(`soldier_ai_lab/weapons/2026-09-12_projectile_port.md`) 그 수정이
빠졌다.

**휘즈는 감쇠 문제가 아니다.** 탄이 카메라 2m 안을 지날 때 카메라 옆 최근접점에서 재생하는
설계(`SoldierProjectile.cpp:292~303`)라, 관전 카메라로 오버슛 탄이 날아오면 들리는 게 맞다.
`WhizDetectionRadiusCm` 200은 유지.

### 5.2 에셋 값 변경 (MCP, CDO)

| 에셋 | 프로퍼티 | 전 | 후 | 소멸 거리 |
|---|---|---|---|---|
| `/Game/SoldierLab/Weapons/Blueprints/BP_RifleProjectile` | `ImpactSoundAttenuationRadiusCm / FalloffDistanceCm` | 1000 / 150000 | **200 / 2500** | ~27m |
| `/Game/Vehicles/UGV/Effects/BP_RCWSProjectile` | 동일 | 1500 / 25000 | **1000 / 15000** | ~160m |
| `L_SoldierScenario` UGV · 트럭 인스턴스 | `RCWSFireControl.FireSoundFalloffDistanceCm` | 250000 | **150000** | 레벨은 사용자가 에디터 저장 |

### 5.3 C++ — 런타임 감쇠 헬퍼 통일 (.cpp만, 헤더 무변경, Live Coding 적용·검증)

파일별 static 헬퍼 `RCWSFireControl_ConfigureRuntimeAttenuation` /
`RCWSProjectile_ConfigureRuntimeAttenuation` / `SoldierProjectile_ConfigureRuntimeAttenuation`
(Unity Build 심볼 충돌 방지로 파일 접두사). 내용은 셋 다 같음:

- `DistanceAlgorithm = NaturalSound`(-60dB)
- `bAttenuateWithLPF` 2m→50m, 20kHz→500Hz — `SA_Weapon`과 같은 **절대값**. 고주파 공기
  흡수는 소리의 크기와 무관한 거리 함수라 반경/폴오프가 달라도 모든 런타임 감쇠가 같은
  LPF 곡선을 공유하는 게 맞다.

| 파일 | 적용 위치 |
|---|---|
| `Source/titan_example/Vehicles/RCWSFireControlComponent.cpp` | 발사음 (1615) |
| `Source/titan_example/Vehicles/RCWSProjectile.cpp` | 휘즈 (337) · 피격 (1072) |
| `Source/SoldierLab/Weapons/SoldierProjectile.cpp` | 휘즈 (318) · 피격 (1143) |

### 5.4 RCWS 발사음 Concurrency 신설

`RCWSFireControl_GetSharedFireSoundConcurrency()` — 정적 공유 `USoundConcurrency`(AddToRoot)
**Max 8 / StopOldest / Steal Release 0.05**를 `SpawnSoundAtLocation`에 전달. 이유: `UGV_Gunshot`
2.07s × 1200rpm = **동시 41보이스**가 상한 없이 64 예산을 잠식하고 있었다.

---

## 6. 수정 4 — SoldierLab `BP_RifleProjectile.SurfaceImpactEffects`가 비어 있었다

디스크 CDO를 읽어 보니 `SurfaceImpactEffects` 5행이 **전부 `Dirt/None` 빈 껍데기**였다 —
`soldier_ai_lab/IMPLEMENTED.md`와 `weapons/2026-09-12_projectile_port.md`의 "5행 — 행마다
이펙트 + 사운드 + M_Decal_Bullet + MS_Ricochet" 서술과 달랐다. 왜 비어 있었는지는 확인하지
않음(두 문서에 정정 주석만 추가).

titan 구 `/Game/Soldiers/Weapons/BP_RifleProjectile` 값을 SoldierLab 로컬 사본 애셋으로 옮겨 채움:

| Surface | 이펙트 | 사운드 | 데칼 | 도탄 흡수율 |
|---|---|---|---|---|
| Wood | `SoldierLab/Effects/Impact/NS_Rifle_Wood` | `SoldierLab/Effects/Audio/MetaSounds/MS_hit_rifle_wood` | (원본도 없음) | 0.3 |
| Hard | `NS_Rifle_Hard` | `MS_hit_rifle_hard` | `M_Decal_Bullet` | 0.7 |
| Dirt | `NS_Rifle_Dirt` | `MS_hit_rifle_dirt` | `M_Decal_Bullet` | 0.2 |
| Metal | `NS_Rifle_Metal` | `MS_hit_rifle_metal` | `M_Decal_Bullet` | 0.8 |
| Glass | `NS_Rifle_Glass` | `MS_hit_rifle_glass` | `M_Decal_Bullet` | 0.1 |

데칼은 `Gun_effect/Decal_Bullet/Demo/Materials/M_Decal_Bullet` 7×7 / 60s, 도탄음 `MS_Ricochet`
공통. MCP `set_properties`로 5행이 한 번에 들어감("1원소씩 늘어나는" 함정에는 안 걸림).

---

## 7. 수정 5 — 1인칭 사수 총성 강조 / 자기 탄 휘즈 제거

증상: 1인칭으로 쏘면 **옆 아군 총성이 내 총보다 크고**, 내가 쏜 탄의 휘즈가 **매 발** 들림.

### 7.1 자기 탄 휘즈 — 총구↔카메라 거리 기준 스킵

총구가 카메라 앞 50cm라 첫 틱 이동 구간이 항상 2m 판정 반경 안에 들어온다.
`SoldierProjectile.cpp:56,314` / `RCWSProjectile.cpp:58,329`: 발사점(`SpawnLocation`)↔카메라
거리 ≤ 300cm(`*_WhizMuzzleExclusionCm` static constexpr)면 그 카메라엔 휘즈 스킵.

Instigator 비교 대신 **거리 기준**인 이유: 관전 1인칭은 뷰타겟이 관전 폰이라 Instigator로는
못 잡는다.

### 7.2 총성 대비 — 새 2D 사운드 없이 감쇠 애셋만으로 (사용자 제안)

원인: `SA_Weapon` 풀볼륨 반경이 30m라 0.5m 앞 내 총과 5m 옆 총이 **같은 볼륨**이고, 총구가
오른쪽 앞이라 내 총이 하드 패닝된다. 1P 전용 2D MSS를 따로 만드는 대신 **감쇠 애셋 하나로**
해결:

`/Game/SoldierLab/Weapons/Blueprints/SA_Weapon` — **정본으로 채택**(`/Game/Soldiers/Weapons/SA_Weapon`과
값이 같았던 08-12 사본이고 참조자 0이었음):

| 프로퍼티 | 전 | 후 |
|---|---|---|
| 풀볼륨 반경 | 3000cm | **100cm** |
| DistanceAlgorithm | NaturalSound | **Custom** 곡선 (X = (거리-1m)/1500m 정규화): 1m 1.0 / 3m 0.55 / 10m 0.4 / 50m 0.28 / 300m 0.1 / 1.5km 0, 선형 |
| `NonSpatializedRadiusStart/End` | 0 / 0 | **100 / 250cm** (내 총구는 패닝 없이 가운데) |
| `bEnablePriorityAttenuation` | false | **true** (Min/Max 1.0/0.3, 4m~40m — 먼 총성이 보이스 정렬에서 먼저 밀림) |
| LPF · 폴오프 | 유지 | 유지 |

MSS 3개(Crack / Crack_Enemy / Tail)의 `AttenuationSettings`를 이 SoldierLab `SA_Weapon`으로
재지정. `/Game/Soldiers/Weapons/SA_Weapon`은 구 라이플 2개 + 웨이브가 계속 참조하므로 **미변경**.

결과: 내 총 : 옆 아군(3~5m) ≈ **2:1**. 사용자 "아주 잘됨".

---

## 7b. 수정 6 — 음속 지연 (`USoldierAudioLibrary`) — 완료(풀 빌드 · BP 배선 · PIE 검증)

### 7b.1 왜

실제 전장에서 총성은 총구 화염보다 늦게 온다(100m당 약 0.3s). 그리고 나한테 쏜 탄은
초음속 크랙(휘즈, 즉시)이 먼저 오고 총성이 뒤따른다 — 지연을 넣으면 이 **순서가 자동으로**
생긴다(휘즈는 귀 옆에서 나는 소리라 지연 대상에서 제외).

### 7b.2 무엇

신규 `Source/SoldierLab/Weapons/SoldierAudioLibrary.h/.cpp` — `USoldierAudioLibrary`
(BlueprintFunctionLibrary):

| 함수 | 동작 |
|---|---|
| `SpawnSoundAtLocationSpeedOfSound(WorldContext, Sound, Location, Rotation, Volume=1, Pitch=1, Attenuation=null, Concurrency=null, MinDelaySeconds=0.02)` | 가장 가까운 로컬 플레이어 카메라(`PlayerCameraManager`, 휘즈와 같은 기준)까지 거리 ÷ 음속만큼 늦게 `UGameplayStatics::SpawnSoundAtLocation`. 지연 < `MinDelay`(약 7m 이내 — 1인칭 사수 본인)면 즉시 |
| `ComputeSpeedOfSoundDelaySeconds(WorldContext, Location)` | BlueprintPure, 위 지연값만 계산 |

cvar `SoldierLab.Audio.SpeedOfSoundCms` 기본 34300, **0이면 지연 끔**(비교 청취용).

### 7b.3 어떻게 — 발마다 독립 람다 타이머

**엔진에 이미 있지 않나?** (사용자 질문, 09-21) — **없다.** UE 5.8 런타임에서 음속 상수를 쓰는 곳은
`SoundNodeDoppler.cpp:59`(Sound Cue 도플러 **피치**용), MetaSound `ITDPanner`/Spatialization
`ITDSpatializer`(양 귀 시간차, ms 이하), `AISense_Hearing.h:75`(**AI 인지**의 청각 지연 — 플레이어가 듣는
오디오와 무관), ResonanceAudio 플러그인 `shoe_box_room.cc`(실내 초기 반사 지연 — 직접음 전파 지연 아님)
뿐이고, `FSoundAttenuationSettings`에 전파 지연 필드는 없다. 이걸 해주는 건 외부 플러그인(Steam
Audio, Project Acoustics — 실내 반사/전파 시뮬 목적, 이 프로젝트 `Plugins/`에 없고 리눅스 패키징
의존성이 늘어남). Battlefield/Squad/Arma도 게임 코드에서 직접 건다 — 엔진 오디오는 "리스너와
소스가 같은 시간"이 전제라 전파 지연은 게임플레이 층 몫. 코드 없이 되는 유일한 경로는 아래 표의
MetaSound `Trigger Delay`인데 보이스 점유 때문에 기각.

| 후보 | 기각/채택 이유 |
|---|---|
| BP `Delay` 노드 | 실행 중 재진입을 버림 → 8발/s 버스트에서 발이 빠짐 |
| `SetTimerByEvent` | 같은 델리게이트면 기존 타이머를 교체 → 마찬가지로 발이 빠짐 |
| MetaSound 안 `Trigger Delay` | **지연 중에도 보이스를 잡음**(300m면 +0.9s) → Concurrency 16 예산을 갉아먹음 |
| **`FTimerDelegate::CreateLambda` 발마다 독립 타이머** | **채택** — 재생 전까지 보이스 0, 발이 안 빠짐 |

세부:
- **오너 유지**: `SpawnSoundAtLocation`은 `WorldContextObject`에서 오너(액터/컴포넌트 오너)를
  유도하므로 지연 후에도 같은 컨텍스트로 스폰 → `SC_RifleTail_Owner`의 Limit To Owner가 그대로
  동작. 컨텍스트가 지연 중 파괴되면(사수 사망) 월드 컨텍스트로 오너 없이 틀어 소리는 낸다.
- **GC**: 런타임 감쇠 `USoundAttenuation`(NewObject, UPROPERTY 없음)은 지연 중 GC 위험 →
  람다에는 **`FSoundAttenuationSettings` 값 복사**만 들고 있다가 재생 시점에
  `NewObject<USoundAttenuation>(GetTransientPackage())`로 재생성해 넘김(`AudioComponent::Play()`가
  값을 복사하므로 이후 GC 무관). 사운드/Concurrency는 `TWeakObjectPtr`. 오브젝트 강참조는 없음.
  - **PIE 종료 크래시 이력(수정됨)**: 첫 구현은 람다에 `TStrongObjectPtr<USoundAttenuation>`로
    잡았는데, 그 오브젝트의 Outer가 PIE 월드 안 투사체/컴포넌트라 루트 참조가 Outer 사슬
    (액터→레벨→월드)을 살려 PIE 끄는 순간 `PlayLevel.cpp:553` 어설션(`Object 'World
    /Game/SoldierLab/Levels/UEDPIE_0_L_SoldierScenario' from PIE level still referenced`)으로
    에디터가 죽음(풀 빌드 성공 직후 사용자 보고). 위 값 복사 방식으로 고쳐 Live Coding 적용,
    PIE 켰다 끄기 정상 확인.
- **위치는 쏜 순간으로 고정**(부착 아님) — 이동 사격 중 지연된 총성이 사수를 따라다니지 않는다.

### 7b.4 적용 (완료)

| 파일 | 위치 | 소리 |
|---|---|---|
| `RCWSFireControlComponent.cpp` | 1619 | RCWS 발사음 (WorldContext=this) |
| `RCWSProjectile.cpp` | 1075 | RCWS 피격음 |
| `SoldierProjectile.cpp` | 1146 | 소총 피격음 |
| `BP_AR4Rifle.PlayShotSounds` (BP, MCP) | — | 소총 총성 크랙+꼬리: `WeaponMesh.GetSocketLocation("Muzzle")` → 진영 Switch → `SoldierLab\|Audio\|SpawnSoundAtLocation(SpeedOfSoundDelay)`(WorldContext=self, Crack/Crack_Enemy + Tail, 감쇠/Concurrency 핀 비움 = MSS 애셋 설정 사용) ×3 갈래. 컴파일 경고 0, 저장 확인 |

BP 배선 함정: `PlayShotSounds`를 삭제 후 같은 이름으로 재추가하면 구 UFunction이 남아
`PlayShotSounds_0`으로 생긴다 → `_0` 삭제 → 컴파일(호출 노드 에러 1회 예상) → 재추가 순서로
해결.

### 7b.5 검증

- 새 UCLASS라 Live Coding 불가 → 에디터 닫고 풀 빌드(사용자) ✅.
- 빌드 직후 PIE 종료 크래시 1건 → §7b.3 값 복사로 수정 ✅.
- BP 배선 후 사용자 PIE 청취 검증 **"성공"** ✅.

---

## 8. 보류 / 미적용 (사용자 결정)

| 항목 | 상태 |
|---|---|
| 오디오 차폐(`bEnableOcclusion`) | **보류** — 현재 시나리오는 개활지. 레벨 디자인이 바뀌면 재검토(`soldier_ai_lab/OPEN_ITEMS.md` W22) |
| 전장 밀도 베드(`GetGunshotCount`로 루프 1개 구동) | 아이디어만 기록, 미구현 |
| 1P 저역 펀치 레이어(MSS 파라미터로 진영 구분하는 B안) | 미구현 — §7.2 감쇠 애셋만으로 충분했음 |
| 덕킹 SoundMix | 미구현 |
| 휘즈 `WhizDetectionRadiusCm` | 200 유지 |

---

## 9. 변경 파일

| 구분 | 파일 |
|---|---|
| Config | `Config/DefaultEngine.ini` (MaxChannels 64) |
| C++ (.cpp만) | `Source/titan_example/Vehicles/RCWSFireControlComponent.cpp`, `Source/titan_example/Vehicles/RCWSProjectile.cpp`, `Source/SoldierLab/Weapons/SoldierProjectile.cpp` (감쇠 헬퍼 §5.3 + 음속 지연 호출 §7b.4) |
| C++ 신규 (§7b) | `Source/SoldierLab/Weapons/SoldierAudioLibrary.h`, `SoldierAudioLibrary.cpp` |
| BP/애셋 (MCP) | `Content/SoldierLab/Weapons/Blueprints/BP_AR4Rifle.uasset`, `BP_RifleProjectile.uasset`, `SA_Weapon.uasset`, `Content/Vehicles/UGV/Effects/BP_RCWSProjectile.uasset` |
| 신규 (사용자 제작) | `Content/SoldierLab/Effects/Audio/MetaSounds/MSS_RifleCrack` / `MSS_RifleCrack_Enemy` / `MSS_RifleTail`, `Content/SoldierLab/Effects/Audio/SoundConcurrency/SC_RifleTail_Owner` / `SC_RifleTail_Global` |
| 레벨 (사용자 저장) | `Content/SoldierLab/Levels/L_SoldierScenario.umap` (UGV·트럭 인스턴스 `FireSoundFalloffDistanceCm`) |

P4 submit 안 함.

---

## 10. 교훈

1. **총성이 끊기면 보이스 수부터 본다** — `stat sounds`, 로그 `AudioDevice MaxSources`.
   우선순위 동률인 원샷은 가상화 없이 그냥 잘린다. "관리 시스템에서 빠졌나"보다 "몇 개가
   동시에 나는가"가 먼저.
2. **C++ 런타임 `USoundAttenuation`은 두 값만 넣으면 Linear · LPF 없음이 기본이다.** 애셋
   감쇠(NaturalSound+LPF)를 쓰는 소리보다 원거리에서 더 또렷해지는 역전이 난다. 이
   프로젝트의 런타임 감쇠는 이제 `*_ConfigureRuntimeAttenuation` 헬퍼로 통일 — 새로 런타임
   감쇠를 만들 일이 있으면 이 헬퍼 패턴을 따를 것.
3. **긴 여운 샘플은 발마다 틀지 않는다.** 크랙/꼬리 분리 + Owner 제한 + 전역 상한 2겹
   Concurrency.
4. **1인칭 강조는 새 2D 사운드 없이도 된다** — 감쇠 커스텀 곡선 + `NonSpatializedRadius` +
   `PriorityAttenuation` 조합.
5. **`SA_Weapon` 정본은 `/Game/SoldierLab/Weapons/Blueprints/`.** `/Game/Soldiers/Weapons/`
   쪽은 구 라이플용으로 남아 있을 뿐.
6. **긴 지연은 타이머로, MetaSound 지연은 보이스를 잡는다.** `Trigger Delay`는 기다리는 동안도
   Concurrency 슬롯 하나를 차지하고, BP `Delay`/`SetTimerByEvent`는 버스트에서 발을 빠뜨린다 —
   발마다 독립 람다 타이머(§7b.3).
7. **타이머/람다에 PIE 월드 안 오브젝트를 `TStrongObjectPtr`로 잡지 말 것** — Outer 사슬이
   월드를 붙잡아 PIE 종료 시 `PlayLevel.cpp:553` 어설션 크래시. 값 복사 + 재생 시점 재생성으로
   (§7b.3).
