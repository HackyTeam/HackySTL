// Title: String basics
// Level: Basic
// Shows: building hsd::string, length versus size, concatenation, searching and sub_string.

#include <Containers/String.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

int main()
{
    hsd::string greeting{"Hello"};

    // length() counts characters. size() also counts the terminating NUL.
    hsd::println("length={} size={}"_fmt, greeting.length(), greeting.size());

    greeting += hsd::string{", "};
    greeting += hsd::string{"world"};
    greeting.push_back('!');
    hsd::println("{}"_fmt, greeting);

    // operator+ builds a new string and leaves the operands alone.
    hsd::string shout = greeting + " Again!";
    hsd::println("{} / {}"_fmt, greeting, shout);

    hsd::println("ends: {} and {}"_fmt, greeting.front(), greeting.back());
    hsd::println("greeting[4]={}"_fmt, greeting[4]);

    // find() returns the index of a match, or npos when there is none.
    hsd::usize comma = greeting.find(',');
    hsd::println("comma at {}"_fmt, comma);
    hsd::println("'xyz' found: {}"_fmt,
        greeting.find("xyz") != hsd::string::npos ? "true" : "false");
    hsd::println("rfind 'o' at {}"_fmt, greeting.rfind('o'));

    hsd::println("starts with Hello: {}"_fmt, greeting.starts_with("Hello") ? "true" : "false");
    hsd::println("ends with '?': {}"_fmt, greeting.ends_with('?') ? "true" : "false");
    hsd::println("contains 'wor': {}"_fmt, greeting.contains("wor") ? "true" : "false");

    // sub_string() returns a result: a range past the end is an error.
    auto word = greeting.sub_string(7, 5);
    if (word.is_ok())
    {
        hsd::println("word: '{}'"_fmt, word.unwrap());
    }

    auto bad = greeting.sub_string(100, 2);
    hsd::println("sub_string(100, 2) is {}"_fmt, bad.is_ok() ? "ok" : "an error");

    // Strings compare by content.
    hsd::string other{"Hello, world!"};
    hsd::println("equal: {}"_fmt, greeting == other ? "true" : "false");

    // to_string() turns a number into text.
    hsd::string count = hsd::to_string(2024);
    hsd::println("number as text: {} ({} characters)"_fmt, count, count.length());

    // Appending into spare capacity still updates the length.
    hsd::string padded{"abc"};
    padded.reserve(20);
    padded += "de";
    hsd::println("padded: {} ({} characters)"_fmt, padded, padded.length());

    // An empty string is a valid operand on either side of a concatenation.
    hsd::string empty;
    hsd::println("empty + word: '{}'"_fmt, empty + hsd::string{"word"});
    hsd::println("word + empty: '{}'"_fmt, hsd::string{"word"} + empty);

    return 0;
}
