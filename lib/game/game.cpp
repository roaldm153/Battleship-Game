#include <cstdint>
#include <iostream>
#include <fstream>

#include "game.hpp"


// class Player methods
void Player::SetMaster() {
    type_ = PlayerType::kMaster;
}

bool Player::CheckMaster() {
    return (type_ == PlayerType::kMaster);
}

void Player::AddShip(Ship* ship) {
    for (size_t i = 0; i < ship->array.size(); ++i) {
        ships_[ship->array[i]] = ship;
    }
}

bool Player::CheckCoord(const Coordinate& coord, const Game& game) {
    return ships_.contains(coord);
}

Ship* Player::GetShip(const Coordinate& coord) {
    if (ships_.contains(coord)) {
        return ships_[coord];
    }

    return nullptr;
}

void Player::DeleteCoord(const Coordinate& coord) {
    auto iterator = ships_.find(coord);
    if (iterator != ships_.end()) {
        ships_[coord] = nullptr;
    }
}

void Player::SetShotResult(const ShotResult& result) {
    last_shot_result_ = result;
}

void Player::DumpShips(std::ofstream& file) {
    if (file.is_open()) {
        Ship* last = nullptr;
        for (auto iterator: ships_) {
            Ship* ship = iterator.second;
            if (ship && ship != last) {
                file << ship->array.size() << " ";
                if (ship->is_horizontal) {
                    file << 'h' << " ";
                } else {
                    file << 'v' << " ";
                }
                file << ship->head.x<< " " << ship->head.y << '\n';
                last = ship;
            }
        }
    }
}

// class Game methods
const uint64_t& Game::GetCountUtil(size_t n) const {
    if (n >= 1 && n <= 4) {
        return field_.ships_cnt_[n - 1];
    }

    return field_.ships_cnt_[0];
}

bool Game::SetCountUtil(size_t n, uint64_t value) {
    if (n >= 1 && n <= 4) {
        field_.ships_cnt_[n - 1] = value;

        return true;
    }

    return false;
}

bool Game::SetHeight(uint64_t height) {
    field_.height = height;

    return true;
}

bool Game::SetWidth(uint64_t width) {
    field_.width = width;

    return true;
}

bool Game::CheckCapacityUtil(size_t n, uint64_t value, int64_t height, int64_t width) {
    if (height <= 1 || width <= 1) {
        return std::max(height / (n + 1), width / (n + 1)) >= value;
    }
    int64_t res = (std::max(
                    ((height / (n + 1)) + (width - 1) / (n + 1)) * 2
                    , (width / (n + 1)) + (height - 1) / (n + 1) * 2)
                    );
    if (res >= value) {
        return true;
    }
    if (res == 0 && value > 0) {
        return false;
    }

    return CheckCapacityUtil(n, value - res, height - 4, width - 4);
}

bool Game::SetCount(size_t n, uint64_t value) {
    if (!CheckCapacityUtil(n, value, field_.height, field_.width)) {
        return false;
    }
    
    return SetCountUtil(n, value);
}

const uint64_t& Game::GetHeight() const {
    return field_.height;
}

const uint64_t& Game::GetWidth() const {
    return field_.width;
}

const uint64_t& Game::GetCount(size_t n) const {
    return GetCountUtil(n);
}

bool Game::IsWin() {
    return (current_game_status_ == GameStatus::kWin);
}

bool Game::IsLose() {
    return (current_game_status_ == GameStatus::kLose);
}

bool Game::IsFinished() {
    return (current_game_process_ == GameStatus::kFinished);
}

void Game::Create(const PlayerType& type) {
    delete player_;
    player_ = new Player;
    if (type == PlayerType::kMaster) {
        player_->SetMaster();
        SetDefaultParametersUtil();
    }
}

void Game::SetDefaultParametersUtil() {
    SetWidth(kDefaultWidth);
    SetHeight(kDefaultHeight);
    SetCount(1, kOneDeckDefaultValue);
    SetCount(2, kTwoDeckDefaultValue);
    SetCount(3, kThreeDeckDefaultValue);
    SetCount(4, kFourDeckDefaultValue);
}

