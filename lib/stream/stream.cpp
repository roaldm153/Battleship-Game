#include <charconv>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "stream.hpp"
#include "game/game.hpp"


bool Stream::TryParseDigit(const char& digit, size_t* dest) {
    if (!(digit >= '0' && digit <= '9')) {
        return false;
    }
    *dest = digit - '0';

    return true;
}

bool Stream::TryParseNumber(const std::string& number, uint64_t* dest) {
    int64_t result;
    std::from_chars_result parse_result = std::from_chars(
                            number.data()
                            , number.data() + number.size()
                            , result
                            );
    if (parse_result.ec == std::errc() && parse_result.ptr == number.data() + number.size()) {
        *dest = result;

        return true;
    }

    return false;
}

void Stream::SendResponse(const std::string& response) {
    std::cout << response << '\n';
}

void Stream::SendResponse(const uint64_t& response) {
    std::cout << response << '\n';
}

void Stream::SendResponse(const Coordinate& response) {
    std::cout << response.x << " " << response.y << '\n';
}

void Stream::SendErrorResponse() {
    std::cerr << "Error: Wrong argument!" << '\n';
}

std::vector<std::string> Stream::SplitString(const std::string& str) {
    std::istringstream iss(str);
    std::string token;
    std::vector<std::string> splited_string;
    while (iss >> token) {
        splited_string.push_back(token);
    }

    return splited_string;
}

signed Stream::WaitForQuery(Game& game) {
    while(true) {
        std::getline(std::cin, query);
        if(query == "exit") {
            break;
        } else if (query == "ping") {
            SendResponse("pong");
        } else if (query == "create master") {
            game.Create(PlayerType::kMaster);
            SendResponse("ok");
        } else if (query == "create slave") {
            game.Create(PlayerType::kSlave);
            SendResponse("ok");
        } else if (query.find("set count") == 0) {
            size_t number_of_ship;
            const size_t kShipPosition = 10;
            const size_t kShipCntPosition = 12;
            char digit = query[kShipPosition];
            std::string str_number = query.substr(kShipCntPosition);
            uint64_t ship_cnt;
            bool result = false;
            if (TryParseDigit(digit, &number_of_ship) && TryParseNumber(str_number, &ship_cnt)) {
                result = game.SetCount(number_of_ship, ship_cnt);
            }
            result ? SendResponse("ok") : SendResponse("failed");
        } else if (query.find("set height") == 0) {
            const size_t kHeightPosition = 11;
            uint64_t number;
            std::string str_number = query.substr(kHeightPosition);
            bool result = false;
            if (TryParseNumber(str_number, &number)) {
                result = game.SetHeight(number);
            }
            result ? SendResponse("ok") : SendResponse("failed");
        } else if (query.find("set width") == 0) {
            const size_t kWidthPosition = 10;
            bool result = false;
            uint64_t number;
            std::string str_number = query.substr(kWidthPosition);
            if (TryParseNumber(str_number, &number)) {
                result = game.SetWidth(number);
            }
            result ? SendResponse("ok") : SendResponse("failed");
        } else if (query == "get height") {
            SendResponse(game.GetHeight());
        } else if (query == "get width") {
            SendResponse(game.GetWidth());
        } else if (query == "start") {
            game.Start();
            SendResponse("ok");
        } else if (query.find("get count") == 0) {
            const size_t kDigitPos = 10;
            size_t number;
            uint64_t result;
            if (TryParseDigit(query[kDigitPos], &number)) {
                result = game.GetCount(number);
            }
            SendResponse(result);
        } else if (query == "print") {
            game.PrintField();
        } else if (query.find("shot") == 0 && query != "shot") {
            const size_t kXpos = 1;
            const size_t kYpos = 2;
            std::vector<std::string> coordinates = SplitString(query);
            if (!(kXpos < coordinates.size() && kYpos < coordinates.size())) {
                SendErrorResponse();
                continue;
            }
            std::string x_coord_str = coordinates[kXpos];
            std::string y_coord_str = coordinates[kYpos];
            uint64_t x_coord;
            uint64_t y_coord;
            if (TryParseNumber(x_coord_str, &x_coord) 
                && TryParseNumber(y_coord_str, &y_coord)) {
                    SendResponse(game.CheckShot(Coordinate(x_coord, y_coord)));
            } else {
                SendErrorResponse();
            }
        } else if (query == "shot") {
            SendResponse(game.SetShot());
        } else if (query == "set strategy ordered") {
            game.SetStrategy(StrategyType::kOrdered);
            SendResponse("ok");
        } else if (query == "set strategy custom") {
            game.SetStrategy(StrategyType::kCustom);
            SendResponse("ok");
        } else if (query.find("set result") == 0 && query != "set result") {
            const size_t kResultPos = 11;
            game.SetShotResult(query.substr(kResultPos));
            SendResponse("ok");
        } else if (query == "win") {
            game.IsWin() ? SendResponse("yes") : SendResponse("no");
        } else if (query == "lose") {
            game.IsLose() ? SendResponse("yes") : SendResponse("no");
        } else if (query == "finished") {
            game.IsFinished() ? SendResponse("yes") : SendResponse("no");
        } else if (query == "stop") {
            game.Stop();
            SendResponse("ok");
        } else if (query.find("load") == 0) {
            const size_t kPathPosition = 5;
            game.Load(query.substr(kPathPosition));
            SendResponse("ok");
        } else if (query.find("dump") == 0) {
            const size_t kPathPosition = 5;
            game.Dump(query.substr(kPathPosition));
            SendResponse("ok");
        } else {
            SendErrorResponse();
        }
    }

    return 0;
}