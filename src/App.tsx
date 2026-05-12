import { useEffect, useMemo, useRef, useState } from 'react';
import {
  DavinciGame,
  GameTile,
  GuessResult,
  Player,
  TileColor,
  TurnPhase,
} from './gameEngine';

const players: Player[] = ['computer', 'user'];
const computerPlayer: Player = 'computer';
const humanPlayer: Player = 'user';

const playerLabel: Record<Player, string> = {
  computer: 'PC',
  user: '나',
};

const phaseLabel: Record<TurnPhase, string> = {
  draw: '타일 뽑기',
  guess: '추리',
  continueOrPass: '계속 또는 패스',
  revealOwnTile: '자기 타일 공개',
  gameOver: '게임 종료',
};

const resultLabel: Record<GuessResult, string> = {
  correct: '정답',
  wrong: '오답',
  invalidMove: '잘못된 선택',
  alreadyRevealed: '이미 공개됨',
};

function colorLabel(color: TileColor): string {
  return color === 'black' ? '흑' : '백';
}

function winnerMessage(winner: Player): string {
  return `${playerLabel[winner]} 승리`;
}

function countStock(game: DavinciGame, color: TileColor): number {
  return game.stock().filter((tile) => tile.color === color).length;
}

function firstHiddenIndex(hand: GameTile[]): number {
  return hand.findIndex((tile) => !tile.revealed);
}

function clampNumber(value: number, min: number, max: number): number {
  if (!Number.isFinite(value)) {
    return min;
  }

  return Math.min(max, Math.max(min, Math.trunc(value)));
}

function randomItem<T>(items: T[]): T {
  return items[Math.floor(Math.random() * items.length)];
}

function tileOrder(tile: Pick<GameTile, 'color' | 'number'>): number {
  return tile.number * 2 + (tile.color === 'black' ? 0 : 1);
}

function chooseComputerDrawColor(game: DavinciGame): TileColor {
  const canDrawBlack = game.canDraw('black');
  const canDrawWhite = game.canDraw('white');

  if (canDrawBlack && canDrawWhite) {
    return Math.random() < 0.5 ? 'black' : 'white';
  }

  return canDrawBlack ? 'black' : 'white';
}

function chooseComputerTargetIndex(game: DavinciGame): number | null {
  const hiddenIndexes = game
    .hand(humanPlayer)
    .map((tile, index) => ({ tile, index }))
    .filter(({ tile }) => !tile.revealed)
    .map(({ index }) => index);

  return hiddenIndexes.length === 0 ? null : randomItem(hiddenIndexes);
}

function chooseComputerGuessNumber(game: DavinciGame, targetIndex: number): number {
  const targetTile = game.hand(humanPlayer)[targetIndex];
  const knownTiles = [
    ...game.hand(computerPlayer),
    ...game.hand(humanPlayer).filter((tile) => tile.revealed),
  ];

  const candidates = Array.from({ length: 12 }, (_, number) => number).filter((number) => {
    const candidate = { color: targetTile.color, number };
    const candidateOrder = tileOrder(candidate);
    const alreadyKnown = knownTiles.some(
      (tile) => tile.color === targetTile.color && tile.number === number,
    );
    const beforeRevealedNext = game
      .hand(humanPlayer)
      .slice(targetIndex + 1)
      .filter((tile) => tile.revealed)
      .every((tile) => candidateOrder < tileOrder(tile));
    const afterRevealedPrevious = game
      .hand(humanPlayer)
      .slice(0, targetIndex)
      .filter((tile) => tile.revealed)
      .every((tile) => candidateOrder > tileOrder(tile));

    return !alreadyKnown && beforeRevealedNext && afterRevealedPrevious;
  });

  return candidates.length === 0 ? Math.floor(Math.random() * 12) : randomItem(candidates);
}

function chooseComputerSelfRevealIndex(game: DavinciGame): number | null {
  const index = game.hand(computerPlayer).findIndex((tile) => !tile.revealed);
  return index === -1 ? null : index;
}

