#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>

enum class PieceColor { White, Black };
enum class PieceType { Empty, Pawn, Knight, Bishop, Rook, King };

class Coordinate {
public:
    int x, y;

    Coordinate operator+(const Coordinate& other) const {
        return { this->x + other.x, this->y + other.y };
    }

    bool operator==(const Coordinate& other) const {
        return this->x == other.x && this->y == other.y;
    }

    friend std::ostream& operator<<(std::ostream& os, const Coordinate& coord) {
        os << static_cast<char>('a' + coord.x) << 8 - coord.y;
        return os;
    }
};

class Piece {
public:
    PieceColor color;
    PieceType type;

    Piece(PieceColor color, PieceType type)
        : color(color), type(type)
    {}

    friend std::ostream& operator<<(std::ostream& os, const Piece& piece) {
        switch (piece.color) {
            case PieceColor::White:
                os << "\033[97m";
                break;
            case PieceColor::Black:
                os << "\033[90m";
        }

        switch (piece.type) {
            case PieceType::Empty:
                os << " ";
                break;
            case PieceType::Pawn:
                os << "P";
                break;
            case PieceType::Knight:
                os << "N";
                break;
            case PieceType::King:
                os << "K";
                break;
            case PieceType::Bishop:
                os << "B";
                break;
            case PieceType::Rook:
                os << "R";
                break;
        }

        os << "\033[0m";
        return os;
    }
};

class Move {
public:
    Coordinate start, end;

    friend std::ostream& operator<<(std::ostream& os, const Move& move) {
        os << move.start << " -> " << move.end;
        return os;
    }
};

class Board {
private:
    std::vector<std::vector<Piece>> squares;
    bool whiteCheck, blackCheck;
    int whiteEvaluation;

    const std::vector<Coordinate> knightDeltas
    { { -2, -1 }, { -1, -2 }, { 1, -2 }, { 2, -1 }, { 2, 1 }, { 1, 2 }, { -1, 2 }, { -2, 1 } };

    const std::vector<Coordinate> kingDeltas
    { { -1, -1 }, { 0, -1 }, { 1, -1 }, { 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 }, { -1, 0 } };

    const std::vector<Coordinate> bishopDirections
    { { -1, -1 }, { 1, -1 }, { 1, 1 }, { -1, 1 } };

    const std::vector<Coordinate> rookDirections
    { { -1, 0 }, { 1, 0}, { 0, -1 }, { 0, 1 } };

public:
    Board() : whiteCheck(false), blackCheck(false), whiteEvaluation(0) {
        for (int y = 0; y < 8; y++) {
            std::vector<Piece> row;
            for (int x = 0; x < 8; x++) {
                row.push_back(Piece(PieceColor::Black, PieceType::Empty));
            }
            this->squares.push_back(row);
        }
    }

    void fill(unsigned int pawnCount = 8, unsigned int knightCount = 2,
              unsigned int bishopCount = 2, unsigned int rookCount = 2) {
        pawnCount = std::min(pawnCount, 8u);
        knightCount = std::min(knightCount, 2u);
        bishopCount = std::min(bishopCount, 2u);
        rookCount = std::min(rookCount, 2u);

        for (unsigned int i = 0; i < rookCount; i++) {
            this->squares[0][i * 7] = Piece(PieceColor::Black, PieceType::Rook);
        }

        for (unsigned int i = 0; i < knightCount; i++) {
            this->squares[0][i * 5 + 1] = Piece(PieceColor::Black, PieceType::Knight);
        }

        for (unsigned int i = 0; i < bishopCount; i++) {
            this->squares[0][i * 3 + 2] = Piece(PieceColor::Black, PieceType::Bishop);
        }

        this->squares[0][4] = Piece(PieceColor::Black, PieceType::King);

        for (unsigned int i = 0; i < pawnCount; i++) {
            this->squares[1][i] = Piece(PieceColor::Black, PieceType::Pawn);
        }

        for (unsigned int i = 0; i < pawnCount; i++) {
            this->squares[6][i] = Piece(PieceColor::White, PieceType::Pawn);
        }

        for (unsigned int i = 0; i < rookCount; i++) {
            this->squares[7][i * 7] = Piece(PieceColor::White, PieceType::Rook);
        }

        for (unsigned int i = 0; i < knightCount; i++) {
            this->squares[7][i * 5 + 1] = Piece(PieceColor::White, PieceType::Knight);
        }

        for (unsigned int i = 0; i < bishopCount; i++) {
            this->squares[7][i * 3 + 2] = Piece(PieceColor::White, PieceType::Bishop);
        }

        this->squares[7][4] = Piece(PieceColor::White, PieceType::King);
    }

