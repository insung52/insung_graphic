/**
 * 임무장비 모의기 진행현황 — 구글시트 서식 스크립트
 *
 * 사용법: 구글시트에서 확장 프로그램 ▸ Apps Script ▸ 이 파일 내용 붙여넣기 ▸ 저장
 *   1_진행현황 탭을 클릭해 활성화한 뒤  formatProgressSheet  실행
 *     → 상단 제목 블록(진행률 자동/체감) + 서식·그룹·필터·드롭다운 + 완료일~비고 사이 인라인 Gantt
 * 처음 실행 시 권한 승인 창이 뜸(이 스프레드시트만 접근). 데이터를 고친 뒤 재실행하면 전부 다시 그림.
 * 재실행해도 체감 진행률(수동 입력)·메모·갱신일은 지우지 않는다.
 *
 * 열은 **헤더 이름으로 찾는다** — 순서를 바꿔도 됨. 필수 헤더:
 *   WBS, L1, L2, 구분, 기존WBS, 작업, 현재상태, 실제구현내용, 근거문서, 시작일, 완료일, 비고
 * 선택: 기존비고, 기존완료여부. Gantt는 '완료일' 바로 오른쪽에 삽입된다(재실행 시 기존 Gantt 열은 지우고 다시).
 *
 * Gantt 방식은 아래 GANTT_MODE 로 고른다:
 *   'single' (기본) — XLGantt처럼 넓은 열 하나("Gantt"), 셀마다 SPARKLINE 막대. 헤더 위 2행에 기간 축(월 띠 + 오늘).
 *                    막대는 수식이라 시트에서 시작/완료/상태를 고치면 즉시 따라가고, 축은 행 추가에도 자동으로 늘어남.
 *   'weeks'         — 주마다 열 하나, 조건부 서식으로 칸을 칠함. 인쇄·격자 확인용.
 */

var TITLE_ROWS = 4;              // 헤더 위 제목 블록 행 수
var REQUIRED = ['WBS', 'L1', 'L2', '구분', '기존WBS', '작업', '현재상태', '실제구현내용', '근거문서', '시작일', '완료일', '비고'];

var STATUS_STYLE = {
  '완료':                 { bg: '#d9ead3', fg: '#000000', bar: '#6aa84f' },
  '완료(구현방식 확정)':  { bg: '#cfe2f3', fg: '#000000', bar: '#3d85c6' },
  '진행중':               { bg: '#fff2cc', fg: '#000000', bar: '#f1c232' },
  '미착수':               { bg: '#fce5cd', fg: '#000000', bar: '#e69138' },
  '보류(상위체계 미확정)': { bg: '#e4d7f5', fg: '#000000', bar: '#8e7cc3' },
  '보류(일시중단)':       { bg: '#e4d7f5', fg: '#000000', bar: '#8e7cc3' },
  '폐기/불필요':          { bg: '#d9d9d9', fg: '#808080', bar: '#999999' }
};
var STATUS_LIST = Object.keys(STATUS_STYLE);

var HDR_BG = '#37474f', HDR_FG = '#ffffff';
var L1_BG = '#b0bec5', L2_BG = '#eceff1';
var INPUT_BG = '#fff9c4';        // 사용자가 직접 입력하는 칸

// Gantt 방식 — 'single': XLGantt처럼 넓은 열 하나에 셀 안 막대(SPARKLINE), 'weeks': 주마다 열 하나(조건부 서식)
var GANTT_MODE = 'single';
var GANTT_COL_WIDTH = 440;       // single 모드 열 너비(px)
var WEEK_COL_WIDTH = 20;         // weeks 모드 주 열 너비(px)
var WIDTHS = { 'WBS':58, 'L1':90, 'L2':160, '구분':48, '기존WBS':62, '작업':330, '현재상태':130,
               '실제구현내용':520, '근거문서':270, '시작일':88, '완료일':88, '비고':170, '기존비고':150, '기존완료여부':52 };

