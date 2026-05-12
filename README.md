# Da Vinci Code Web

React + Vite로 만든 2인 로컬 플레이용 다빈치 코드 웹 앱입니다.

## 기능

- 한 브라우저에서 두 플레이어가 번갈아 플레이합니다.
- 흑/백 타일 뽑기, 상대 타일 추리, 계속 추리, 패스, 자기 타일 공개를 지원합니다.
- 현재 턴, 진행 단계, 남은 덱, 승리 상태를 화면에 표시합니다.
- 모바일과 데스크톱 화면에 맞춰 반응형으로 동작합니다.

## 실행

```bash
npm install
npm run dev
```

기본 개발 서버 주소는 다음과 같습니다.

```text
http://127.0.0.1:5173/
```

## 빌드

```bash
npm run build
```

## 주요 파일

```text
src/gameEngine.ts  # 다빈치 코드 게임 상태와 규칙
src/App.tsx        # 2인 로컬 플레이 UI
src/styles.css     # 반응형 화면 스타일
```
