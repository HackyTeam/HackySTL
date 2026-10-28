#include <cassert>

#include <Containers/Bitset.hpp>
#include <Serialize/StringParser.hpp>

using namespace hsd::string_view_literals;

enum class game_status
{
    ongoing,
    x_won,
    o_won,
    draw
};

class tic_tac_toe
{
    hsd::bitset<9> _x;
    hsd::bitset<9> _o;
    bool _x_turn = true;
    bool _finished = false;

    static bool has_won(const hsd::bitset<9>& board)
    {
        constexpr int lines[][3] = {
            {0, 1, 2}, {3, 4, 5}, {6, 7, 8},
            {0, 3, 6}, {1, 4, 7}, {2, 5, 8},
            {0, 4, 8}, {2, 4, 6}
        };

        for (const auto& line : lines)
        {
            if (board.test(line[0]).unwrap() &&
                board.test(line[1]).unwrap() &&
                board.test(line[2]).unwrap())
            {
                return true;
            }
        }
        return false;
    }

public:
    auto apply(const char* move)
        -> hsd::result<game_status, hsd::runtime_error>
    {
        auto parsed = hsd::parse_string<"{} {},{}", 3>(move);
        if (!parsed.is_ok())
        {
            return hsd::runtime_error{"Malformed move"};
        }

        auto fields = parsed.unwrap();
        const char player = fields[0].to_string_view()[0];
        const hsd::i32 row = fields[1].to_i32();
        const hsd::i32 column = fields[2].to_i32();
        if (_finished)
        {
            return hsd::runtime_error{"Game is already finished"};
        }
        if ((player == 'X') != _x_turn || (player != 'X' && player != 'O'))
        {
            return hsd::runtime_error{"Move is out of turn"};
        }
        if (row < 0 || row >= 3 || column < 0 || column >= 3)
        {
            return hsd::runtime_error{"Move is outside the board"};
        }

        const auto index = static_cast<hsd::usize>(row * 3 + column);
        if (_x.test(index).unwrap() || _o.test(index).unwrap())
        {
            return hsd::runtime_error{"Cell is already occupied"};
        }

        auto& board = _x_turn ? _x : _o;
        board.set(index).unwrap();
        if (has_won(board))
        {
            _finished = true;
            return _x_turn ? game_status::x_won : game_status::o_won;
        }
        if (_x.count() + _o.count() == 9)
        {
            _finished = true;
            return game_status::draw;
        }

        _x_turn = !_x_turn;
        return game_status::ongoing;
    }

    hsd::usize occupied_cells() const
    {
        return _x.count() + _o.count();
    }
};

int main()
{
    tic_tac_toe win_game;
    assert(win_game.apply("X 0,0").unwrap() == game_status::ongoing);
    assert(!win_game.apply("X 1,0").is_ok());
    assert(win_game.occupied_cells() == 1);
    assert(win_game.apply("O 1,0").unwrap() == game_status::ongoing);
    assert(win_game.apply("X 0,1").unwrap() == game_status::ongoing);
    assert(win_game.apply("O 1,1").unwrap() == game_status::ongoing);
    assert(win_game.apply("X 0,2").unwrap() == game_status::x_won);
    assert(!win_game.apply("O 2,2").is_ok());
    assert(win_game.occupied_cells() == 5);

    tic_tac_toe draw_game;
    constexpr const char* draw_moves[] = {
        "X 0,0", "O 0,1", "X 0,2", "O 1,1", "X 1,0",
        "O 2,0", "X 1,2", "O 2,2", "X 2,1"
    };
    for (hsd::usize index = 0; index < 8; ++index)
    {
        assert(draw_game.apply(draw_moves[index]).unwrap() == game_status::ongoing);
    }
    assert(draw_game.apply(draw_moves[8]).unwrap() == game_status::draw);
}