// ================================================================== main
function formatProgressSheet() {
  var sh = SpreadsheetApp.getActiveSheet();
  var hdrRow = findHeaderRow_(sh);
  if (hdrRow < 0) throw new Error('1_진행현황 탭을 활성화한 뒤 실행하세요 (A열 위쪽 10행 안에 "WBS" 헤더 필요). 현재 탭: ' + sh.getName());

  // 헤더 위에 제목 블록 자리 확보
  if (hdrRow <= TITLE_ROWS) { sh.insertRowsBefore(1, TITLE_ROWS + 1 - hdrRow); hdrRow = TITLE_ROWS + 1; }
  var first = hdrRow + 1;

  // -- 초기화: 제목 블록 수동값 보존 → 고정 해제 → 기존 Gantt 주 열 제거 → 서식/규칙/필터/그룹 제거
  var col = headerMap_(sh, hdrRow);
  var keep = readTitleInputs_(sh, col['작업']);
  sh.setFrozenRows(0); sh.setFrozenColumns(0);          // 고정 경계에 걸친 병합/해제가 막히지 않게
  sh.getRange(1, 1, TITLE_ROWS, sh.getLastColumn()).breakApart();
  removeGanttColumns_(sh, hdrRow);
  col = headerMap_(sh, hdrRow);
  var lastRow = sh.getLastRow();
  var nData = lastRow - hdrRow;
  var lastCol = sh.getLastColumn();
  sh.getRange(1, 1, lastRow, lastCol).clearFormat();
  sh.clearConditionalFormatRules();
  if (sh.getFilter()) sh.getFilter().remove();
  collapseAllGroups_(sh, lastRow);

  // -- 날짜 열: 텍스트 → 진짜 날짜 (Gantt·타임라인 전제)
  fixDates_(sh, first, nData, col['시작일']);
  fixDates_(sh, first, nData, col['완료일']);

  // -- 인라인 Gantt 삽입 (완료일 오른쪽). 이후 열 인덱스가 바뀌므로 headerMap 다시.
  var gantt = (GANTT_MODE === 'weeks') ? insertWeekColumns_(sh, hdrRow, first, nData, col)
                                       : insertGanttColumn_(sh, hdrRow, first, nData, col);
  col = headerMap_(sh, hdrRow);
  lastCol = sh.getLastColumn();
  var values = sh.getRange(first, 1, nData, lastCol).getValues();

  // -- 제목 블록
  writeTitleBlock_(sh, col, keep, gantt.startCol);
  if (gantt.mode === 'single') writeGanttHeader_(sh, hdrRow, gantt);

  // -- 헤더
  sh.getRange(hdrRow, 1, 1, lastCol).setBackground(HDR_BG).setFontColor(HDR_FG).setFontWeight('bold').setFontSize(10)
    .setHorizontalAlignment('center').setVerticalAlignment('middle').setWrap(true);
  sh.setRowHeight(hdrRow, gantt.mode === 'weeks' ? 48 : 34);
  sh.setFrozenRows(hdrRow);
  sh.setFrozenColumns(col['작업']);   // 작업 열까지 고정 — 주 열로 스크롤해도 작업명이 보이게

  // -- 열 너비
  for (var name in WIDTHS) if (col[name]) sh.setColumnWidth(col[name], WIDTHS[name]);

  // -- 본문 공통
  sh.getRange(first, 1, nData, lastCol).setFontSize(9).setVerticalAlignment('top').setWrap(true);
  ['WBS', '구분', '기존WBS', '기존완료여부'].forEach(function (n) {
    if (col[n]) sh.getRange(first, col[n], nData, 1).setHorizontalAlignment('center');
  });
  sh.getRange(first, col['현재상태'], nData, 1).setHorizontalAlignment('center').setFontWeight('bold');
  sh.getRange(first, col['시작일'], nData, 1).setHorizontalAlignment('center').setNumberFormat('yyyy-mm-dd');
  sh.getRange(first, col['완료일'], nData, 1).setHorizontalAlignment('center').setNumberFormat('yyyy-mm-dd');

  // -- L1/L2 행 스타일 + 행 그룹
  var groups = styleAndGroup_(sh, values, col['구분'], first, lastRow, lastCol);

  // -- 조건부 서식
  var rules = [];
  var L = function (c) { return columnLetter_(c); };
  var stL = L(col['현재상태']), kdL = L(col['구분']), sL = L(col['시작일']), eL = L(col['완료일']);
  var statusRange = sh.getRange(first, col['현재상태'], nData, 1);
  STATUS_LIST.forEach(function (st) {
    rules.push(SpreadsheetApp.newConditionalFormatRule().whenTextEqualTo(st)
      .setBackground(STATUS_STYLE[st].bg).setFontColor(STATUS_STYLE[st].fg).setRanges([statusRange]).build());
  });
  if (gantt.mode === 'weeks' && gantt.n > 0) {
    var bar = sh.getRange(first, gantt.startCol, nData, gantt.n);
    var wL = L(gantt.startCol);
    STATUS_LIST.forEach(function (st) {
      var f = '=AND($' + sL + first + '<>"", $' + eL + first + '<>"", ' + wL + '$' + hdrRow + '<=$' + eL + first +
              ', ' + wL + '$' + hdrRow + '+6>=$' + sL + first + ', $' + stL + first + '="' + st + '")';
      rules.push(SpreadsheetApp.newConditionalFormatRule().whenFormulaSatisfied(f).setBackground(STATUS_STYLE[st].bar).setRanges([bar]).build());
    });
    // 오늘이 속한 주: 헤더 빨강 + 본문 세로 띠
    var wkHdr = sh.getRange(hdrRow, gantt.startCol, 1, gantt.n);
    rules.push(SpreadsheetApp.newConditionalFormatRule()
      .whenFormulaSatisfied('=AND(' + wL + '$' + hdrRow + '<=TODAY(), ' + wL + '$' + hdrRow + '+6>=TODAY())')
      .setBackground('#e53935').setFontColor('#ffffff').setRanges([wkHdr]).build());
    rules.push(SpreadsheetApp.newConditionalFormatRule()
      .whenFormulaSatisfied('=AND(' + wL + '$' + hdrRow + '<=TODAY(), ' + wL + '$' + hdrRow + '+6>=TODAY(), $' + sL + first + '="")')
      .setBackground('#fdecea').setRanges([bar]).build());
  }
  // 폐기 행 전체 흐리게, 신규 행 표시
  rules.push(SpreadsheetApp.newConditionalFormatRule()
    .whenFormulaSatisfied('=$' + stL + first + '="폐기/불필요"')
    .setBackground('#efefef').setFontColor('#9e9e9e').setRanges([sh.getRange(first, 1, nData, lastCol)]).build());
  rules.push(SpreadsheetApp.newConditionalFormatRule()
    .whenFormulaSatisfied('=$' + kdL + first + '="신규"').setBackground('#e8f0fe')
    .setRanges([sh.getRange(first, col['WBS'], nData, 1), sh.getRange(first, col['구분'], nData, 1)]).build());
  sh.setConditionalFormatRules(rules);

  // -- 드롭다운
  var statusRule = SpreadsheetApp.newDataValidation().requireValueInList(STATUS_LIST, true).setAllowInvalid(true).build();
  var kindRule = SpreadsheetApp.newDataValidation().requireValueInList(['L1', 'L2', '기존', '신규'], true).setAllowInvalid(true).build();
  for (var j = 0; j < nData; j++) {
    var k = String(values[j][col['구분'] - 1]).trim();
    if (k === '기존' || k === '신규') sh.getRange(first + j, col['현재상태']).setDataValidation(statusRule);
  }
  sh.getRange(first, col['구분'], nData, 1).setDataValidation(kindRule);

  // -- 필터 + 테두리
  sh.getRange(hdrRow, 1, nData + 1, lastCol).createFilter();
  sh.getRange(hdrRow, 1, nData + 1, lastCol).setBorder(true, true, true, true, true, true, '#d0d0d0', SpreadsheetApp.BorderStyle.SOLID);
  if (gantt.mode === 'weeks' && gantt.n > 0) {
    // 주 열 블록 좌우 굵은 선 + 월 경계선
    sh.getRange(hdrRow, gantt.startCol, nData + 1, gantt.n).setBorder(null, true, null, true, null, null, '#616161', SpreadsheetApp.BorderStyle.SOLID_MEDIUM);
    for (var w = 1; w < gantt.weeks.length; w++) {
      if (gantt.weeks[w].getMonth() !== gantt.weeks[w - 1].getMonth())
        sh.getRange(hdrRow, gantt.startCol + w, nData + 1, 1).setBorder(null, true, null, null, null, null, '#9e9e9e', SpreadsheetApp.BorderStyle.SOLID_MEDIUM);
    }
  } else if (gantt.mode === 'single') {
    sh.setColumnWidth(gantt.startCol, GANTT_COL_WIDTH);
    sh.getRange(TITLE_ROWS - 1, gantt.startCol, nData + 3, 1)
      .setBorder(null, true, null, true, null, null, '#616161', SpreadsheetApp.BorderStyle.SOLID_MEDIUM);
    sh.getRange(first, gantt.startCol, nData, 1).setVerticalAlignment('middle').setHorizontalAlignment('left').setWrap(false);
  }

  var ganttMsg = gantt.mode === 'weeks' ? ('Gantt ' + gantt.n + '주') : ('Gantt ' + fmt_(gantt.min) + '~' + fmt_(gantt.max));
  SpreadsheetApp.getActive().toast('완료: 작업 ' + nData + '행, L1 ' + groups.l1 + '/L2 ' + groups.l2 + ' 그룹, ' + ganttMsg, '진행현황', 6);
}