void Game::SetStrategy(const StrategyType& strategy_type) {
    delete strategy_;
    if (strategy_type == StrategyType::kCustom) {
        strategy_ = new CustomStrategy;

        return;
    }
    strategy_ = new OrderedStrategy;
}

void Game::Start() {
    if (!player_) {
        return;
    }
    if (!strategy_) {
        SetStrategy(StrategyType::kCustom);
    }
    strategy_->PlaceShips(*this);
    field_.my_ships_alive = GetCount(1) + GetCount(2) + GetCount(3) + GetCount(4);
    field_.enemy_ships_alive = field_.my_ships_alive;
}

void Game::Stop() {
    current_game_process_ = GameStatus::kFinished;
}

void Game::PrintField() const {
    for (int64_t y = 0; y < field_.height; ++y) {
        for (int64_t x = 0; x < field_.width; ++x) {
            if (!player_->CheckCoord(Coordinate(x, y), *this) ) {
                std::cout << 0 << " ";
            } else {
                Ship* ship = player_->GetShip(Coordinate(x, y));
                if (!ship) {
                    std::cout << "*" << " ";
                } else {
                    std::cout << 1 << " ";
                }
            }
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}

std::string Game::CheckShot(const Coordinate& coord) {
    if (!player_) {
        return "miss";
    }
    Ship* ship = player_->GetShip(coord);
    if (!ship) {
        return "miss";
    }

    for (size_t i = 0; i < ship->array.size(); ++i) {
        if (ship->array[i] == coord) {
            player_->DeleteCoord(coord);
            std::swap(ship->array[i], ship->array[ship->array.size() - 1]);
            ship->array.pop_back();
            if (ship->array.empty()) {
                --field_.my_ships_alive;
                if (field_.my_ships_alive <= 0) {
                    current_game_status_ = GameStatus::kLose;
                }
                return "kill";
            }

            return "hit";
        }
    }

    return "miss";
}

const Coordinate& Game::SetShot() {
    return strategy_->ShotUtil(*this);
}

ShotResult Game::SetShotResult(const std::string& result) {
    if (result == "miss") {
        return ShotResult::kMiss;
    } else if (result == "hit") {
        return ShotResult::kHit;
    } else if (result == "kill") {
        --field_.enemy_ships_alive;
        if (field_.enemy_ships_alive <= 0) {
            current_game_status_ = GameStatus::kWin;
        }
        return ShotResult::kKill;
    }

    return ShotResult::kUndefined;
}

void Game::Load(const std::string& path) {
    if (!player_) {
        Create(PlayerType::kSlave);
    }
    std::ifstream file(path);
    uint64_t field_width{};
    uint64_t field_height{};

    if (file.is_open()) {
        file >> field_width >> field_height;
        SetWidth(field_width);
        SetHeight(field_height);
    }

    while (!file.eof()) {
        size_t ship_size;
        char direction;
        uint64_t x_coord;
        uint64_t y_coord;
        file >> ship_size >> direction >> x_coord >> y_coord;
        Ship* ship = new Ship;
        ship->array.push_back(Coordinate(x_coord, y_coord));
        switch (direction) {
            case 'h':
                for (uint64_t i = 1; i < ship_size; ++i) {
                    ship->array.push_back(Coordinate(x_coord + i, y_coord));
                }
                ship->is_horizontal = true;
                ship->head = Coordinate(x_coord, y_coord);
                player_->AddShip(ship);
                break;
            case 'v':
                for (uint64_t i = 1; i < ship_size; ++i) {
                    ship->array.push_back(Coordinate(x_coord, y_coord + i));
                }
                ship->head = Coordinate(x_coord, y_coord);
                player_->AddShip(ship);
                break;
        }
    }

    file.close();
}

void Game::Dump(const std::string& path) {
    if (!player_) {
        Create(PlayerType::kSlave);
    }
    std::ofstream file(path);
    if (file.is_open()) {
        file << GetWidth() << " " << GetHeight() << '\n';
    }
    player_->DumpShips(file);
    file.close();
}

// Strategy methods
bool Strategy::ValidateCell(const Coordinate& coord, const Game& game) {
    if (coord.x >= game.field_.width || coord.y >= game.field_.height) {
        return false;
    }
    if (coord.x < 0 || coord.y < 0) {
        return false;
    }
    if (!game.player_) {
        return false;
    }

    Coordinate cell1(coord.x + 1, coord.y);
    Coordinate cell2(coord.x, coord.y - 1);
    Coordinate cell3(coord.x - 1, coord.y);
    Coordinate cell4(coord.x, coord.y + 1);
    Coordinate cell5(coord.x + 1, coord.y + 1);
    Coordinate cell6(coord.x - 1, coord.y + 1);
    Coordinate cell7(coord.x + 1, coord.y - 1);
    Coordinate cell8(coord.x - 1, coord.y - 1);

    bool is_valid = 
                    !game.player_->CheckCoord(cell1, game) 
                    && !game.player_->CheckCoord(cell2, game)
                    && !game.player_->CheckCoord(cell3, game)
                    && !game.player_->CheckCoord(cell4, game)
                    && !game.player_->CheckCoord(cell5, game)
                    && !game.player_->CheckCoord(cell6, game)
                    && !game.player_->CheckCoord(cell7, game)
                    && !game.player_->CheckCoord(cell8, game);

    return is_valid;
}

bool Strategy::TryPlaceShip(Ship* ship, const Game& game) {
    for (size_t i = 0; i < ship->array.size(); ++i) {
        if (!ValidateCell(ship->array[i], game)) {
            delete ship;
            return false;
        }
    }
    game.player_->AddShip(ship);

    return true;
}

void Strategy::PlaceOneSizeShips(size_t size, int64_t n, const Game& game) {
    while (n > 0) {
        for (int64_t x = 0; x < game.GetWidth(); ++x) {
            for (int64_t y = 0; y < game.GetHeight(); ++y) {
                Coordinate coord(x, y);
                if (ValidateCell(coord, game)) {
                    Ship* ship1 = new Ship;
                    Ship* ship2 = new Ship;
                    ship1->array.push_back(coord);
                    ship1->head = coord;
                    ship2->array.push_back(coord);
                    ship2->head = coord;

                    for (int64_t i = 1; i <= size; ++i) {
                        ship1->array.push_back(Coordinate(x + i, y));
                        ship2->array.push_back(Coordinate(x, y + i));
                    }
                    ship1->is_horizontal = true;

                    if (TryPlaceShip(ship1, game)) {
                        --n;
                    } else if (TryPlaceShip(ship2, game)) {
                        --n;
                    }
                    if (n <= 0) {
                        return;
                    }
                }
            }
        }
    }
}

void Strategy::PlaceShips(const Game& game) {
    const int8_t kFourIndex = 3;
    const int8_t kThreeIndex = 2;
    const int8_t kTwoIndex = 1;
    const int8_t kOneIndex = 0;
    int64_t four_cnt = game.field_.ships_cnt_[kFourIndex];
    int64_t three_cnt = game.field_.ships_cnt_[kThreeIndex];
    int64_t two_cnt = game.field_.ships_cnt_[kTwoIndex];
    int64_t one_cnt = game.field_.ships_cnt_[kOneIndex];
    PlaceOneSizeShips(kFourIndex, four_cnt, game);
    PlaceOneSizeShips(kThreeIndex, three_cnt, game);
    PlaceOneSizeShips(kTwoIndex, two_cnt, game);
    PlaceOneSizeShips(kOneIndex, one_cnt, game);
}

const Coordinate& OrderedStrategy::ShotUtil(const Game& game) {
    if (next_shot_coord_.x + 1 <= game.GetWidth()) {
        ++next_shot_coord_.x;
    } else {
        ++next_shot_coord_.y;
        next_shot_coord_.x = 0;
    }
    if (next_shot_coord_.y > game.GetHeight()) {
        next_shot_coord_.y = 0;
    }

    return next_shot_coord_;
}

const Coordinate& CustomStrategy::ShotUtil(const Game& game) {
    if (next_shot_coord_.y + 1 <= game.GetHeight()) {
        ++next_shot_coord_.y;
    } else {
        ++next_shot_coord_.x;
        next_shot_coord_.y = 0;
    }
    if (next_shot_coord_.x > game.GetWidth()) {
        next_shot_coord_.x = 0;
    }

    return next_shot_coord_;
}