function App() {
  const gameRef = useRef(new DavinciGame());
  const [version, setVersion] = useState(0);
  const [userBlackCount, setUserBlackCount] = useState(2);
  const [selectedTileIndex, setSelectedTileIndex] = useState<number | null>(null);
  const [guessNumber, setGuessNumber] = useState(0);
  const [message, setMessage] = useState('새 게임을 시작하세요.');

  const game = gameRef.current;
  const currentPlayer = game.currentPlayer();
  const started = game.started();
  const isComputerTurn = started && currentPlayer === computerPlayer;
  const safeUserBlackCount = clampNumber(userBlackCount, 0, 4);
  const userWhiteCount = 4 - safeUserBlackCount;

  const visibleVersion = version;
  const blackStock = useMemo(() => countStock(game, 'black'), [game, visibleVersion]);
  const whiteStock = useMemo(() => countStock(game, 'white'), [game, visibleVersion]);

  function refresh() {
    setVersion((value) => value + 1);
  }

  function runComputerStep() {
    const activeGame = gameRef.current;

    if (
      !activeGame.started() ||
      activeGame.currentPlayer() !== computerPlayer ||
      activeGame.phase() === 'gameOver'
    ) {
      return;
    }

    if (activeGame.phase() === 'draw') {
      const color = chooseComputerDrawColor(activeGame);
      activeGame.draw(color);
      setMessage(`PC가 ${colorLabel(color)} 타일을 뽑았습니다.`);
      refresh();
      return;
    }

    if (activeGame.phase() === 'guess') {
      const targetIndex = chooseComputerTargetIndex(activeGame);
      if (targetIndex === null) {
        setMessage('PC가 추리할 타일을 찾지 못했습니다.');
        refresh();
        return;
      }

      const guess = chooseComputerGuessNumber(activeGame, targetIndex);
      const outcome = activeGame.guess(humanPlayer, targetIndex, guess);

      if (activeGame.winner()) {
        setMessage(winnerMessage(activeGame.winner()!));
      } else {
        setMessage(
          `PC ${resultLabel[outcome.result]}: 내 ${targetIndex + 1}번째 ${colorLabel(
            activeGame.hand(humanPlayer)[targetIndex].color,
          )} 타일을 ${guess}로 추리했습니다.`,
        );
      }
      refresh();
      return;
    }

    if (activeGame.phase() === 'continueOrPass') {
      const hiddenHumanTileCount = activeGame.hand(humanPlayer).filter((tile) => !tile.revealed).length;
      const shouldContinue = hiddenHumanTileCount <= 1;

      if (shouldContinue && activeGame.continueGuess()) {
        setMessage('PC가 계속 추리합니다.');
      } else if (activeGame.pass()) {
        setMessage('내 차례입니다.');
      }
      refresh();
      return;
    }

    if (activeGame.phase() === 'revealOwnTile') {
      const revealIndex = chooseComputerSelfRevealIndex(activeGame);
      if (revealIndex !== null) {
        activeGame.revealOwnTile(revealIndex);
        setMessage(activeGame.winner() ? winnerMessage(activeGame.winner()!) : 'PC가 자기 타일 하나를 공개했습니다. 내 차례입니다.');
      }
      refresh();
    }
  }

  useEffect(() => {
    if (!isComputerTurn || game.phase() === 'gameOver') {
      return undefined;
    }

    const timerId = window.setTimeout(runComputerStep, 650);
    return () => window.clearTimeout(timerId);
  }, [isComputerTurn, version]);

  function startGame() {
    gameRef.current = new DavinciGame();
    gameRef.current.start(safeUserBlackCount, userWhiteCount);
    setUserBlackCount(safeUserBlackCount);
    setSelectedTileIndex(null);
    setGuessNumber(0);
    setMessage('PC가 먼저 시작합니다.');
    refresh();
  }

  function drawTile(color: TileColor) {
    if (game.currentPlayer() !== humanPlayer) {
      return;
    }

    const ok = game.draw(color);
    if (ok) {
      const drawnIndex = game.drawnTileIndex();
      const drawnTile = drawnIndex === null ? null : game.hand(humanPlayer)[drawnIndex];
      setMessage(
        `내가 ${drawnTile ? colorLabel(drawnTile.color) + drawnTile.number : colorLabel(color)} 타일을 뽑았습니다.`,
      );
      setSelectedTileIndex(firstHiddenIndex(game.hand(computerPlayer)));
    } else {
      setMessage('지금은 그 색의 타일을 뽑을 수 없습니다.');
    }
    refresh();
  }

  function submitGuess() {
    if (game.currentPlayer() !== humanPlayer) {
      return;
    }

    if (selectedTileIndex === null) {
      setMessage('추리할 PC 타일을 선택하세요.');
      return;
    }

    const safeGuessNumber = clampNumber(guessNumber, 0, 11);
    setGuessNumber(safeGuessNumber);

    const outcome = game.guess(computerPlayer, selectedTileIndex, safeGuessNumber);
    const baseMessage = `${resultLabel[outcome.result]}: PC의 ${selectedTileIndex + 1}번째 타일을 ${safeGuessNumber}로 추리했습니다.`;
    setMessage(game.winner() ? winnerMessage(game.winner()!) : baseMessage);

    if (outcome.result === 'correct' && !game.winner()) {
      setSelectedTileIndex(firstHiddenIndex(game.hand(computerPlayer)));
    }
    if (game.winner()) {
      setSelectedTileIndex(null);
    }
    if (outcome.result === 'wrong') {
      setSelectedTileIndex(null);
    }
    refresh();
  }

  function continueGuessing() {
    if (game.currentPlayer() !== humanPlayer) {
      return;
    }

    if (game.continueGuess()) {
      setSelectedTileIndex(firstHiddenIndex(game.hand(computerPlayer)));
      setMessage('내가 계속 추리합니다.');
    }
    refresh();
  }

  function passTurn() {
    if (game.currentPlayer() !== humanPlayer) {
      return;
    }

    if (game.pass()) {
      setSelectedTileIndex(null);
      setMessage('PC 차례입니다.');
    }
    refresh();
  }

  function revealOwnTile(index: number) {
    if (game.currentPlayer() !== humanPlayer) {
      return;
    }

    if (game.revealOwnTile(index)) {
      setMessage(game.winner() ? winnerMessage(game.winner()!) : 'PC 차례입니다.');
      setSelectedTileIndex(null);
    } else {
      setMessage('공개할 수 없는 타일입니다.');
    }
    refresh();
  }

  return (
    <main className="app-shell">
      <section className="top-bar">
        <div>
          <p className="eyebrow">Da Vinci Code</p>
          <h1>다빈치 코드</h1>
        </div>
        <button className="secondary-button" type="button" onClick={startGame}>
          새 게임
        </button>
      </section>

      {!started ? (
        <section className="setup-panel">
          <div>
            <h2>게임 설정</h2>
            <p>내 시작 패를 정하면 PC와 바로 대전합니다.</p>
          </div>
          <div className="setup-controls">
            <label>
              흑
              <input
                min="0"
                max="4"
                type="number"
                value={userBlackCount}
                onChange={(event) => setUserBlackCount(clampNumber(Number(event.target.value), 0, 4))}
              />
            </label>
            <label>
              백
              <input readOnly type="number" value={userWhiteCount} />
            </label>
            <button type="button" onClick={startGame}>
              시작
            </button>
          </div>
        </section>
      ) : (
        <>
          <section className="status-band">
            <div>
              <span>현재 턴</span>
              <strong>{playerLabel[currentPlayer]}</strong>
            </div>
            <div>
              <span>단계</span>
              <strong>{phaseLabel[game.phase()]}</strong>
            </div>
            <div>
              <span>남은 덱</span>
              <strong>
                흑 {blackStock} · 백 {whiteStock}
              </strong>
            </div>
            <div>
              <span>결과</span>
              <strong>{game.winner() ? winnerMessage(game.winner()!) : '진행 중'}</strong>
            </div>
          </section>

          <section className="board">
            {players.map((player) => (
              <PlayerHand
                key={player}
                currentPlayer={currentPlayer}
                owner={player}
                selectedTileIndex={player === computerPlayer ? selectedTileIndex : null}
                tiles={game.hand(player)}
                onSelectTarget={(index) => {
                  if (
                    currentPlayer === humanPlayer &&
                    player === computerPlayer &&
                    game.phase() === 'guess' &&
                    !game.hand(player)[index].revealed
                  ) {
                    setSelectedTileIndex(index);
                  }
                }}
                onRevealOwnTile={(index) => {
                  if (currentPlayer === humanPlayer && player === humanPlayer && game.phase() === 'revealOwnTile') {
                    revealOwnTile(index);
                  }
                }}
              />
            ))}
          </section>

          <section className="action-panel">
            <div className="message-line">{message}</div>
            {isComputerTurn && game.phase() !== 'gameOver' && (
              <div className="computer-panel">
                <strong>PC 진행 중</strong>
                <span>잠시 후 자동으로 다음 행동을 합니다.</span>
              </div>
            )}

            {!isComputerTurn && game.phase() === 'draw' && (
              <div className="button-row">
                <button type="button" disabled={!game.canDraw('black')} onClick={() => drawTile('black')}>
                  흑 타일 뽑기
                </button>
                <button type="button" disabled={!game.canDraw('white')} onClick={() => drawTile('white')}>
                  백 타일 뽑기
                </button>
              </div>
            )}

            {!isComputerTurn && game.phase() === 'guess' && (
              <div className="guess-panel">
                <label>
                  PC 타일
                  <select
                    value={selectedTileIndex ?? ''}
                    onChange={(event) => setSelectedTileIndex(Number(event.target.value))}
                  >
                    <option value="" disabled>
                      선택
                    </option>
                    {game.hand(computerPlayer).map((tile, index) => (
                      <option key={`${tile.color}-${tile.number}-${index}`} disabled={tile.revealed} value={index}>
                        {index + 1}번째 · {colorLabel(tile.color)}
                      </option>
                    ))}
                  </select>
                </label>
                <label>
                  숫자
                  <input
                    min="0"
                    max="11"
                    type="number"
                    value={guessNumber}
                    onChange={(event) => setGuessNumber(clampNumber(Number(event.target.value), 0, 11))}
                  />
                </label>
                <button type="button" onClick={submitGuess}>
                  추리
                </button>
              </div>
            )}

            {!isComputerTurn && game.phase() === 'continueOrPass' && (
              <div className="choice-panel">
                <button type="button" onClick={continueGuessing}>
                  계속 추리
                </button>
                <button className="danger-button" type="button" onClick={passTurn}>
                  패스
                </button>
              </div>
            )}

            {!isComputerTurn && game.phase() === 'revealOwnTile' && (
              <div className="reveal-panel">
                <strong>내 차례</strong>
                <span>공개할 자기 타일을 선택하세요.</span>
              </div>
            )}

            {game.phase() === 'gameOver' && (
              <div className="game-over">
                <strong>{winnerMessage(game.winner()!)}</strong>
                <button type="button" onClick={startGame}>
                  다시 시작
                </button>
              </div>
            )}
          </section>
        </>
      )}
    </main>
  );
}