    void makeMove(const Move& move) {
        Piece piece = this->squares[move.start.y][move.start.x];
        this->squares[move.end.y][move.end.x] = piece;
        this->squares[move.start.y][move.start.x] = Piece(PieceColor::Black, PieceType::Empty);

        this->updateChecks();
        this->updateEvaluations();
    }

    bool isChecked(PieceColor color) const {
        return color == PieceColor::White ? this->whiteCheck : this->blackCheck;
    }

    int getEvaluation(PieceColor color) const {
        return color == PieceColor::White ? this->whiteEvaluation : -this->whiteEvaluation;
    }

    std::vector<Move> getAvailableMoves(PieceColor color) const {
        std::vector<Move> moves;
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                const Piece& piece = this->squares[y][x];
                if (piece.type == PieceType::Empty || piece.color != color) {
                    continue;
                }
                this->getAvailablePieceMoves(moves, { x, y });
            }
        }
        return moves;
    }

    std::vector<std::pair<int, Move>> getEvaluatedMoves(PieceColor color) const {
        std::vector<std::pair<int, Move>> ratings;
        for (const Move& move : this->getAvailableMoves(color)) {
            Board boardCopy = *this;
            boardCopy.makeMove(move);
            ratings.push_back({ boardCopy.getEvaluation(color), move });
        }
        std::sort(ratings.begin(), ratings.end(), [](const std::pair<int, Move>& a, const std::pair<int, Move>& b) { return a.first > b.first; });
        return ratings;
    }

    friend std::ostream& operator<<(std::ostream& os, const Board& board) {
        os << "   +---+---+---+---+---+---+---+---+" << std::endl;
        for (int y = 0; y < 8; y++) {
            os << 8 - y << "  | ";
            for (int x = 0; x < 8; x++) {
                os << board.squares[y][x] << " | ";
            }
            os << std::endl << "   +---+---+---+---+---+---+---+---+" << std::endl;
        }
        os << "     a   b   c   d   e   f   g   h" << std::endl;
        return os;
    }

