#include "minesweeper.h"

#include <random>
#include <queue>

std::vector<std::vector<char>> ClearField(size_t width, size_t height) {
    std::vector<std::vector<char>> field(height, std::vector<char>(width, '-'));
    return field;
}

void PlacementMines(size_t mines_count, size_t height, size_t width, std::set<Minesweeper::Cell> &cells_with_mines) {

    bool flag = (height * width - mines_count < mines_count - 1);
    std::vector<size_t> cells(height * width);
    for (size_t i = 0; i < cells.size(); ++i) {
        cells[i] = i;
        if (flag) {
            size_t x = i % width;
            size_t y = i / width;
            Minesweeper::Cell new_cell;
            new_cell.x = x;
            new_cell.y = y;
            cells_with_mines.insert(new_cell);
        }
    }

    for (size_t i = 0; i < std::min(mines_count - 1, height * width - mines_count); ++i) {
        std::mt19937 rand_cell_method = std::mt19937(std::random_device()());
        size_t rand_cell = (rand_cell_method() % (height * width - i - 1) + i + 1);
        std::swap(cells[i], cells[rand_cell]);
        size_t x = rand_cell % width;
        size_t y = rand_cell / width;
        Minesweeper::Cell cell;
        cell.x = x;
        cell.y = y;
        if (mines_count - 1 < height * width - mines_count) {
            cells_with_mines.insert(cell);
        } else {
            cells_with_mines.erase(cell);
        }
    }
}

Minesweeper::Minesweeper(size_t width, size_t height, size_t mines_count) {
    width_ = width;
    height_ = height;
    mines_count_ = mines_count;
    game_status_ = GameStatus::NOT_STARTED;
    PlacementMines(mines_count_, height_, width_, cells_with_mines_);
    cells_with_flags_.clear();
    field_ = ClearField(width_, height_);
}

Minesweeper::Minesweeper(size_t width, size_t height, const std::vector<Cell> &cells_with_mines) {
    width_ = width;
    height_ = height;
    cells_with_mines_.clear();
    game_status_ = GameStatus::NOT_STARTED;
    for (size_t i = 0; i < cells_with_mines.size(); ++i) {
        cells_with_mines_.insert(cells_with_mines[i]);
    }
    cells_with_flags_.clear();
    field_ = ClearField(width_, height_);
}

void Minesweeper::NewGame(size_t width, size_t height, size_t mines_count) {
    width_ = width;
    height_ = height;
    mines_count_ = mines_count;
    game_status_ = GameStatus::NOT_STARTED;
    PlacementMines(mines_count_, height_, width_, cells_with_mines_);
    cells_with_flags_.clear();
    field_ = ClearField(width_, height_);
}

void Minesweeper::NewGame(size_t width, size_t height, const std::vector<Cell> &cells_with_mines) {
    width_ = width;
    height_ = height;
    cells_with_mines_.clear();
    game_status_ = GameStatus::NOT_STARTED;
    for (size_t i = 0; i < cells_with_mines.size(); ++i) {
        cells_with_mines_.insert(cells_with_mines[i]);
    }
    cells_with_flags_.clear();
    field_ = ClearField(width_, height_);
}

bool IsExist(const Minesweeper::Cell &cell, const std::pair<int8_t, int8_t> &move, const size_t height,
             const size_t width) {
    return !(0 == cell.x && move.first == -1) && cell.x + move.first < width && !(0 == cell.y && move.second == -1) &&
           cell.y + move.second < height;
}

std::vector<std::pair<int8_t, int8_t>> GetMovesToNeighbour() {

    std::vector<std::pair<int8_t, int8_t>> moves_to_neighbour;
    for (int8_t i = -1; i < 2; ++i) {
        for (int8_t j = -1; j < 2; ++j) {
            std::pair<int8_t, int8_t> move = {i, j};
            if (i != 0 || j != 0) {
                moves_to_neighbour.emplace_back(move);
            }
        }
    }

    return moves_to_neighbour;
}

bool FindMinesInNeighbourCells(const Minesweeper::Cell &cell, const size_t height, const size_t width,
                               const std::set<Minesweeper::Cell> &cells_with_mines, size_t &cnt_mines) {

    std::vector<std::pair<int8_t, int8_t>> moves_to_neighbour = GetMovesToNeighbour();

    Minesweeper::Cell neighbour_cell;
    for (size_t i = 0; i < moves_to_neighbour.size(); ++i) {
        if (IsExist(cell, moves_to_neighbour[i], height, width)) {
            neighbour_cell.x = cell.x + moves_to_neighbour[i].first;
            neighbour_cell.y = cell.y + moves_to_neighbour[i].second;
            if (cells_with_mines.find(neighbour_cell) != cells_with_mines.end()) {
                ++cnt_mines;
            }
        }
    }

    return cnt_mines > 0;
}

