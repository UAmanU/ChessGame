#include "chess/opening_book.h"
#include <algorithm>

namespace {
    struct BookEntry {
        std::vector<Move> position_history;
        Move reply;
    };

    const std::vector<BookEntry>& entries() {
        static const std::vector<BookEntry> book{
            // Mainline responses to common first moves.
            {{Move{6, 4, 4, 4}}, Move{1, 4, 3, 4}}, // 1. e4 e5
            {{Move{6, 3, 4, 3}}, Move{1, 3, 3, 3}}, // 1. d4 d5
            {{Move{7, 2, 5, 2}}, Move{1, 4, 3, 4}}, // 1. c4 e5
            {{Move{7, 6, 5, 5}}, Move{1, 3, 3, 3}}, // 1. Nf3 d5

            // Open games: develop a knight and contest the center.
            {{Move{6, 4, 4, 4}, Move{1, 4, 3, 4}, Move{7, 6, 5, 5}}, Move{0, 1, 2, 2}}, // 1. e4 e5 2. Nf3 Nc6
            {{Move{6, 4, 4, 4}, Move{1, 4, 3, 4}, Move{7, 5, 4, 2}}, Move{0, 1, 2, 2}}, // 1. e4 e5 2. Bc4 Nc6
            {{Move{6, 4, 4, 4}, Move{1, 4, 3, 4}, Move{6, 3, 4, 3}}, Move{3, 4, 4, 3}}, // 1. e4 e5 2. d4 exd4
            {{Move{6, 3, 4, 3}, Move{1, 3, 3, 3}, Move{7, 1, 5, 2}}, Move{0, 6, 2, 5}}, // 1. d4 d5 2. Nc3 Nf6
            {{Move{6, 3, 4, 3}, Move{1, 3, 3, 3}, Move{7, 6, 5, 5}}, Move{0, 6, 2, 5}}, // 1. d4 d5 2. Nf3 Nf6
            {{Move{7, 2, 5, 2}, Move{1, 4, 3, 4}, Move{7, 6, 5, 5}}, Move{0, 1, 2, 2}}  // 1. c4 e5 2. Nf3 Nc6
        };
        return book;
    }
}

std::optional<Move> OpeningBook::find_move(const std::vector<Move>& history, const std::vector<Move>& legal_moves) {
    for (const BookEntry& entry : entries()) {
        if (entry.position_history != history) continue;
        const auto legal = std::find(legal_moves.begin(), legal_moves.end(), entry.reply);
        if (legal != legal_moves.end()) return *legal;
    }
    return std::nullopt;
}
