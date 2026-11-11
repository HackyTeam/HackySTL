#include <cassert>

#include <Base/CString.hpp>

// Edge cases of the C-string helpers: text without digits, signs in front of
// floating-point values, ranges that end early, compares longer than one block of the
// block-wise scan, and copies with a null destination or a short source.
int main()
{
    // find_rev returns nullptr when the needle is not in the scanned part of the string.
    const char* hello = "hello";
    assert(hsd::cstring::find_rev(hello, "xyz", 4) == nullptr);
    assert(hsd::cstring::find_rev(hello, "ll", 4) == hello + 2);

    // Null-terminated floating-point parse: text without digits gives zero, and a sign
    // without a number after it is still applied.
    assert(hsd::cstring::parse<float>("abc") == 0.0f);
    assert(hsd::cstring::parse<float>("-7") == -7.0f);
    assert(hsd::cstring::parse<float>("-2.5") == -2.5f);
    assert(hsd::cstring::parse<float>("-2.5x") == -2.5f);
    assert(hsd::cstring::parse<float>("3.25") == 3.25f);
    assert(hsd::cstring::parse<float>("12x") == 12.0f);
    assert(hsd::cstring::parse<float>("-12x") == -12.0f);

    // Range parse of integers and floating-point values, with a sign and early ends.
    const char* signed_text = "-2.5";
    assert(hsd::cstring::parse<float>(signed_text, signed_text + 4) == -2.5f);
    const char* letters = "abc";
    assert(hsd::cstring::parse<float>(letters, letters + 3) == 0.0f);
    const char* short_negative = "-7";
    assert(hsd::cstring::parse<float>(short_negative, short_negative + 2) == -7.0f);
    const char* digits_then_letter = "12x";
    assert(hsd::cstring::parse<float>(digits_then_letter, digits_then_letter + 3) == 12.0f);
    const char* negative_digits = "-12x";
    assert(hsd::cstring::parse<float>(negative_digits, negative_digits + 4) == -12.0f);
    const char* unsigned_text = "x42";
    assert(hsd::cstring::parse<unsigned>(unsigned_text, unsigned_text + 3) == 42u);
    const char* negative_int = "-17";
    assert(hsd::cstring::parse<int>(negative_int, negative_int + 3) == -17);

    // Compare with a length: equal prefixes, a difference at the end, and a difference in
    // the tail after the blocks. The result sign follows the first differing character.
    assert(hsd::cstring::compare("abc", "abc", 3) == 0);
    assert(hsd::cstring::compare("abc", "abd", 3) < 0);
    assert(hsd::cstring::compare("abd", "abc", 3) > 0);
    assert(hsd::cstring::compare("abc", "abd", 0) == 0);
    assert(hsd::cstring::compare("abc", "abd") < 0);
    assert(hsd::cstring::compare("abd", "abc") > 0);

    const char* long_a = "0123456789012345678901234567890123456789ABCDEFGH";
    const char* long_b = "0123456789012345678901234567890123456789ABCDEFGX";
    const char* long_same = "0123456789012345678901234567890123456789ABCDEFGH";
    assert(hsd::cstring::compare(long_a, long_same, 48) == 0);
    assert(hsd::cstring::compare(long_a, long_b, 48) < 0);
    assert(hsd::cstring::compare(long_b, long_a, 48) > 0);

    // A difference inside the first 32-character block, and one in a later block.
    const char* early_a = "0123456789abcdefghijklmnopqrstuv_tail";
    const char* early_b = "0123456789abcdefghijklmnopqrstuw_tail";
    assert(hsd::cstring::compare(early_a, early_b, 36) < 0);
    const char* late_a = "0123456789abcdefghijklmnopqrstuvwxyz0123456789";
    const char* late_b = "0123456789abcdefghijklmnopqrstuvwxyz012345678_";
    assert(hsd::cstring::compare(late_a, late_b, 46) < 0);

    // A terminator stops the comparison early, so later characters are ignored.
    const char* nul_a = "ab\0x";
    const char* nul_b = "ab\0y";
    assert(hsd::cstring::compare(nul_a, nul_b, 4) == 0);
    const char* block_nul_a = "0123456789abcdefghijklmnopqrstuv\0xyz";
    const char* block_nul_b = "0123456789abcdefghijklmnopqrstuv\0xyq";
    assert(hsd::cstring::compare(block_nul_a, block_nul_b, 40) == 0);

    // Copy with a length and a source that ends before the length.
    char buffer[8] = {};
    assert(hsd::cstring::copy(nullptr, "ab", 2) == nullptr);
    char* end_of_copy = hsd::cstring::copy(buffer, "ab", 5);
    assert(buffer[0] == 'a' && buffer[1] == 'b' && buffer[2] == '\0');
    assert(end_of_copy == buffer + 2);

    // Copy with no length: a null destination returns null, and a string is copied in full.
    assert(hsd::cstring::copy(nullptr, "x") == nullptr);
    char full[8] = {};
    assert(hsd::cstring::copy(full, "hello") == full);
    assert(hsd::cstring::compare(full, "hello") == 0);
    return 0;
}