// ================================================================== 제목 블록
//  1행: 제목 ................................................ 갱신일(수동, 비어 있으면 오늘)
//  2행: 전체 진행률(자동)  [%]  [바]                        집계 기준 설명
//  3행: 체감 진행률(수동)  [%]  [바]                        메모(수동)
//  4행: 상태별 집계(자동)
// 제목 블록의 병합은 고정 열 경계(fz = '작업' 열)를 절대 가로지르지 않는다 — 구글시트가 금지함.
//   왼쪽(A..fz): 제목 / 진행률 % + 바 / 상태별 집계        오른쪽(fz+1..): 갱신일 / 집계 기준 / 메모 / 작업 행 집계
function readTitleInputs_(sh, fz) {
  return { updated: sh.getRange(1, fz + 2).getValue(), felt: sh.getRange(3, 2).getValue(), memo: sh.getRange(3, fz + 2).getValue() };
}
function writeTitleBlock_(sh, col, keep, ganttCol) {
  var fz = col['작업'];                         // 고정 열 경계
  var R = fz + 1, RV = fz + 2, RW = 4;           // 오른쪽 블록: 라벨 열, 값 시작 열, 값 병합 폭
  if (ganttCol > RV && ganttCol - RV < RW) RW = ganttCol - RV;   // Gantt 열 자리는 병합에 넣지 않는다
  if (RW < 1) RW = 1;
  var S = columnLetter_(col['현재상태']) + ':' + columnLetter_(col['현재상태']);
  var K = columnLetter_(col['구분']) + ':' + columnLetter_(col['구분']);
  var cnt = function (v) { return 'COUNTIF(' + S + ',"' + v + '")'; };
  var done = cnt('완료'), done2 = cnt('완료(구현방식 확정)'), prog = cnt('진행중'), todo = cnt('미착수'),
      hold = cnt('보류(상위체계 미확정)') + '+' + cnt('보류(일시중단)'), drop = cnt('폐기/불필요');
  var tasks = '(COUNTIF(' + K + ',"기존")+COUNTIF(' + K + ',"신규"))';
  var denom = '(' + tasks + '-' + drop + ')';
  var mergeL = function (row, c0) { if (fz > c0) sh.getRange(row, c0, 1, fz - c0 + 1).merge(); };
  var mergeR = function (row) { sh.getRange(row, RV, 1, RW).merge(); };

  // 1행
  sh.getRange(1, 1).setValue('임무장비 모의기 — 개발 진행현황').setFontSize(16).setFontWeight('bold');
  mergeL(1, 1);
  sh.getRange(1, R).setValue('갱신일').setFontWeight('bold').setHorizontalAlignment('right');
  sh.getRange(1, RV).setValue(keep.updated instanceof Date ? keep.updated : new Date())
    .setNumberFormat('yyyy-mm-dd').setBackground(INPUT_BG).setHorizontalAlignment('center');
  sh.setRowHeight(1, 30);

  // 2행 자동 진행률
  sh.getRange(2, 1).setValue('전체 진행률 (자동)').setFontWeight('bold');
  sh.getRange(2, 2).setFormula('=IFERROR((' + done + '+' + done2 + ')/' + denom + ',0)')
    .setNumberFormat('0.0%').setFontSize(12).setFontWeight('bold').setHorizontalAlignment('center');
  sh.getRange(2, 3).setFormula('=SPARKLINE(B2,{"charttype","bar";"max",1;"color1","#6aa84f"})');
  mergeL(2, 3);
  sh.getRange(2, R).setValue('집계 기준').setFontWeight('bold').setHorizontalAlignment('right');
  sh.getRange(2, RV).setValue('완료 + 완료(구현방식 확정) ÷ 작업 행 전체(폐기/불필요 제외). 진행중은 0으로 계산.')
    .setFontSize(9).setFontColor('#616161').setWrap(true);
  mergeR(2);

  // 3행 체감 진행률(수동)
  sh.getRange(3, 1).setValue('체감 진행률 (수동)').setFontWeight('bold');
  var felt = sh.getRange(3, 2);
  felt.setValue(typeof keep.felt === 'number' ? keep.felt : '');
  felt.setNumberFormat('0%').setFontSize(12).setFontWeight('bold').setHorizontalAlignment('center').setBackground(INPUT_BG);
  felt.setDataValidation(SpreadsheetApp.newDataValidation().requireNumberBetween(0, 1).setAllowInvalid(true)
    .setHelpText('0~1 사이 소수 또는 %로 입력 (예: 0.6 또는 60%)').build());
  sh.getRange(3, 3).setFormula('=IF(B3="","",SPARKLINE(B3,{"charttype","bar";"max",1;"color1","#3d85c6"}))');
  mergeL(3, 3);
  sh.getRange(3, R).setValue('메모').setFontWeight('bold').setHorizontalAlignment('right');
  sh.getRange(3, RV).setValue(keep.memo || '').setBackground(INPUT_BG).setFontSize(9).setWrap(true);
  mergeR(3);

  // 4행 집계 — 왼쪽: 상태별, 오른쪽: 작업 행 수
  sh.getRange(4, 1).setValue('상태별 집계').setFontWeight('bold');
  sh.getRange(4, 2).setFormula(
    '="완료 "&' + done + '&" · 구현방식 확정 "&' + done2 + '&" · 진행중 "&' + prog + '&" · 미착수 "&' + todo +
    '&" · 보류 "&(' + hold + ')&" · 폐기 "&' + drop).setFontSize(9).setFontColor('#424242');
  mergeL(4, 2);
  sh.getRange(4, R).setValue('작업 행').setFontWeight('bold').setHorizontalAlignment('right');
  sh.getRange(4, RV).setFormula('=' + tasks + '&" (기존 "&COUNTIF(' + K + ',"기존")&" / 신규 "&COUNTIF(' + K + ',"신규")&")"')
    .setFontSize(9).setFontColor('#424242');
  mergeR(4);

  sh.getRange(1, 1, TITLE_ROWS, RV + RW).setVerticalAlignment('middle');
  sh.getRange(2, 1, 3, 1).setFontSize(10);
  for (var r = 2; r <= TITLE_ROWS; r++) sh.setRowHeight(r, 24);
  sh.getRange(TITLE_ROWS, 1, 1, sh.getLastColumn()).setBorder(null, null, true, null, null, null, '#9e9e9e', SpreadsheetApp.BorderStyle.SOLID_MEDIUM);
}

