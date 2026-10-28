// Title: Stack array matrices
// Level: Intermediate
// Shows: nested stack_array as a fixed-size matrix, transpose and multiplication.

#include <Containers/StackArray.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// A stack_array of stack_arrays stores the whole matrix inline, no allocation.
template <hsd::usize N>
using matrix = hsd::stack_array<hsd::stack_array<hsd::i32, N>, N>;

template <hsd::usize N>
void print_matrix(const char* label, const matrix<N>& mat)
{
    hsd::println("{}:"_fmt, label);
    for (const auto& row : mat)
    {
        hsd::print("  "_fmt);
        for (hsd::usize col = 0; col < N; ++col)
        {
            // Column 0 has no separator, so lines carry no trailing space.
            if (col != 0)
            {
                hsd::print(" "_fmt);
            }
            hsd::print("{}"_fmt, row[col]);
        }
        hsd::println(""_fmt);
    }
}

template <hsd::usize N>
matrix<N> transpose(const matrix<N>& mat)
{
    matrix<N> result;
    for (hsd::usize row = 0; row < N; ++row)
    {
        for (hsd::usize col = 0; col < N; ++col)
        {
            result[col][row] = mat[row][col];
        }
    }
    return result;
}

template <hsd::usize N>
matrix<N> multiply(const matrix<N>& lhs, const matrix<N>& rhs)
{
    matrix<N> result;
    for (hsd::usize row = 0; row < N; ++row)
    {
        for (hsd::usize col = 0; col < N; ++col)
        {
            hsd::i32 sum = 0;
            for (hsd::usize k = 0; k < N; ++k)
            {
                sum += lhs[row][k] * rhs[k][col];
            }
            result[row][col] = sum;
        }
    }
    return result;
}

int main()
{
    matrix<3> mat;
    hsd::i32 counter = 1;
    for (auto& row : mat)
    {
        for (auto& cell : row)
        {
            cell = counter++;
        }
    }
    print_matrix<3>("matrix", mat);

    print_matrix<3>("transposed", transpose<3>(mat));

    // Build an identity matrix: fill the rows with zero, then set the diagonal.
    matrix<3> identity;
    for (hsd::usize idx = 0; idx < 3; ++idx)
    {
        identity[idx].fill(0);
        identity[idx][idx] = 1;
    }
    print_matrix<3>("times identity", multiply<3>(mat, identity));
    print_matrix<3>("squared", multiply<3>(mat, mat));

    // Matrices are plain values, so swap() works on whole matrices too.
    matrix<3> flipped = transpose<3>(mat);
    hsd::swap(mat, flipped);
    hsd::println("after swap, first row of matrix: {} {} {}"_fmt,
        mat[0][0], mat[0][1], mat[0][2]);

    return 0;
}
