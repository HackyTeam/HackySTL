// Title: Vector of vectors as a matrix
// Level: Intermediate
// Shows: nested hsd::vector as a matrix, transpose, multiplication and row sums, with checked access.

#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

using row = hsd::vector<hsd::i32>;
using matrix = hsd::vector<row>;

// Builds a rows x cols matrix filled with 1, 2, 3, ... in row-major order.
matrix make_matrix(hsd::usize rows, hsd::usize cols)
{
    matrix result;
    hsd::i32 next = 1;

    for (hsd::usize i = 0; i < rows; ++i)
    {
        // emplace_back(size) value-initializes a row of that many zeros.
        result.emplace_back(cols);
        for (hsd::usize j = 0; j < cols; ++j)
        {
            result[i][j] = next++;
        }
    }

    return result;
}

matrix transpose(const matrix& source)
{
    hsd::usize rows = source.size();
    hsd::usize cols = rows == 0 ? 0 : source[0].size();
    matrix result;

    for (hsd::usize j = 0; j < cols; ++j)
    {
        result.emplace_back(rows);
        for (hsd::usize i = 0; i < rows; ++i)
        {
            result[j][i] = source[i][j];
        }
    }

    return result;
}

// Returns an empty matrix when the inner dimensions do not agree.
matrix multiply(const matrix& lhs, const matrix& rhs)
{
    matrix result;

    if (lhs.size() == 0 || rhs.size() == 0 || lhs[0].size() != rhs.size())
    {
        return result;
    }

    for (hsd::usize i = 0; i < lhs.size(); ++i)
    {
        result.emplace_back(rhs[0].size());
        for (hsd::usize j = 0; j < rhs[0].size(); ++j)
        {
            for (hsd::usize k = 0; k < rhs.size(); ++k)
            {
                result[i][j] += lhs[i][k] * rhs[k][j];
            }
        }
    }

    return result;
}

void print_matrix(const char* label, const matrix& value)
{
    hsd::println("{}:"_fmt, label);
    for (const auto& line : value)
    {
        for (hsd::usize j = 0; j < line.size(); ++j)
        {
            if (j == 0)
            {
                hsd::print("{}"_fmt, line[j]);
            }
            else
            {
                hsd::print(" {}"_fmt, line[j]);
            }
        }
        hsd::println(""_fmt);
    }
}

int main()
{
    matrix a = make_matrix(2, 3);
    print_matrix("a", a);

    matrix a_t = transpose(a);
    print_matrix("transpose of a", a_t);

    // (2x3) * (3x2) gives a 2x2 matrix.
    print_matrix("a * transpose", multiply(a, a_t));

    // (2x3) * (2x3) cannot be multiplied, so the result is empty.
    matrix bad = multiply(a, a);
    hsd::println("a * a has {} rows"_fmt, bad.size());

    // Row sums use at() so an index mistake becomes an error, not a bad read.
    for (hsd::usize i = 0; i < 3; ++i)
    {
        auto line = a.at(i);
        if (line.is_ok())
        {
            hsd::i32 sum = 0;
            for (auto value : line.unwrap().get())
            {
                sum += value;
            }
            hsd::println("row {} sum={}"_fmt, i, sum);
        }
        else
        {
            hsd::println("row {} does not exist"_fmt, i);
        }
    }

    return 0;
}