// ================================================================== 인라인 Gantt
function ganttSpan_(sh, first, nData, col) {
  // 데이터 행의 시작일 최소 ~ max(완료일 최대, 오늘). 없으면 null.
  var sv = sh.getRange(first, col['시작일'], nData, 1).getValues();
  var ev = sh.getRange(first, col['완료일'], nData, 1).getValues();
  var minD = null, maxD = null;
  for (var i = 0; i < nData; i++) {
    var s = toDate_(sv[i][0]), e = toDate_(ev[i][0]);
    if (s && (!minD || s < minD)) minD = s;
    if (e && (!maxD || e > maxD)) maxD = e;
  }
  if (!minD) return null;
  var today = new Date(); today = new Date(today.getFullYear(), today.getMonth(), today.getDate());
  if (!maxD || maxD < today) maxD = today;
  return { min: minD, max: maxD, today: today };
}

// --- single 모드: XLGantt처럼 넓은 열 하나, 셀마다 SPARKLINE 스택 바(투명 | 막대 | 투명)
//     막대 위치·길이는 시작일/완료일에서 매 계산되므로 시트에서 날짜를 고치면 즉시 따라간다.
//     기간 축은 MIN(시작일 열)~MAX(완료일 열, 오늘) — 행을 추가해도 자동으로 늘어난다.
function insertGanttColumn_(sh, hdrRow, first, nData, col) {
  var span = ganttSpan_(sh, first, nData, col);
  if (!span) return { mode: 'single', n: 0, startCol: -1 };
  var at = col['완료일'] + 1;
  sh.insertColumnsAfter(col['완료일'], 1);
  sh.getRange(hdrRow, at).setValue('Gantt');

  var sL = columnLetter_(col['시작일']), eL = columnLetter_(col['완료일']), stL = columnLetter_(col['현재상태']);
  var kind = sh.getRange(first, col['구분'], nData, 1).getValues();
  var axisMin = 'MIN($' + sL + ':$' + sL + ')';
  var axisMax = 'MAX(MAX($' + eL + ':$' + eL + '),TODAY())';
  // 상태 → 막대 색
  var colorExpr = 'IFERROR(SWITCH($' + stL + '{r}';
  STATUS_LIST.forEach(function (st) { colorExpr += ',"' + st + '","' + STATUS_STYLE[st].bar + '"'; });
  colorExpr += '),"#999999")';

  var formulas = [];
  for (var i = 0; i < nData; i++) {
    var r = first + i;
    var k = String(kind[i][0]).trim();
    if (k === 'L1' || k === 'L2') { formulas.push(['']); continue; }
    var S = '$' + sL + r, E = '$' + eL + r;
    var f = '=IF(OR(' + S + '="",' + E + '=""),"",' +
      'SPARKLINE({' + S + '-' + axisMin + ', ' + E + '-' + S + '+1, ' + axisMax + '-' + E + '},' +
      '{"charttype","bar";"color1","#ffffff";"color2",' + colorExpr.replace('{r}', r) + ';"color3","#ffffff";' +
      '"max",' + axisMax + '-' + axisMin + '+1}))';
    formulas.push([f]);
  }
  sh.getRange(first, at, nData, 1).setFormulas(formulas);
  return { mode: 'single', n: 1, startCol: at, min: span.min, max: span.max, today: span.today };
}

