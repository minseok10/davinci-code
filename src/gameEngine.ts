export type TileColor = 'black' | 'white';
export type Player = 'computer' | 'user';
export type TurnPhase = 'draw' | 'guess' | 'continueOrPass' | 'revealOwnTile' | 'gameOver';
export type GuessResult = 'correct' | 'wrong' | 'invalidMove' | 'alreadyRevealed';

export interface GameTile {
  color: TileColor;
  number: number;
  revealed: boolean;
}

export interface GuessOutcome {
  result: GuessResult;
  phase: TurnPhase;
  winner: Player | null;
  needsSelfReveal: boolean;
}

const tileCountPerColor = 12;
const initialHandSize = 4;

export function otherPlayer(player: Player): Player {
  return player === 'computer' ? 'user' : 'computer';
}

function tileComesBefore(left: GameTile, right: GameTile): boolean {
  if (left.number !== right.number) {
    return left.number < right.number;
  }

  return left.color === 'black' && right.color === 'white';
}

function randomIndex(length: number): number {
  return Math.floor(Math.random() * length);
}

export class DavinciGame {
  private startedValue = false;
  private currentPlayerValue: Player = 'computer';
  private phaseValue: TurnPhase = 'draw';
  private winnerValue: Player | null = null;
  private drawnTileIndexValue: number | null = null;
  private stockValue: GameTile[] = [];
  private handsValue: Record<Player, GameTile[]> = {
    computer: [],
    user: [],
  };

  start(userBlackCount: number, userWhiteCount: number): void {
    if (
      userBlackCount < 0 ||
      userWhiteCount < 0 ||
      userBlackCount + userWhiteCount !== initialHandSize
    ) {
      throw new Error('initial user tile counts must add up to 4');
    }

    this.startedValue = true;
    this.currentPlayerValue = 'computer';
    this.phaseValue = 'draw';
    this.winnerValue = null;
    this.drawnTileIndexValue = null;
    this.stockValue = [];
    this.handsValue = {
      computer: [],
      user: [],
    };

    for (let number = 0; number < tileCountPerColor; number += 1) {
      this.stockValue.push({ color: 'black', number, revealed: false });
      this.stockValue.push({ color: 'white', number, revealed: false });
    }

    for (let i = 0; i < userBlackCount; i += 1) {
      this.takeInitialTile('user', 'black');
    }
    for (let i = 0; i < userWhiteCount; i += 1) {
      this.takeInitialTile('user', 'white');
    }
    for (let i = 0; i < initialHandSize; i += 1) {
      const color =
        this.canDraw('black') && this.canDraw('white')
          ? randomIndex(2) === 0
            ? 'black'
            : 'white'
          : this.canDraw('black')
            ? 'black'
            : 'white';
      this.takeInitialTile('computer', color);
    }
  }

  draw(color: TileColor): boolean {
    if (!this.startedValue || this.phaseValue !== 'draw' || !this.canDraw(color)) {
      return false;
    }

    const index = this.drawRandomTile(color);
    const [tile] = this.stockValue.splice(index, 1);
    this.drawnTileIndexValue = this.insertSorted(this.currentPlayerValue, tile);
    this.phaseValue = 'guess';
    return true;
  }

  guess(target: Player, tileIndex: number, number: number): GuessOutcome {
    if (
      !this.startedValue ||
      this.phaseValue === 'gameOver' ||
      this.phaseValue === 'draw' ||
      this.phaseValue === 'revealOwnTile' ||
      target !== otherPlayer(this.currentPlayerValue) ||
      tileIndex < 0 ||
      tileIndex >= this.hand(target).length ||
      number < 0 ||
      number >= tileCountPerColor
    ) {
      return this.outcome('invalidMove', false);
    }

    const targetTile = this.handsValue[target][tileIndex];
    if (targetTile.revealed) {
      return this.outcome('alreadyRevealed', false);
    }

    if (targetTile.number === number) {
      targetTile.revealed = true;
      this.finishIfGameOver();
      if (this.winnerValue === null) {
        this.phaseValue = 'continueOrPass';
      }
      return this.outcome('correct', false);
    }

    if (this.drawnTileIndexValue !== null) {
      const ownHand = this.handsValue[this.currentPlayerValue];
      if (this.drawnTileIndexValue < ownHand.length) {
        ownHand[this.drawnTileIndexValue].revealed = true;
      }
      this.finishIfGameOver();
      if (this.winnerValue === null) {
        this.advanceTurn();
      }
      return this.outcome('wrong', false);
    }

    this.phaseValue = 'revealOwnTile';
    return this.outcome('wrong', true);
  }

