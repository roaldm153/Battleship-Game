#pragma once
#include <string>

#include "game/game.hpp"


class Stream {
public:
    std::string query {};
    std::string response {};
    
    signed WaitForQuery(Game&);
private:
    void SendResponse(const std::string&);
    void SendResponse(const uint64_t&);
    void SendResponse(const Coordinate&);
    void SendErrorResponse();
    bool TryParseNumber(const std::string&, uint64_t*);
    bool TryParseDigit(const char&, uint64_t*);
    std::vector<std::string> SplitString(const std::string&);
};