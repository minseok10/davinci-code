#include "game.h"

#include <algorithm>
#include <stdexcept>

namespace {
constexpr int kTileCountPerColor = 12;
constexpr int kInitialHandSize = 4;

std::size_t playerIndex(Player player)
{
	return static_cast<std::size_t>(player);
}

bool tileComesBefore(const GameTile& left, const GameTile& right)
{
	if (left.number != right.number) {
		return left.number < right.number;
	}

	return left.color == TileColor::Black && right.color == TileColor::White;
}
}

Player otherPlayer(Player player)
{
	return player == Player::Computer ? Player::User : Player::Computer;
}

DavinciGame::DavinciGame(unsigned int seed)
	: started_(false),
	  currentPlayer_(Player::Computer),
	  phase_(TurnPhase::Draw),
	  rng_(seed)
{
}

void DavinciGame::start(int userBlackCount, int userWhiteCount)
{
	if (userBlackCount < 0 || userWhiteCount < 0 ||
		userBlackCount + userWhiteCount != kInitialHandSize) {
		throw std::invalid_argument("initial user tile counts must add up to 4");
	}

	started_ = true;
	currentPlayer_ = Player::Computer;
	phase_ = TurnPhase::Draw;
	winner_.reset();
	drawnTileIndex_.reset();
	stock_.clear();
	hands_[playerIndex(Player::Computer)].clear();
	hands_[playerIndex(Player::User)].clear();

	for (int number = 0; number < kTileCountPerColor; ++number) {
		stock_.push_back({ TileColor::Black, number, false });
		stock_.push_back({ TileColor::White, number, false });
	}

	for (int i = 0; i < userBlackCount; ++i) {
		const std::size_t index = drawRandomTile(TileColor::Black);
		insertSorted(Player::User, stock_[index]);
		stock_.erase(stock_.begin() + static_cast<std::ptrdiff_t>(index));
	}
	for (int i = 0; i < userWhiteCount; ++i) {
		const std::size_t index = drawRandomTile(TileColor::White);
		insertSorted(Player::User, stock_[index]);
		stock_.erase(stock_.begin() + static_cast<std::ptrdiff_t>(index));
	}
	for (int i = 0; i < kInitialHandSize; ++i) {
		const TileColor color = canDraw(TileColor::Black) && canDraw(TileColor::White)
			? (std::uniform_int_distribution<int>(0, 1)(rng_) == 0 ? TileColor::Black : TileColor::White)
			: (canDraw(TileColor::Black) ? TileColor::Black : TileColor::White);
		const std::size_t index = drawRandomTile(color);
		insertSorted(Player::Computer, stock_[index]);
		stock_.erase(stock_.begin() + static_cast<std::ptrdiff_t>(index));
	}
}

bool DavinciGame::draw(TileColor color)
{
	if (!started_ || phase_ != TurnPhase::Draw || !canDraw(color)) {
		return false;
	}

	const std::size_t index = drawRandomTile(color);
	GameTile tile = stock_[index];
	stock_.erase(stock_.begin() + static_cast<std::ptrdiff_t>(index));
	drawnTileIndex_ = insertSorted(currentPlayer_, tile);
	phase_ = TurnPhase::Guess;
	return true;
}

GuessOutcome DavinciGame::guess(Player target, std::size_t tileIndex, int number)
{
	if (!started_ || phase_ == TurnPhase::GameOver || phase_ == TurnPhase::Draw ||
		phase_ == TurnPhase::RevealOwnTile || target != otherPlayer(currentPlayer_) ||
		tileIndex >= hand(target).size() || number < 0 || number >= kTileCountPerColor) {
		return { GuessResult::InvalidMove, phase_, winner_, false };
	}

	std::vector<GameTile>& targetHand = mutableHand(target);
	GameTile& targetTile = targetHand[tileIndex];
	if (targetTile.revealed) {
		return { GuessResult::AlreadyRevealed, phase_, winner_, false };
	}

	if (targetTile.number == number) {
		targetTile.revealed = true;
		finishIfGameOver();
		if (phase_ != TurnPhase::GameOver) {
			phase_ = TurnPhase::ContinueOrPass;
		}
		return { GuessResult::Correct, phase_, winner_, false };
	}

	if (drawnTileIndex_.has_value()) {
		std::vector<GameTile>& ownHand = mutableHand(currentPlayer_);
		if (*drawnTileIndex_ < ownHand.size()) {
			ownHand[*drawnTileIndex_].revealed = true;
		}
		finishIfGameOver();
		if (phase_ != TurnPhase::GameOver) {
			advanceTurn();
		}
		return { GuessResult::Wrong, phase_, winner_, false };
	}

	phase_ = TurnPhase::RevealOwnTile;
	return { GuessResult::Wrong, phase_, winner_, true };
}