interface PlayerHandProps {
  currentPlayer: Player;
  owner: Player;
  selectedTileIndex: number | null;
  tiles: GameTile[];
  onSelectTarget: (index: number) => void;
  onRevealOwnTile: (index: number) => void;
}

function PlayerHand({
  currentPlayer,
  owner,
  selectedTileIndex,
  tiles,
  onSelectTarget,
  onRevealOwnTile,
}: PlayerHandProps) {
  const isCurrent = owner === currentPlayer;
  const isHuman = owner === humanPlayer;

  return (
    <section className={`hand-section ${isCurrent ? 'active-hand' : ''}`}>
      <div className="hand-heading">
        <h2>{playerLabel[owner]}</h2>
        <span>{isCurrent ? '현재 차례' : isHuman ? '내 패' : '상대'}</span>
      </div>
      <div className="tile-row">
        {tiles.map((tile, index) => {
          const canSeeNumber = isHuman || tile.revealed;
          const isSelected = selectedTileIndex === index;
          return (
            <button
              className={`tile ${tile.color} ${tile.revealed ? 'revealed' : ''} ${isSelected ? 'selected' : ''}`}
              key={`${tile.color}-${tile.number}-${index}`}
              type="button"
              onClick={() => {
                if (isCurrent) {
                  onRevealOwnTile(index);
                } else {
                  onSelectTarget(index);
                }
              }}
            >
              <span className="tile-index">{index + 1}</span>
              <span className="tile-color">{colorLabel(tile.color)}</span>
              <strong>{canSeeNumber ? tile.number : '?'}</strong>
            </button>
          );
        })}
      </div>
    </section>
  );
}

export default App;
