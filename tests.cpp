#include "game.h"

#include <cassert>
#include <cstddef>
#include <iostream>

namespace {
TileColor firstAvailableColor(const DavinciGame& game)
{
	return game.canDraw(TileColor::Black) ? TileColor::Black : TileColor::White;
}

std::size_t firstHiddenIndex(const DavinciGame& game, Player player)
{
	const auto& hand = game.hand(player);
	for (std::size_t i = 0; i < hand.size(); ++i) {
		if (!hand[i].revealed) {
			return i;
		}
	}
	assert(false && "expected at least one hidden tile");
	return 0;
}

int wrongNumberFor(const GameTile& tile)
{
	return (tile.number + 1) % 12;
}

void assertSorted(const DavinciGame& game, Player player)
{
	const auto& hand = game.hand(player);
	for (std::size_t i = 1; i < hand.size(); ++i) {
		const GameTile& prev = hand[i - 1];
		const GameTile& curr = hand[i];
		assert(prev.number < curr.number ||
			(prev.number == curr.number &&
				prev.color == TileColor::Black &&
				curr.color == TileColor::White));
	}
}

void startCreatesSortedHands()
{
	DavinciGame game(7);
	game.start(2, 2);

	assert(game.started());
	assert(game.stock().size() == 16);
	assert(game.hand(Player::Computer).size() == 4);
	assert(game.hand(Player::User).size() == 4);
	assert(game.currentPlayer() == Player::Computer);
	assert(game.phase() == TurnPhase::Draw);
	assertSorted(game, Player::Computer);
	assertSorted(game, Player::User);
}

void correctGuessCanBePassed()
{
	DavinciGame game(11);
	game.start(2, 2);
	assert(game.draw(firstAvailableColor(game)));

	const std::size_t targetIndex = firstHiddenIndex(game, Player::User);
	const int number = game.hand(Player::User)[targetIndex].number;
	const GuessOutcome outcome = game.guess(Player::User, targetIndex, number);

	assert(outcome.result == GuessResult::Correct);
	assert(game.phase() == TurnPhase::ContinueOrPass);
	assert(game.hand(Player::User)[targetIndex].revealed);
	assert(game.pass());
	assert(game.currentPlayer() == Player::User);
	assert(game.phase() == TurnPhase::Draw);
}

void wrongGuessRevealsDrawnTile()
{
	DavinciGame game(13);
	game.start(2, 2);
	assert(game.draw(firstAvailableColor(game)));

	const std::size_t drawnIndex = *game.drawnTileIndex();
	const std::size_t targetIndex = firstHiddenIndex(game, Player::User);
	const int wrongNumber = wrongNumberFor(game.hand(Player::User)[targetIndex]);
	const GuessOutcome outcome = game.guess(Player::User, targetIndex, wrongNumber);

	assert(outcome.result == GuessResult::Wrong);
	assert(!outcome.needsSelfReveal);
	assert(game.hand(Player::Computer)[drawnIndex].revealed);
	assert(game.currentPlayer() == Player::User);
	assert(game.phase() == TurnPhase::Draw);
}

void revealingEveryOpponentTileEndsGame()
{
	DavinciGame game(17);
	game.start(2, 2);
	assert(game.draw(firstAvailableColor(game)));

	while (!game.winner().has_value()) {
		const std::size_t targetIndex = firstHiddenIndex(game, Player::User);
		const int number = game.hand(Player::User)[targetIndex].number;
		const GuessOutcome outcome = game.guess(Player::User, targetIndex, number);
		assert(outcome.result == GuessResult::Correct);
	}

	assert(game.winner() == Player::Computer);
	assert(game.phase() == TurnPhase::GameOver);
}

void emptyStockWrongGuessRequiresChosenSelfReveal()
{
	DavinciGame game(23);
	game.start(2, 2);

	while (!game.stock().empty() && !game.winner().has_value()) {
		assert(game.phase() == TurnPhase::Draw);
		const Player current = game.currentPlayer();
		const Player target = otherPlayer(current);
		assert(game.draw(firstAvailableColor(game)));
		const std::size_t targetIndex = firstHiddenIndex(game, target);
		const int wrongNumber = wrongNumberFor(game.hand(target)[targetIndex]);
		const GuessOutcome outcome = game.guess(target, targetIndex, wrongNumber);
		assert(outcome.result == GuessResult::Wrong);
	}

	assert(!game.winner().has_value());
	assert(game.stock().empty());
	assert(game.phase() == TurnPhase::Guess);

	const Player current = game.currentPlayer();
	const Player target = otherPlayer(current);
	const std::size_t targetIndex = firstHiddenIndex(game, target);
	const int wrongNumber = wrongNumberFor(game.hand(target)[targetIndex]);
	const GuessOutcome outcome = game.guess(target, targetIndex, wrongNumber);

	assert(outcome.result == GuessResult::Wrong);
	assert(outcome.needsSelfReveal);
	assert(game.phase() == TurnPhase::RevealOwnTile);

	const std::size_t ownIndex = firstHiddenIndex(game, current);
	assert(game.revealOwnTile(ownIndex));
	assert(game.currentPlayer() == target);
	assert(game.phase() == TurnPhase::Guess);
}
}

int main()
{
	startCreatesSortedHands();
	correctGuessCanBePassed();
	wrongGuessRevealsDrawnTile();
	revealingEveryOpponentTileEndsGame();
	emptyStockWrongGuessRequiresChosenSelfReveal();

	std::cout << "All tests passed.\n";
	return 0;
}
