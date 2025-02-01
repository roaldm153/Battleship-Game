#include "stream/stream.hpp"

int main() {
    Game game;
    Stream stream;
    stream.WaitForQuery(game);

    return 0;
}