// 제목 블록의 Gantt 열 자리(헤더 위 2행)에 기간 축을 그린다:
//   TITLE_ROWS-1 행: "05/04 ── 09/21" 텍스트 + 월 라벨,  TITLE_ROWS 행: 월별 띠(교대색) + 오늘 표시(빨강) SPARKLINE
function writeGanttHeader_(sh, hdrRow, g) {
  var c = g.startCol;
  var labelRow = TITLE_ROWS - 1, bandRow = TITLE_ROWS;
  // 월별 일수 세그먼트 (오늘이 속한 월은 오늘 앞/오늘(1일)/오늘 뒤 세 조각으로)
  var segs = [], colors = [], labels = [];
  var d = new Date(g.min), i = 0;
  while (d <= g.max) {
    var mStart = new Date(d);
    var mEnd = new Date(d.getFullYear(), d.getMonth() + 1, 0);
    if (mEnd > g.max) mEnd = new Date(g.max);
    var days = Math.round((mEnd - mStart) / 86400000) + 1;
    var base = (i % 2 === 0) ? '#b0bec5' : '#78909c';
    if (g.today >= mStart && g.today <= mEnd) {
      var before = Math.round((g.today - mStart) / 86400000);
      var after = days - before - 1;
      if (before > 0) { segs.push(before); colors.push(base); }
      segs.push(1); colors.push('#e53935');
      if (after > 0) { segs.push(after); colors.push(base); }
    } else { segs.push(days); colors.push(base); }
    labels.push((d.getMonth() + 1) + '월');
    d = new Date(d.getFullYear(), d.getMonth() + 1, 1); i++;
  }
  var opts = ['"charttype","bar"', '"max",' + segs.reduce(function (a, b) { return a + b; }, 0)];
  colors.forEach(function (cl, k) { opts.push('"color' + (k + 1) + '","' + cl + '"'); });
  sh.getRange(bandRow, c).setFormula('=SPARKLINE({' + segs.join(',') + '},{' + opts.join(';') + '})');
  sh.getRange(labelRow, c).setValue(fmt2_(g.min) + '  ' + labels.join(' · ') + '  ' + fmt2_(g.max) + '   (■ 오늘 ' + fmt2_(g.today) + ')')
    .setFontSize(9).setFontColor('#37474f').setHorizontalAlignment('center').setWrap(false);
  sh.getRange(labelRow, c, 2, 1).setVerticalAlignment('middle');
}

