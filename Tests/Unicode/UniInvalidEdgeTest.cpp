#include <cassert>

#include <Base/Unicode.hpp>

// Invalid and empty input to the Unicode helpers: a decode with no bytes left, a length of
// a null string, and a length of UTF-8 input whose sequences do not decode, which counts
// each invalid byte as one unit.
int main()
{
    const hsd::char8 _bytes[] = {static_cast<hsd::char8>('a'), static_cast<hsd::char8>(0)};
    hsd::char32 _code_point = 0;
    assert(hsd::unicode::unicode_detail::decode(_bytes, 0, _code_point) == 0);

    assert((hsd::unicode::length<hsd::char16, hsd::char32>(nullptr)) == 0);

    // 0xFF cannot start a UTF-8 sequence, so each of the two invalid bytes counts once.
    const hsd::char8 _invalid[] = {static_cast<hsd::char8>(0xFF), static_cast<hsd::char8>(0xFE), static_cast<hsd::char8>(0)};
    assert((hsd::unicode::length<hsd::char16, hsd::char8>(_invalid)) == 2);
    return 0;
}