Minesweeper::Situation GetSituation(const Minesweeper::Cell &cell, const Minesweeper::GameStatus &game_status,
                                    const std::set<Minesweeper::Cell> &cells_with_flags,
                                    const std::set<Minesweeper::Cell> &cells_with_mines, size_t height, size_t width,
                                    size_t &cnt_mines) {

    Minesweeper::Situation situation = Minesweeper::Situation::OPEN_NEIGHBOURS;
    if (game_status == Minesweeper::GameStatus::VICTORY || game_status == Minesweeper::GameStatus::DEFEAT) {
        situation = Minesweeper::Situation::NOTHING;
    } else if (cells_with_flags.find(cell) != cells_with_flags.end()) {
        situation = Minesweeper::Situation::NOTHING;
    } else if (cells_with_mines.find(cell) != cells_with_mines.end()) {
        situation = Minesweeper::Situation::OPEN_ALL_CELLS;
    } else if (FindMinesInNeighbourCells(cell, height, width, cells_with_mines, cnt_mines)) {
        situation = Minesweeper::Situation::OPEN_THIS_CELL;
    }

    return situation;
}

void OpenNeighbours(const Minesweeper::Cell &cell, std::vector<std::vector<char>> &field, size_t height, size_t width,
                    const std::set<Minesweeper::Cell> &cells_with_mines,
                    const std::set<Minesweeper::Cell> &cells_with_flags, const Minesweeper::GameStatus &game_status) {

    Minesweeper::Cell neighbour_cell;
    std::queue<Minesweeper::Cell> handling_cells;
    std::vector<std::pair<int8_t, int8_t>> moves_to_neighbour = GetMovesToNeighbour();
    for (size_t i = 0; i < moves_to_neighbour.size(); ++i) {
        if (IsExist(cell, moves_to_neighbour[i], height, width)) {
            neighbour_cell.x = cell.x + moves_to_neighbour[i].first;
            neighbour_cell.y = cell.y + moves_to_neighbour[i].second;
            handling_cells.push(neighbour_cell);
        }
    }

    size_t cnt_mines = 0;
    Minesweeper::Cell n_cell;
    Minesweeper::Cell treatment_cell;
    std::vector<char> digits = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    while (!handling_cells.empty()) {
        treatment_cell = handling_cells.front();
        handling_cells.pop();
        if (field[treatment_cell.y][treatment_cell.x] != '-') {
            continue;
        }
        for (size_t i = 0; i < moves_to_neighbour.size(); ++i) {
            cnt_mines = 0;
            Minesweeper::Situation situation =
                GetSituation(treatment_cell, game_status, cells_with_flags, cells_with_mines, height, width, cnt_mines);
            switch (situation) {
                case Minesweeper::Situation::NOTHING:
                    break;
                case Minesweeper::Situation::OPEN_THIS_CELL:
                    if (cnt_mines) {
                        field[treatment_cell.y][treatment_cell.x] = digits[cnt_mines];
                    } else {
                        field[treatment_cell.y][treatment_cell.x] = '.';
                    }
                    break;
                case Minesweeper::Situation::OPEN_NEIGHBOURS:
                    field[treatment_cell.y][treatment_cell.x] = '.';
                    for (size_t j = 0; j < moves_to_neighbour.size(); ++j) {
                        if (IsExist(treatment_cell, moves_to_neighbour[j], height, width)) {
                            n_cell.x = treatment_cell.x + moves_to_neighbour[j].first;
                            n_cell.y = treatment_cell.y + moves_to_neighbour[j].second;
                            if (field[n_cell.y][n_cell.x] == '-') {
                                handling_cells.push(n_cell);
                            }
                        }
                    }
                    break;

                case Minesweeper::Situation::OPEN_ALL_CELLS:
                    break;
            }
        }
    }
}