bool DavinciGame::pass()
{
	if (!started_ || phase_ != TurnPhase::ContinueOrPass) {
		return false;
	}

	advanceTurn();
	return true;
}

bool DavinciGame::revealOwnTile(std::size_t tileIndex)
{
	if (!started_ || phase_ != TurnPhase::RevealOwnTile ||
		tileIndex >= hand(currentPlayer_).size()) {
		return false;
	}

	std::vector<GameTile>& ownHand = mutableHand(currentPlayer_);
	if (ownHand[tileIndex].revealed) {
		return false;
	}

	ownHand[tileIndex].revealed = true;
	finishIfGameOver();
	if (phase_ != TurnPhase::GameOver) {
		advanceTurn();
	}
	return true;
}

bool DavinciGame::started() const
{
	return started_;
}

Player DavinciGame::currentPlayer() const
{
	return currentPlayer_;
}

TurnPhase DavinciGame::phase() const
{
	return phase_;
}

std::optional<Player> DavinciGame::winner() const
{
	return winner_;
}

std::optional<std::size_t> DavinciGame::drawnTileIndex() const
{
	return drawnTileIndex_;
}

const std::vector<GameTile>& DavinciGame::stock() const
{
	return stock_;
}

const std::vector<GameTile>& DavinciGame::hand(Player player) const
{
	return hands_[playerIndex(player)];
}

bool DavinciGame::canDraw(TileColor color) const
{
	return std::any_of(stock_.begin(), stock_.end(), [color](const GameTile& tile) {
		return tile.color == color;
	});
}

std::vector<GameTile>& DavinciGame::mutableHand(Player player)
{
	return hands_[playerIndex(player)];
}

std::size_t DavinciGame::drawRandomTile(TileColor color)
{
	std::vector<std::size_t> candidates;
	for (std::size_t i = 0; i < stock_.size(); ++i) {
		if (stock_[i].color == color) {
			candidates.push_back(i);
		}
	}

	if (candidates.empty()) {
		throw std::logic_error("no tile of the requested color is available");
	}

	const std::size_t pick = std::uniform_int_distribution<std::size_t>(
		0, candidates.size() - 1)(rng_);
	return candidates[pick];
}

std::size_t DavinciGame::insertSorted(Player player, GameTile tile)
{
	std::vector<GameTile>& tiles = mutableHand(player);
	const auto position = std::lower_bound(tiles.begin(), tiles.end(), tile, tileComesBefore);
	const std::size_t index = static_cast<std::size_t>(position - tiles.begin());
	tiles.insert(position, tile);
	return index;
}

std::optional<Player> DavinciGame::findWinner() const
{
	if (started_ && !hasHiddenTile(Player::Computer)) {
		return Player::User;
	}
	if (started_ && !hasHiddenTile(Player::User)) {
		return Player::Computer;
	}
	return std::nullopt;
}

void DavinciGame::advanceTurn()
{
	currentPlayer_ = otherPlayer(currentPlayer_);
	drawnTileIndex_.reset();
	phase_ = stock_.empty() ? TurnPhase::Guess : TurnPhase::Draw;
}

void DavinciGame::finishIfGameOver()
{
	winner_ = findWinner();
	if (winner_.has_value()) {
		phase_ = TurnPhase::GameOver;
	}
}

bool DavinciGame::hasHiddenTile(Player player) const
{
	const std::vector<GameTile>& tiles = hand(player);
	return std::any_of(tiles.begin(), tiles.end(), [](const GameTile& tile) {
		return !tile.revealed;
	});
}
