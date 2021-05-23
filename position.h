#pragma once

#include <string>

// Позиция ячейки. Индексация с нуля.
struct Position {
    int row = 0;
    int col = 0;

    bool operator==(Position rhs) const;
    bool operator<(Position rhs) const;

    bool IsValid() const;
    std::string ToString() const;

    static Position FromString(std::string_view str);

    static const int MAX_ROWS = 16384;
    static const int MAX_COLS = 16384;
    static const Position NONE;
};

struct PositionHasher {
    size_t operator()(Position pos) const {
        return ((((size_t)pos.row<<16)&0xFFFF0000) | ((size_t)pos.col&0x0000FFFF));
    }
};
