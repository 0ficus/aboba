#pragma once

#include <set>
#include <string>
#include <vector>
#include <tuple>

class Minesweeper {
public:
    struct Cell {
        size_t x = 0;
        size_t y = 0;

        bool operator<(const Cell& cell) const {
            return std::tie(x, y) < std::tie(cell.x, cell.y);
        }
    };

    enum class GameStatus {
        NOT_STARTED,
        IN_PROGRESS,
        VICTORY,
        DEFEAT,
    };

    enum class Situation {
        OPEN_NEIGHBOURS,
        NOTHING,
        OPEN_ALL_CELLS,
        OPEN_THIS_CELL,
    };

    using RenderedField = std::vector<std::string>;

    Minesweeper(size_t width, size_t height, size_t mines_count);
    Minesweeper(size_t width, size_t height, const std::vector<Cell>& cells_with_mines);

    void NewGame(size_t width, size_t height, size_t mines_count);
    void NewGame(size_t width, size_t height, const std::vector<Cell>& cells_with_mines);

    void OpenCell(const Cell& cell);
    void MarkCell(const Cell& cell);

    GameStatus GetGameStatus() const;
    time_t GetGameTime() const;

    RenderedField RenderField() const;

private:
    size_t width_;
    size_t height_;
    time_t start_time_;
    time_t finish_time_;
    size_t mines_count_;
    GameStatus game_status_ = GameStatus::NOT_STARTED;
    std::set<Cell> cells_with_mines_;
    std::set<Cell> cells_with_flags_;
    std::vector<std::vector<char>> field_;
};