// --- weeks 모드: 주마다 열 하나, 막대는 조건부 서식
function insertWeekColumns_(sh, hdrRow, first, nData, col) {
  var span = ganttSpan_(sh, first, nData, col);
  if (!span) return { mode: 'weeks', n: 0, startCol: -1, weeks: [] };
  var minD = mondayOf_(span.min), maxD = mondayOf_(span.max);
  var weeks = [];
  for (var d = new Date(minD); d <= maxD; d.setDate(d.getDate() + 7)) weeks.push(new Date(d));
  var at = col['완료일'] + 1;
  sh.insertColumnsAfter(col['완료일'], weeks.length);
  sh.getRange(hdrRow, at, 1, weeks.length).setValues([weeks]).setNumberFormat('mm/dd').setTextRotation(90);
  for (var c = 0; c < weeks.length; c++) sh.setColumnWidth(at + c, WEEK_COL_WIDTH);
  return { mode: 'weeks', n: weeks.length, startCol: at, weeks: weeks, min: span.min, max: span.max };
}

// 이전 실행이 넣은 Gantt 열 제거 — 헤더가 날짜인 열(weeks) + 헤더가 'Gantt'인 열(single)
function removeGanttColumns_(sh, hdrRow) {
  var lastCol = sh.getLastColumn();
  var hv = sh.getRange(hdrRow, 1, 1, lastCol).getValues()[0];
  var runs = [];
  for (var c = 0; c < hv.length; c++) {
    var isG = (hv[c] instanceof Date) || String(hv[c]).trim() === 'Gantt';
    if (isG) {
      if (runs.length && runs[runs.length - 1][1] === c) runs[runs.length - 1][1] = c + 1;
      else runs.push([c + 1, c + 1]);
    }
  }
  for (var r = runs.length - 1; r >= 0; r--) sh.deleteColumns(runs[r][0], runs[r][1] - runs[r][0] + 1);
}

