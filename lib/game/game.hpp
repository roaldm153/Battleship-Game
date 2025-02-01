#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>


struct Coordinate {
    int64_t x {0};
    int64_t y {0};

    Coordinate() = default;
    Coordinate(int64_t x, int64_t y): x(x), y(y){}
    bool operator==(const Coordinate& other) const {
        return x == other.x && y == other.y;
    }
};

enum class ShotResult {
    kUndefined = -1,
    kMiss = 0,
    kHit = 1,
    kKill = 2,
};

enum class GameStatus {
    kUndefined = -1,
    kLose = 0,
    kWin = 1,
    kFinished = 2,
};

enum class PlayerType {
    kSlave = 0,
    kMaster = 1,
};

enum class StrategyType {
    kOrdered = 0,
    kCustom = 1,
};

class Game;
class Strategy;

struct Ship {
    std::vector<Coordinate> array;
    bool is_horizontal {false};
    Coordinate head {};
};

struct HashFunction {
    size_t operator()(const Coordinate& coord) const {
        return std::hash<int64_t>()(coord.x) ^ (std::hash<int64_t>()(coord.y) << 1);
    }
};

class Player {
private:
    PlayerType type_{PlayerType::kSlave};
    std::unordered_map<Coordinate, Ship*, HashFunction> ships_;
    ShotResult last_shot_result_ {ShotResult::kUndefined};
public:
    void SetMaster();
    bool CheckMaster();
    bool CheckCoord(const Coordinate&, const Game&);
    void AddShip(Ship*);
    Ship* GetShip(const Coordinate&);
    void DeleteCoord(const Coordinate&);
    void SetShotResult(const ShotResult&);
    const ShotResult& GetShotResult();
    void DumpShips(std::ofstream&);
};

struct Field {
    constexpr static size_t kCntSize {4};
    uint64_t ships_cnt_[kCntSize] {0, 0, 0, 0};
    uint64_t height {0};
    uint64_t width {0};
    uint64_t my_ships_alive {0};
    uint64_t enemy_ships_alive {0};
};

class Strategy {
protected:
    Coordinate next_shot_coord_ {};
    ShotResult last_shot_result {};
public:
    virtual const Coordinate& ShotUtil(const Game&) = 0;
    void PlaceOneSizeShips(size_t, int64_t, const Game&);
    void PlaceShips(const Game&);
    bool TryPlaceShip(Ship*, const Game&);
    bool ValidateCell(const Coordinate&, const Game&);

    virtual ~Strategy() = default;
};

class OrderedStrategy: public Strategy {
    const Coordinate& ShotUtil(const Game&) override;
};

class CustomStrategy: public Strategy {
    const Coordinate& ShotUtil(const Game&) override;
};

class Game {
private:
    const uint64_t kDefaultWidth {10};
    const uint64_t kDefaultHeight {10};
    const uint64_t kOneDeckDefaultValue {1};
    const uint64_t kTwoDeckDefaultValue {1};
    const uint64_t kThreeDeckDefaultValue {1};
    const uint64_t kFourDeckDefaultValue {1};

    Field field_ {};
    Player* player_ {nullptr};
    Strategy* strategy_ {nullptr};
    GameStatus current_game_status_ {GameStatus::kUndefined};
    GameStatus current_game_process_ {GameStatus::kUndefined};

    bool SetCountUtil(size_t, uint64_t);
    const uint64_t& GetCountUtil(size_t) const;
    bool CheckCapacityUtil(size_t, uint64_t, int64_t, int64_t);
    void SetDefaultParametersUtil();
public:
    // control methods
    void PrintField() const;
    void Create(const PlayerType&);
    void Start();
    void Stop();

    // parameters methods
    bool SetHeight(uint64_t);
    bool SetWidth(uint64_t);
    bool SetCount(size_t, uint64_t);
    const uint64_t& GetHeight() const;
    const uint64_t& GetWidth() const;
    const uint64_t& GetCount(size_t) const;
    void Load(const std::string&);
    void Dump(const std::string&);

    // ingame methods
    void SetStrategy(const StrategyType&);
    const Coordinate& SetShot();
    std::string CheckShot(const Coordinate&);
    ShotResult SetShotResult(const std::string&);
    bool IsFinished();
    bool IsWin();
    bool IsLose();

    Game() = default;
    ~Game() {
        delete player_;
        delete strategy_;
    }
    Game& operator=(const Game& other) = delete;
    Game(const Game& other) = delete;

    friend class Strategy;
};