private:
    void getAvailablePieceMoves(std::vector<Move>& moves, const Coordinate& start) const {
        const Piece& piece = this->squares[start.y][start.x];
        switch (piece.type) {
            case PieceType::Empty:
                break;
            case PieceType::Pawn: {
                int deltaY = piece.color == PieceColor::White ? -1 : 1;
                this->checkAndAddMove(moves, { start, start + Coordinate { 0, deltaY } });
                if ((piece.color == PieceColor::White && start.y == 6) || (piece.color == PieceColor::Black && start.y == 1)) {
                    this->checkAndAddMove(moves, { start, start + Coordinate { 0, deltaY * 2 } });
                }
                this->checkAndAddAttack(moves, piece.color, { start, start + Coordinate { -1, deltaY } });
                this->checkAndAddAttack(moves, piece.color, { start, start + Coordinate { 1, deltaY } });
                break;
            }
            case PieceType::Knight: {
                for (const Coordinate& delta : this->knightDeltas) {
                    this->checkAndAddAttackOrOccupy(moves, piece.color, { start, start + delta });
                }
                break;
            }
            case PieceType::King: {
                for (const Coordinate& delta : this->kingDeltas) {
                    this->checkAndAddAttackOrOccupy(moves, piece.color, { start, start + delta });
                }
                break;
            }
            case PieceType::Bishop: {
                for (const Coordinate& direction : this->bishopDirections) {
                    for (int i = 1; i < 8; i++) {
                        Move move { start, start + Coordinate { i * direction.x, i * direction.y } };
                        if (this->checkAndAddAttack(moves, piece.color, move) || !this->checkAndAddMove(moves, move)) {
                            break;
                        }
                    }
                }
                break;
            }
            case PieceType::Rook: {
                for (const Coordinate& direction : this->rookDirections) {
                    for (int i = 1; i < 8; i++) {
                        Move move { start, start + Coordinate { i * direction.x, i * direction.y } };
                        if (this->checkAndAddAttack(moves, piece.color, move) || !this->checkAndAddMove(moves, move)) {
                            break;
                        }
                    }
                }
                break;
            }
        }
    }

    bool isWithinBounds(const Coordinate& coord) const {
        return coord.y >= 0 && coord.x >= 0 && coord.y < 8 && coord.x < 8;
    }

    bool checkAndAddMove(std::vector<Move>& moves, const Move& move) const {
        if (!isWithinBounds(move.end)) {
            return false;
        }
        const Piece& piece = this->squares[move.end.y][move.end.x];
        if (piece.type != PieceType::Empty) {
            return false;
        }
        moves.push_back(move);
        return true;
    }

    bool checkAndAddAttack(std::vector<Move>& moves, PieceColor originColor, const Move& move) const {
        if (!isWithinBounds(move.end)) {
            return false;
        }
        const Piece& piece = this->squares[move.end.y][move.end.x];
        if (piece.type == PieceType::Empty || piece.color == originColor) {
            return false;
        }
        moves.push_back(move);
        return true;
    }

    bool checkAndAddAttackOrOccupy(std::vector<Move>& moves, PieceColor originColor, const Move& move) const {
        if (!isWithinBounds(move.end)) {
            return false;
        }
        const Piece& piece = this->squares[move.end.y][move.end.x];
        if (piece.type != PieceType::Empty && piece.color == originColor) {
            return false;
        }
        moves.push_back(move);
        return true;
    }

    void updateChecks() {
        this->whiteCheck = this->blackCheck = false;
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                const Piece& piece = this->squares[y][x];
                if (piece.type != PieceType::King) {
                    continue;
                }
                Coordinate king { x, y };
                for (const Move& move : this->getAvailableMoves(static_cast<PieceColor>(!static_cast<bool>(piece.color)))) {
                    if (move.end == king) {
                        if (piece.color == PieceColor::White) {
                            this->blackCheck = true;
                        } else {
                            this->whiteCheck = true;
                        }
                        break;
                    }
                }
            }
        }
    }

    void updateEvaluations() {
        this->whiteEvaluation = 0;
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                const Piece& piece = this->squares[y][x];
                int delta = piece.color == PieceColor::White ? 1 : -1;
                switch (piece.type) {
                    case PieceType::Empty:
                        break;
                    case PieceType::Pawn:
                        this->whiteEvaluation += 1 * delta;
                        break;
                    case PieceType::Knight:
                    case PieceType::Bishop:
                        this->whiteEvaluation += 3 * delta;
                        break;
                    case PieceType::Rook:
                        this->whiteEvaluation += 5 * delta;
                        break;
                    case PieceType::King:
                        if (this->isChecked(PieceColor::White)) {
                            this->whiteEvaluation += 1000;
                        }
                        if (this->isChecked(PieceColor::Black)) {
                            this->whiteEvaluation -= 1000;
                        }
                        break;
                }
            }
        }
    }
};

class Game {
private:
    Board board;
    PieceColor turn;

public:
    Game() : turn(PieceColor::White) {}

    void fill(unsigned int pawnCount = 8, unsigned int knightCount = 2,
              unsigned int bishopCount = 2, unsigned int rookCount = 2) {
        this->board.fill(pawnCount, knightCount, bishopCount, rookCount);
    }

    void randomMove() {
        std::vector<Move> availableMoves = this->board.getAvailableMoves(this->turn);
        if (availableMoves.empty()) {
            return;
        }
        const Move& move = availableMoves[std::rand() % availableMoves.size()];
        this->board.makeMove(move);
        this->changeTurn();
    }

    std::vector<std::vector<Move>> evaluateBestMoves() {
        std::vector<std::vector<Move>> result;
        for (int i = 0; i < 3; i++) {
            std::vector<Move> moves;
            Board boardCopy = this->board;
            for (int j = 0; j < 3; j++) {
                Move move = boardCopy.getEvaluatedMoves(this->turn)[i].second;
                boardCopy.makeMove(move);
                moves.push_back(move);
                if (boardCopy.isChecked(this->turn)) {
                    break;
                }
            }
            result.push_back(moves);
        }
        return result;
    }

    friend std::ostream& operator<<(std::ostream& os, const Game& game) {
        os << game.board;
        return os;
    }

private:
    void changeTurn() {
        this->turn = static_cast<PieceColor>(!static_cast<bool>(this->turn));
    }
};

int main() {
    srand(static_cast<unsigned>(time(NULL)));

    Game game;

    game.fill();
    for (int i = 0; i < 30; i++) {
        game.randomMove();
    }

    std::cout << game << std::endl;

    for (const std::vector<Move>& moves : game.evaluateBestMoves()) {
        for (const Move& move : moves) {
            std::cout << move.start << '-' << move.end << "; ";
        }
        std::cout << std::endl;
    }

    return 0;
}