// ================================================================== helpers
function headerMap_(sh, hdrRow) {
  var hv = sh.getRange(hdrRow, 1, 1, sh.getLastColumn()).getValues()[0];
  var m = {};
  hv.forEach(function (h, i) { var k = String(h).trim(); if (k && !(h instanceof Date)) m[k] = i + 1; });
  var missing = REQUIRED.filter(function (k) { return !m[k]; });
  if (missing.length) throw new Error('헤더에 없는 열: ' + missing.join(', ') + '  (헤더 이름이 정확히 일치해야 합니다)');
  return m;
}
function styleAndGroup_(sh, values, kindCol, first, lastRow, ncol) {
  var groupsL1 = [], groupsL2 = [], l1Start = -1, l2Start = -1;
  for (var i = 0; i < values.length; i++) {
    var r = first + i;
    var kind = String(values[i][kindCol - 1]).trim();
    if (kind === 'L1') {
      if (l2Start > 0) { groupsL2.push([l2Start + 1, r - 1]); l2Start = -1; }
      if (l1Start > 0) groupsL1.push([l1Start + 1, r - 1]);
      l1Start = r;
      sh.getRange(r, 1, 1, ncol).setBackground(L1_BG).setFontWeight('bold').setFontSize(11).setVerticalAlignment('middle');
      sh.setRowHeight(r, 26);
    } else if (kind === 'L2') {
      if (l2Start > 0) groupsL2.push([l2Start + 1, r - 1]);
      l2Start = r;
      sh.getRange(r, 1, 1, ncol).setBackground(L2_BG).setFontWeight('bold').setFontSize(10).setVerticalAlignment('middle');
      sh.setRowHeight(r, 22);
    }
  }
  if (l2Start > 0) groupsL2.push([l2Start + 1, lastRow]);
  if (l1Start > 0) groupsL1.push([l1Start + 1, lastRow]);
  sh.setRowGroupControlPosition(SpreadsheetApp.GroupControlTogglePosition.BEFORE);
  groupsL1.forEach(function (g) { if (g[1] >= g[0]) sh.getRange(g[0], 1, g[1] - g[0] + 1, 1).shiftRowGroupDepth(1); });
  groupsL2.forEach(function (g) { if (g[1] >= g[0]) sh.getRange(g[0], 1, g[1] - g[0] + 1, 1).shiftRowGroupDepth(1); });
  return { l1: groupsL1.length, l2: groupsL2.length };
}
function findHeaderRow_(sh) {
  var v = sh.getRange(1, 1, 10, 1).getValues();
  for (var r = 0; r < v.length; r++) if (String(v[r][0]).trim() === 'WBS') return r + 1;
  return -1;
}
function fixDates_(sh, row, n, c) {
  var rng = sh.getRange(row, c, n, 1);
  rng.setValues(rng.getValues().map(function (r) { var d = toDate_(r[0]); return [d ? d : (r[0] === '' ? '' : r[0])]; }));
}
function toDate_(x) {
  if (x instanceof Date) return isNaN(x.getTime()) ? null : x;
  var m = String(x).trim().match(/^(\d{4})-(\d{2})-(\d{2})$/);
  return m ? new Date(Number(m[1]), Number(m[2]) - 1, Number(m[3])) : null;
}
function mondayOf_(d) {
  var r = new Date(d.getFullYear(), d.getMonth(), d.getDate());
  r.setDate(r.getDate() - ((r.getDay() + 6) % 7));
  return r;
}
function columnLetter_(c) { var s = ''; while (c > 0) { var m = (c - 1) % 26; s = String.fromCharCode(65 + m) + s; c = (c - m - 1) / 26; } return s; }
function fmt_(d) { return Utilities.formatDate(d, Session.getScriptTimeZone(), 'yyyy-MM-dd'); }
function fmt2_(d) { return Utilities.formatDate(d, Session.getScriptTimeZone(), 'MM/dd'); }
function collapseAllGroups_(sh, lastRow) {
  for (var pass = 0; pass < 10; pass++) {
    var removed = 0;
    for (var r = 1; r <= lastRow; r++) {
      var d = sh.getRowGroupDepth(r);
      if (d > 0) { try { sh.getRowGroup(r, d).remove(); removed++; } catch (e) { /* 이미 제거됨 */ } }
    }
    if (removed === 0) break;
  }
}
