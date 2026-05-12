#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <random>
#include <vector>

enum class TileColor {
	Black,
	White
};

enum class Player {
	Computer = 0,
	User = 1
};

enum class TurnPhase {
	Draw,
	Guess,
	ContinueOrPass,
	RevealOwnTile,
	GameOver
};

enum class GuessResult {
	Correct,
	Wrong,
	InvalidMove,
	AlreadyRevealed
};

struct GameTile {
	TileColor color;
	int number;
	bool revealed;
};

struct GuessOutcome {
	GuessResult result;
	TurnPhase phase;
	std::optional<Player> winner;
	bool needsSelfReveal;
};

Player otherPlayer(Player player);

class DavinciGame {
public:
	explicit DavinciGame(unsigned int seed = std::random_device{}());

	void start(int userBlackCount, int userWhiteCount);
	bool draw(TileColor color);
	GuessOutcome guess(Player target, std::size_t tileIndex, int number);
	bool pass();
	bool revealOwnTile(std::size_t tileIndex);

	bool started() const;
	Player currentPlayer() const;
	TurnPhase phase() const;
	std::optional<Player> winner() const;
	std::optional<std::size_t> drawnTileIndex() const;

	const std::vector<GameTile>& stock() const;
	const std::vector<GameTile>& hand(Player player) const;
	bool canDraw(TileColor color) const;

private:
	std::vector<GameTile>& mutableHand(Player player);
	std::size_t drawRandomTile(TileColor color);
	std::size_t insertSorted(Player player, GameTile tile);
	std::optional<Player> findWinner() const;
	void advanceTurn();
	void finishIfGameOver();
	bool hasHiddenTile(Player player) const;

	bool started_;
	Player currentPlayer_;
	TurnPhase phase_;
	std::optional<Player> winner_;
	std::optional<std::size_t> drawnTileIndex_;
	std::vector<GameTile> stock_;
	std::array<std::vector<GameTile>, 2> hands_;
	std::mt19937 rng_;
};
