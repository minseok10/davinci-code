import { useMemo, useRef, useState } from 'react';
import {
  DavinciGame,
  GameTile,
  GuessResult,
  Player,
  TileColor,
  TurnPhase,
  otherPlayer,
} from './gameEngine';

const players: Player[] = ['computer', 'user'];

const playerLabel: Record<Player, string> = {
  computer: '플레이어 1',
  user: '플레이어 2',
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

function App() {
  const gameRef = useRef(new DavinciGame());
  const [version, setVersion] = useState(0);
  const [userBlackCount, setUserBlackCount] = useState(2);
  const [selectedTileIndex, setSelectedTileIndex] = useState<number | null>(null);
  const [guessNumber, setGuessNumber] = useState(0);
  const [message, setMessage] = useState('새 게임을 시작하세요.');

  const game = gameRef.current;
  const currentPlayer = game.currentPlayer();
  const targetPlayer = otherPlayer(currentPlayer);
  const started = game.started();
  const safeUserBlackCount = clampNumber(userBlackCount, 0, 4);
  const userWhiteCount = 4 - safeUserBlackCount;

  const visibleVersion = version;
  const blackStock = useMemo(() => countStock(game, 'black'), [game, visibleVersion]);
  const whiteStock = useMemo(() => countStock(game, 'white'), [game, visibleVersion]);

  function refresh() {
    setVersion((value) => value + 1);
  }

  function startGame() {
    gameRef.current = new DavinciGame();
    gameRef.current.start(safeUserBlackCount, userWhiteCount);
    setUserBlackCount(safeUserBlackCount);
    setSelectedTileIndex(null);
    setGuessNumber(0);
    setMessage('플레이어 1의 차례입니다.');
    refresh();
  }

  function drawTile(color: TileColor) {
    const ok = game.draw(color);
    if (ok) {
      const drawnIndex = game.drawnTileIndex();
      const drawnTile = drawnIndex === null ? null : game.hand(currentPlayer)[drawnIndex];
      setMessage(
        `${playerLabel[currentPlayer]}가 ${drawnTile ? colorLabel(drawnTile.color) + drawnTile.number : colorLabel(color)} 타일을 뽑았습니다.`,
      );
      setSelectedTileIndex(firstHiddenIndex(game.hand(targetPlayer)));
    } else {
      setMessage('지금은 그 색의 타일을 뽑을 수 없습니다.');
    }
    refresh();
  }

  function submitGuess() {
    if (selectedTileIndex === null) {
      setMessage('추리할 상대 타일을 선택하세요.');
      return;
    }

    const safeGuessNumber = clampNumber(guessNumber, 0, 11);
    setGuessNumber(safeGuessNumber);

    const outcome = game.guess(targetPlayer, selectedTileIndex, safeGuessNumber);
    const baseMessage = `${resultLabel[outcome.result]}: ${playerLabel[targetPlayer]}의 ${selectedTileIndex + 1}번째 타일을 ${safeGuessNumber}로 추리했습니다.`;
    setMessage(game.winner() ? winnerMessage(game.winner()!) : baseMessage);

    if (outcome.result === 'correct' && !game.winner()) {
      setSelectedTileIndex(firstHiddenIndex(game.hand(targetPlayer)));
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
    if (game.continueGuess()) {
      setSelectedTileIndex(firstHiddenIndex(game.hand(targetPlayer)));
      setMessage(`${playerLabel[currentPlayer]}가 계속 추리합니다.`);
    }
    refresh();
  }

  function passTurn() {
    if (game.pass()) {
      setSelectedTileIndex(null);
      setMessage(`${playerLabel[game.currentPlayer()]}의 차례입니다.`);
    }
    refresh();
  }

  function revealOwnTile(index: number) {
    if (game.revealOwnTile(index)) {
      setMessage(game.winner() ? winnerMessage(game.winner()!) : `${playerLabel[game.currentPlayer()]}의 차례입니다.`);
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
            <p>플레이어 2 시작 패</p>
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
                selectedTileIndex={player === targetPlayer ? selectedTileIndex : null}
                tiles={game.hand(player)}
                onSelectTarget={(index) => {
                  if (
                    player === targetPlayer &&
                    game.phase() === 'guess' &&
                    !game.hand(player)[index].revealed
                  ) {
                    setSelectedTileIndex(index);
                  }
                }}
                onRevealOwnTile={(index) => {
                  if (player === currentPlayer && game.phase() === 'revealOwnTile') {
                    revealOwnTile(index);
                  }
                }}
              />
            ))}
          </section>

          <section className="action-panel">
            <div className="message-line">{message}</div>
            {game.phase() === 'draw' && (
              <div className="button-row">
                <button type="button" disabled={!game.canDraw('black')} onClick={() => drawTile('black')}>
                  흑 타일 뽑기
                </button>
                <button type="button" disabled={!game.canDraw('white')} onClick={() => drawTile('white')}>
                  백 타일 뽑기
                </button>
              </div>
            )}

            {game.phase() === 'guess' && (
              <div className="guess-panel">
                <label>
                  상대 타일
                  <select
                    value={selectedTileIndex ?? ''}
                    onChange={(event) => setSelectedTileIndex(Number(event.target.value))}
                  >
                    <option value="" disabled>
                      선택
                    </option>
                    {game.hand(targetPlayer).map((tile, index) => (
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

            {game.phase() === 'continueOrPass' && (
              <div className="choice-panel">
                <button type="button" onClick={continueGuessing}>
                  계속 추리
                </button>
                <button className="danger-button" type="button" onClick={passTurn}>
                  패스
                </button>
              </div>
            )}

            {game.phase() === 'revealOwnTile' && (
              <div className="reveal-panel">
                <strong>{playerLabel[currentPlayer]}</strong>
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

  return (
    <section className={`hand-section ${isCurrent ? 'active-hand' : ''}`}>
      <div className="hand-heading">
        <h2>{playerLabel[owner]}</h2>
        <span>{isCurrent ? '현재 차례' : '상대'}</span>
      </div>
      <div className="tile-row">
        {tiles.map((tile, index) => {
          const canSeeNumber = isCurrent || tile.revealed;
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
