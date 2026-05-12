# Da Vinci Code Web

React + Vite로 만든 2인 로컬 플레이용 다빈치 코드 웹 앱입니다.

## 배포 URL

Cloudflare Pages에 배포된 앱은 아래 주소에서 바로 실행할 수 있습니다.

https://davinci-code-web.pages.dev

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

## Cloudflare Pages 배포

이 프로젝트는 정적 React + Vite 앱이므로 Cloudflare Pages에 바로 배포할 수 있습니다.

### Cloudflare 대시보드로 배포

1. GitHub 또는 GitLab 저장소에 이 프로젝트를 푸시합니다.
2. Cloudflare 대시보드에서 **Workers & Pages > Create application > Pages**를 선택합니다.
3. 저장소를 연결하고 아래처럼 설정합니다.

```text
Framework preset: React (Vite)
Build command: npm run build
Build output directory: dist
Root directory: /
```

배포가 끝나면 `*.pages.dev` 주소가 생성됩니다. 이후 `main` 브랜치에 푸시할 때마다 자동으로 다시 배포됩니다.

### CLI로 배포

Cloudflare 계정에 로그인한 뒤 아래 명령을 실행합니다.

```bash
npm run build
npx wrangler pages deploy dist
```

## 주요 파일

```text
src/gameEngine.ts  # 다빈치 코드 게임 상태와 규칙
src/App.tsx        # 2인 로컬 플레이 UI
src/styles.css     # 반응형 화면 스타일
```
