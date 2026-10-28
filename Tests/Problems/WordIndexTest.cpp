#include <algorithm>
#include <cassert>

#include <Containers/Range.hpp>
#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Containers/UnorderedMap.hpp>
#include <Containers/Vector.hpp>

using namespace hsd::string_view_literals;

using document_postings = hsd::unordered_map<int, hsd::vector<hsd::usize>>;
using inverted_index = hsd::unordered_map<hsd::string, document_postings>;

struct search_hit
{
    int document_id;
    hsd::usize score;
    bool phrase_match;
};

static auto tokenize(hsd::string_view text)
{
    hsd::vector<hsd::string> tokens;
    hsd::string token;

    auto flush = [&]
    {
        if (token.length() != 0)
        {
            tokens.push_back(hsd::move(token));
            token.clear();
        }
    };

    for (const char character : text)
    {
        if ((character >= 'A' && character <= 'Z') ||
            (character >= 'a' && character <= 'z') ||
            (character >= '0' && character <= '9'))
        {
            const char lowered = character >= 'A' && character <= 'Z'
                ? static_cast<char>(character - 'A' + 'a')
                : character;
            token.push_back(lowered);
        }
        else
        {
            flush();
        }
    }
    flush();
    return tokens;
}

static void index_document(inverted_index& index, int document_id, hsd::string_view text)
{
    const auto tokens = tokenize(text);
    for (hsd::usize position = 0; position < tokens.size(); ++position)
    {
        index[tokens[position]][document_id].push_back(position);
    }
}

static bool contains_position(
    const hsd::vector<hsd::usize>& positions, hsd::usize target)
{
    for (const hsd::usize position : positions)
    {
        if (position == target)
        {
            return true;
        }
    }
    return false;
}

static bool matches_phrase(
    const inverted_index& index,
    int document_id,
    const hsd::vector<hsd::string>& terms)
{
    auto first_term = index.at(terms[0]);
    if (!first_term.is_ok())
    {
        return false;
    }
    auto first_document = first_term.unwrap().get().at(document_id);
    if (!first_document.is_ok())
    {
        return false;
    }

    const auto& starts = first_document.unwrap().get();
    for (const hsd::usize start : starts)
    {
        bool consecutive = true;
        for (hsd::usize offset = 1; offset < terms.size(); ++offset)
        {
            auto term = index.at(terms[offset]);
            if (!term.is_ok())
            {
                consecutive = false;
                break;
            }
            auto document = term.unwrap().get().at(document_id);
            if (!document.is_ok() || !contains_position(
                document.unwrap().get(), start + offset))
            {
                consecutive = false;
                break;
            }
        }
        if (consecutive)
        {
            return true;
        }
    }
    return false;
}

static auto search(const inverted_index& index, hsd::string_view query)
{
    const auto terms = tokenize(query);
    hsd::vector<search_hit> hits;
    if (terms.size() == 0)
    {
        return hits;
    }

    auto first_term = index.at(terms[0]);
    if (!first_term.is_ok())
    {
        return hits;
    }

    for (const auto& [document_id, first_positions] : first_term.unwrap().get())
    {
        hsd::usize score = first_positions.size();
        bool contains_all_terms = true;
        for (hsd::usize term_index = 1; term_index < terms.size(); ++term_index)
        {
            auto term = index.at(terms[term_index]);
            if (!term.is_ok())
            {
                contains_all_terms = false;
                break;
            }
            auto document = term.unwrap().get().at(document_id);
            if (!document.is_ok())
            {
                contains_all_terms = false;
                break;
            }
            score += document.unwrap().get().size();
        }

        if (contains_all_terms)
        {
            hits.push_back(search_hit{
                document_id,
                score,
                matches_phrase(index, document_id, terms)
            });
        }
    }

    std::sort(hits.begin(), hits.end(), [](const search_hit& left, const search_hit& right)
    {
        if (left.score != right.score)
        {
            return left.score > right.score;
        }
        return left.document_id < right.document_id;
    });
    return hits;
}

int main()
{
    inverted_index index;
    index_document(index, 101, "Blue fox blue");
    index_document(index, 102, "Fox over red ground");
    index_document(index, 103, "Blue whale swims");
    index_document(index, 104, "blue fox in blue forest blue");

    auto hits = search(index, "BLUE, fox!");
    assert(hits.size() == 2);
    assert(hits[0].document_id == 104 && hits[0].score == 4);
    assert(hits[0].phrase_match);
    assert(hits[1].document_id == 101 && hits[1].score == 3);
    assert(hits[1].phrase_match);

    auto missing = search(index, "whale fox");
    assert(missing.size() == 0);
    auto empty = search(index, "---");
    assert(empty.size() == 0);

    hsd::usize high_score_count = 0;
    hsd::usize high_score_total = 0;
    for (const auto& hit : hits | hsd::ranges::filter(
        [](const search_hit& value) { return value.score >= 4; }
    ))
    {
        ++high_score_count;
        high_score_total += hit.score;
    }
    assert(high_score_count == 1);
    assert(high_score_total == 4);
}