void OpenAllCells(const Minesweeper::Cell &cell, std::vector<std::vector<char>> &field, size_t height, size_t width,
                  const std::set<Minesweeper::Cell> &cells_with_mines,
                  const std::set<Minesweeper::Cell> &cells_with_flags, const Minesweeper::GameStatus &game_status) {

    size_t cnt_mines = 0;
    Minesweeper::Cell treatment_cell;
    std::vector<char> digits = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    for (size_t i = 0; i < field.size(); ++i) {
        for (size_t j = 0; j < field[i].size(); ++j) {
            if (field[i][j] == '-') {
                treatment_cell.x = j;
                treatment_cell.y = i;
                if (cells_with_mines.find(treatment_cell) != cells_with_mines.end()) {
                    field[i][j] = '*';
                } else {
                    cnt_mines = 0;
                    Minesweeper::Situation situation =
                        GetSituation(treatment_cell, Minesweeper::GameStatus::IN_PROGRESS, cells_with_flags,
                                     cells_with_mines, height, width, cnt_mines);
                    if (situation == Minesweeper::Situation::OPEN_THIS_CELL) {
                        field[i][j] = digits[cnt_mines];
                    } else {
                        field[i][j] = '.';
                    }
                }
            }
        }
    }
}

Minesweeper::GameStatus IsVictory(const std::vector<std::vector<char>> &field, const size_t cnt_mines,
                                  const Minesweeper::GameStatus game_status) {

    if (game_status == Minesweeper::GameStatus::DEFEAT) {
        return game_status;
    }
    size_t cnt_opened = 0;
    for (size_t i = 0; i < field.size(); ++i) {
        for (size_t j = 0; j < field[i].size(); ++j) {
            if (field[i][j] != '-' && field[i][j] != '?') {
                ++cnt_opened;
            }
        }
    }

    if (field.size() * field[0].size() - cnt_opened == cnt_mines) {
        return Minesweeper::GameStatus::VICTORY;
    }

    return game_status;
}

void Minesweeper::OpenCell(const Cell &cell) {

    if (game_status_ == Minesweeper::GameStatus::NOT_STARTED) {
        time(&start_time_);
        game_status_ = GameStatus::IN_PROGRESS;
    }
    size_t cnt_mines = 0;
    Situation situation =
        GetSituation(cell, game_status_, cells_with_flags_, cells_with_mines_, height_, width_, cnt_mines);

    std::vector<char> digits = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    switch (situation) {

        case Situation::NOTHING:
            break;

        case Situation::OPEN_ALL_CELLS:
            OpenAllCells(cell, field_, height_, width_, cells_with_mines_, cells_with_flags_, game_status_);
            game_status_ = GameStatus::DEFEAT;
            time(&finish_time_);
            break;

        case Situation::OPEN_THIS_CELL:
            field_[cell.y][cell.x] = digits[cnt_mines];
            break;

        case Situation::OPEN_NEIGHBOURS:
            field_[cell.y][cell.x] = '.';
            OpenNeighbours(cell, field_, height_, width_, cells_with_mines_, cells_with_flags_, game_status_);
            break;
    }

    game_status_ = IsVictory(field_, cells_with_mines_.size(), game_status_);
}

void Minesweeper::MarkCell(const Cell &cell) {

    if (game_status_ == Minesweeper::GameStatus::NOT_STARTED) {
        time(&start_time_);
        game_status_ = GameStatus::IN_PROGRESS;
    }

    if (game_status_ == Minesweeper::GameStatus::VICTORY || game_status_ == Minesweeper::GameStatus::DEFEAT) {
        return;
    }

    if (field_[cell.y][cell.x] != '-' && field_[cell.y][cell.x] != '?') {
        return;
    }

    if (cells_with_flags_.find(cell) == cells_with_flags_.end()) {
        field_[cell.y][cell.x] = '?';
        cells_with_flags_.insert(cell);
    } else {
        field_[cell.y][cell.x] = '-';
        cells_with_flags_.erase(cell);
    }
}

Minesweeper::GameStatus Minesweeper::GetGameStatus() const {
    return game_status_;
}

time_t Minesweeper::GetGameTime() const {

    if (game_status_ == Minesweeper::GameStatus::NOT_STARTED) {
        return 0;
    }

    if (game_status_ == Minesweeper::GameStatus::DEFEAT || game_status_ == Minesweeper::GameStatus::VICTORY) {
        return finish_time_ - start_time_;
    }

    time_t anon = time(nullptr);
    return time(&anon) - start_time_;
}

Minesweeper::RenderedField Minesweeper::RenderField() const {

    Minesweeper::RenderedField field(height_);
    std::string line;
    for (size_t i = 0; i < height_; ++i) {
        line.clear();
        for (size_t j = 0; j < width_; ++j) {
            line += field_[i][j];
        }
        field[i] = line;
    }

    return field;
}