# 1. 통신프로토콜

value 값이 string -> int (현상태) 로 유지하는거로. 수정 필요 없음

# 2. 슈어 - LIG

keys 부분 오타 몇개 존재

Azimuth 등 UGV RCWS Status 에 3개 값 추가

++ RCWS 객체인식 결과 보고 부분 피드백 '영상의 보이는 타겟의 위치와 box의 위치 차이가 심함 
- 이거 우리가 만든 ugv controller demo 로 테스트했을때는 딱히 심하진 않았는데 뭔가 버그가 있는듯?

Runtime 값 추가 (에뮬레이터 구동시간이 시나리오 재시작 시 초기화 되어야 되는 값인가?)

UGV 객체 탐색 ObjectClass 부분 메모 ('값을 기존 claaified')
- 현재 구현 어떻게 되어있음? 내 기억상 저 human, car 는 처음에 lig 에서 제안한거고 우리는 ugv, uav, truck, ally, enemy 등등 객체 분류를 더 상세화 해서 전달했고 그걸 말하는건가?

# 3. LIG - 슈어

Steering 좌/우 조향 감도 낮추기

SelectCameraButton 메모 ('ir 카메라 흑백 마스킹')
- IR 카메라를 초록색 말고 흑백으로 바꾸라는 말인가?