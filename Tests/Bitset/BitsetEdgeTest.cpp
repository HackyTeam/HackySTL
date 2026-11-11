#include <cassert>
#include <string_view>

#include <Containers/Bitset.hpp>

// Edge cases of bitset spanning several words: all() with a clear bit in a later word,
// flip across words, equality that differs only in a later word, shifts by at least the
// size or by whole words, and the out-of-range access message.
int main()
{
    // Out-of-range access reports bad access with its message.
    hsd::bitset<128> _bits;
    auto _missing = _bits.test(200);
    assert(!_missing.is_ok());
    assert(std::string_view{_missing.unwrap_err().pretty_error()} == "Tried to access an element out of bounds");

    // all() is false when a bit in the first word or in a later word is clear.
    hsd::bitset<128> _full;
    _full.set();
    assert(_full.all());
    hsd::bitset<128> _later_clear = _full;
    _later_clear.reset(100);
    assert(!_later_clear.all());
    hsd::bitset<128> _first_clear = _full;
    _first_clear.reset(3);
    assert(!_first_clear.all());

    // flip inverts every word, and the bits beyond the size stay clear.
    hsd::bitset<100> _flipped;
    _flipped.flip();
    assert(_flipped.count() == 100);
    assert(_flipped.test(99).unwrap() == true);

    // Two bitsets that differ only in a later word are not equal.
    hsd::bitset<128> _left;
    hsd::bitset<128> _right;
    _left.set(120).unwrap();
    assert(_left != _right);
    assert(!(_left == _right));
    _right.set(120).unwrap();
    assert(_left == _right);

    // A shift by the size or more clears everything. A shift by zero changes nothing.
    hsd::bitset<128> _shifted;
    _shifted.set(5).unwrap();
    hsd::bitset<128> _left_all = _shifted;
    _left_all <<= 128;
    assert(_left_all.none());
    hsd::bitset<128> _right_all = _shifted;
    _right_all >>= 200;
    assert(_right_all.none());
    hsd::bitset<128> _unchanged = _shifted;
    _unchanged <<= 0;
    _unchanged >>= 0;
    assert(_unchanged == _shifted);

    // Shifts by whole words move the bits between words.
    hsd::bitset<192> _words;
    _words.set(10).unwrap();
    _words.set(150).unwrap();
    _words >>= 64;
    assert(_words.test(10 - 0).unwrap() == false);
    assert(_words.test(150 - 64).unwrap() == true);
    _words <<= 64;
    assert(_words.test(150).unwrap() == true);
    assert(_words.count() == 1);

    // Shifts that are not whole words move bits within and across words.
    hsd::bitset<128> _small;
    _small.set(5).unwrap();
    _small <<= 3;
    assert(_small.test(8).unwrap() == true && _small.count() == 1);
    _small.set(70).unwrap();
    _small >>= 3;
    assert(_small.test(67).unwrap() == true && _small.test(5).unwrap() == true && _small.count() == 2);

    hsd::bitset<192> _wide;
    _wide.set(10).unwrap();
    _wide.set(150).unwrap();
    _wide >>= 70;
    assert(_wide.count() == 1 && _wide.test(80).unwrap() == true);
    _wide <<= 70;
    assert(_wide.count() == 1 && _wide.test(150).unwrap() == true);
    return 0;
}