  pass(): boolean {
    if (!this.startedValue || this.phaseValue !== 'continueOrPass') {
      return false;
    }

    this.advanceTurn();
    return true;
  }

  revealOwnTile(tileIndex: number): boolean {
    if (
      !this.startedValue ||
      this.phaseValue !== 'revealOwnTile' ||
      tileIndex < 0 ||
      tileIndex >= this.hand(this.currentPlayerValue).length
    ) {
      return false;
    }

    const ownTile = this.handsValue[this.currentPlayerValue][tileIndex];
    if (ownTile.revealed) {
      return false;
    }

    ownTile.revealed = true;
    this.finishIfGameOver();
    if (this.winnerValue === null) {
      this.advanceTurn();
    }
    return true;
  }

  started(): boolean {
    return this.startedValue;
  }

  currentPlayer(): Player {
    return this.currentPlayerValue;
  }

  phase(): TurnPhase {
    return this.phaseValue;
  }

  winner(): Player | null {
    return this.winnerValue;
  }

  drawnTileIndex(): number | null {
    return this.drawnTileIndexValue;
  }

  stock(): GameTile[] {
    return this.stockValue;
  }

  hand(player: Player): GameTile[] {
    return this.handsValue[player];
  }

  canDraw(color: TileColor): boolean {
    return this.stockValue.some((tile) => tile.color === color);
  }

  private takeInitialTile(player: Player, color: TileColor): void {
    const index = this.drawRandomTile(color);
    const [tile] = this.stockValue.splice(index, 1);
    this.insertSorted(player, tile);
  }

  private drawRandomTile(color: TileColor): number {
    const candidates = this.stockValue
      .map((tile, index) => ({ tile, index }))
      .filter(({ tile }) => tile.color === color)
      .map(({ index }) => index);

    if (candidates.length === 0) {
      throw new Error('no tile of the requested color is available');
    }

    return candidates[randomIndex(candidates.length)];
  }

  private insertSorted(player: Player, tile: GameTile): number {
    const hand = this.handsValue[player];
    const index = hand.findIndex((currentTile) => tileComesBefore(tile, currentTile));
    if (index === -1) {
      hand.push(tile);
      return hand.length - 1;
    }

    hand.splice(index, 0, tile);
    return index;
  }

  private outcome(result: GuessResult, needsSelfReveal: boolean): GuessOutcome {
    return {
      result,
      phase: this.phaseValue,
      winner: this.winnerValue,
      needsSelfReveal,
    };
  }

  private findWinner(): Player | null {
    if (this.startedValue && !this.hasHiddenTile('computer')) {
      return 'user';
    }
    if (this.startedValue && !this.hasHiddenTile('user')) {
      return 'computer';
    }
    return null;
  }

  private advanceTurn(): void {
    this.currentPlayerValue = otherPlayer(this.currentPlayerValue);
    this.drawnTileIndexValue = null;
    this.phaseValue = this.stockValue.length === 0 ? 'guess' : 'draw';
  }

  private finishIfGameOver(): void {
    this.winnerValue = this.findWinner();
    if (this.winnerValue !== null) {
      this.phaseValue = 'gameOver';
    }
  }

  private hasHiddenTile(player: Player): boolean {
    return this.hand(player).some((tile) => !tile.revealed);
  }
}
