#pragma once

/// @file Json.hpp
/// @brief JSON lexing, parsing, typed values, and tree traversal.
/// @details Builds an owning value tree for objects, arrays, strings, numbers,
/// and primitives with checked typed access and source-completion state.

#include "../Containers/List.hpp"
#include "../Containers/String.hpp"
#include "../Wrappers/UniquePtr.hpp"
#include "../Containers/UnorderedMap.hpp"
#include "../Wrappers/Variant.hpp"
#include "../Serialize/Io.hpp"

namespace hsd
{
    /** @def CASE_CH
     *  @brief Local helper expanding a JSON character into a token case.
     */
    /// @brief Lexical token kinds emitted by JsonStream.
    enum class JsonToken
    {
        Null,    ///< `null` literal.
        True,    ///< `true` literal.
        False,   ///< `false` literal.
        Number,  ///< Integer or floating-point literal.
        String,  ///< Quoted string literal.
        BArray,  ///< `[` begin-array delimiter.
        EArray,  ///< `]` end-array delimiter.
        BObject, ///< `{` begin-object delimiter.
        EObject, ///< `}` end-object delimiter.
        Comma,   ///< `,` value/member separator.
        Colon,   ///< `:` key/value separator.

        Eof = -1,   ///< End of the token stream.
        Error = -2, ///< Lexical error was encountered.
        Empty = -3  ///< Internal: no token is currently being accumulated.
    };

    /// @brief JSON syntax error carrying a source position.
    class JsonError : public runtime_error
    {
        hsd::string _str; ///< Lazily built "message position" text cached by pretty_error().

    public:
        usize position; ///< Byte offset in the source where the error was detected.

        /// @brief Construct an error with a message and the offset where it occurred.
        /// @param msg Human-readable description of the failure.
        /// @param pos Byte offset into the input at which the failure was detected.
        constexpr JsonError(const char* msg, usize pos)
            : runtime_error{msg}, position{pos} {}

        /// @brief Get a human-readable "message position" string.
        /// @details Builds and caches the formatted string on first call.
        /// @return Null-terminated string valid for the lifetime of this error.
        constexpr const char* pretty_error()
        {
            if (_str.length() != 0)
                return _str.c_str();

            return (_str = _err + (" " + to_string(position))).c_str();
        }
    };

    /// @brief Incremental JSON lexer and token buffer.
    /// @details Consumes source fragments incrementally via lex()/lex_file()
    /// and accumulates a queue of tokens plus their associated string/number
    /// payloads for later retrieval by pop_string()/pop_number().
    class JsonStream
    {
        using vstr = basic_string_view<char>; ///< View type used to scan input fragments.
        using str = basic_string<char>; ///< Owning string type used for buffered payloads.
        using num = variant<i64, f128>; ///< Numeric payload: either an integer or a float.

        list<JsonToken> _tokens; ///< FIFO queue of tokens produced so far.
        list<str> _qtok_string; ///< FIFO queue of decoded string payloads, one per String token.
        list<num> _qtok_number; ///< FIFO queue of parsed number payloads, one per Number token.
        JsonToken _current_token = JsonToken::Empty; ///< Token currently being accumulated, if any.
        const char* _token_kw = nullptr; ///< Remaining keyword ("null"/"true"/"false") being matched.
        usize _token_position = 0; ///< Cursor into `_token_kw`/`_token_str` for the in-progress token.
        str _token_str; ///< Raw characters accumulated for the in-progress string or number token.

        usize _pos = 0; ///< Total number of characters consumed across all lex() calls.

    public:
        /// @brief Access the queue of tokens produced by the lexer so far.
        /// @return Reference to the internal FIFO token queue.
        constexpr auto& get_tokens() { return _tokens; }

        /// @brief Remove and return the oldest buffered string payload.
        /// @details Must be called exactly once for each JsonToken::String
        /// token retrieved from get_tokens(), in order.
        /// @return The decoded string associated with the next String token.
        constexpr str pop_string()
        {
            str _s = hsd::move(_qtok_string.front());
            _qtok_string.pop_front();
            return _s;
        }

        /// @brief Remove and return the oldest buffered number payload.
        /// @details Must be called exactly once for each JsonToken::Number
        /// token retrieved from get_tokens(), in order.
        /// @return The parsed integer or floating-point value.
        constexpr num pop_number()
        {
            num _n = hsd::move(_qtok_number.front());
            _qtok_number.pop_front();
            return _n;
        }

        /// @brief Feed a fragment of JSON source text into the lexer.
        /// @details Tokens and their payloads are appended to the internal
        /// queues incrementally, so a single JSON document can be lexed
        /// across multiple calls with `frag` boundaries falling anywhere,
        /// including mid-token. If an error occurs, the rest of the buffer
        /// can still be passed in on a subsequent call to continue recovery.
        /// @param frag Next chunk of source text to scan.
        /// @return Empty on success, or a JsonError describing the first
        /// syntax error and its position.
        constexpr option_err<JsonError> lex(vstr frag)
        {
            constexpr const char* const s_keywords[] = {"null", "true", "false"};
            for (bool _floating = false; char _ch : frag)
            {
                ++_pos;
                
                _reparse:
                if (_current_token == JsonToken::Empty)
                {
                    if (basic_cstring<char>::iswhitespace(_ch))
                        continue;
                    
                    switch (_ch)
                    {
                        #define CASE_CH(ch, tok)    \
                        case ch:\
                        {                           \
                            _tokens.push_back(tok);  \
                            break;                  \
                        }

                        CASE_CH('[', JsonToken::BArray)
                        CASE_CH(']', JsonToken::EArray)
                        CASE_CH('{', JsonToken::BObject)
                        CASE_CH('}', JsonToken::EObject)
                        CASE_CH(',', JsonToken::Comma)
                        CASE_CH(':', JsonToken::Colon)
                        #undef CASE_CH

                        case 'n':
                        {
                            _current_token = JsonToken::Null;
                            _token_kw = s_keywords[0];
                            ++_token_position;
                            break;
                        }
                        case 't':
                        {
                            _current_token = JsonToken::True;
                            _token_kw = s_keywords[1];
                            ++_token_position;
                            break;
                        }
                        case 'f':
                        {
                            _current_token = JsonToken::False;
                            _token_kw = s_keywords[2];
                            ++_token_position;
                            break;
                        }
                        case '"':
                        {
                            _current_token = JsonToken::String;
                            break;
                        }
                        default:
                        {
                            // Only ASCII numbers supported
                            if (_ch == '-' || _ch == '+' || (_ch >= '0' && _ch <= '9'))
                            {
                                _current_token = JsonToken::Number;
                                
                                if (_ch != '+' && _ch != '-')
                                    _token_str.push_back('+'); // Explicit sign for parse<i32>
                                
                                _token_str.push_back(_ch);
                                ++_token_position;
                            }
                            else
                            {
                                _tokens.push_back(JsonToken::Error);
                                return JsonError{"Syntax error: unexpected character", _pos};
                            }
                        }
                    }
                }
                else
                {
                    if (_token_kw)
                    {
                        if (_ch != _token_kw[_token_position])
                        {
                            // Error and recover
                            _tokens.push_back(JsonToken::Error);
                            _current_token = JsonToken::Empty;
                            _token_position = 0;
                            _token_kw = nullptr;
                            return JsonError{"Syntax error: unexpected character", _pos};
                        }
                        
                        ++_token_position;
                        
                        if (_token_kw[_token_position] == 0)
                        {
                            _tokens.push_back(_current_token);
                            _current_token = JsonToken::Empty;
                            _token_position = 0;
                            _token_kw = nullptr;
                        }
                    }
                    else if (_current_token == JsonToken::String)
                    {
                        // handle escape sequences
                        if (_ch == '"')
                        {
                            _tokens.push_back(_current_token);
                            _qtok_string.push_back(hsd::move(_token_str));
                            _current_token = JsonToken::Empty;
                            _token_position = 0;
                            _token_str.clear();
                        }
                        else
                        {
                            ++_token_position;
                            _token_str.push_back(_ch);
                        }
                    }
                    else if (_current_token == JsonToken::Number)
                    {
                        if (_ch >= '0' and _ch <= '9')
                        {
                            _token_position++;
                            _token_str.push_back(_ch);
                        }
                        else if (_ch == '.')
                        {
                            if (_floating == true)
                            {
                                return JsonError{"Syntax error: unexpected character", _pos};
                            }

                            _floating = true;
                            _token_position++;
                            _token_str.push_back(_ch);
                        }
                        else if (_ch == 'e' || _ch == 'E')
                        {
                            return JsonError{"Work in progress: exponential notation", _pos};
                        }
                        else
                        {
                            _tokens.push_back(_current_token);
                            
                            if (_floating == true)
                            {
                                if (_token_str.back() == '.')
                                    _token_str.push_back('0');

                                _floating = false;
                                _qtok_number.emplace_back(
                                    cstring::template parse<f128>(_token_str.c_str())
                                );
                            }
                            else
                            {
                                _qtok_number.emplace_back(
                                    cstring::template parse<i64>(_token_str.c_str())
                                );
                            }
                            
                            _current_token = JsonToken::Empty;
                            _token_position = 0;
                            _token_str.clear();

                            goto _reparse;
                        }
                    }
                }
            }

            return {};
        }

        /// @brief Lex an entire file in fixed-size chunks.
        /// @details Opens `filename`, streams its contents through io::load_file
        /// in 1024-byte chunks, and forwards each chunk to lex().
        /// @param filename Path of the file to read and lex.
        /// @return Empty on success, or a JsonError if the file could not be
        /// opened or a syntax error was found while lexing.
        inline option_err<JsonError> lex_file(string_view filename)
        {
            //static const char* const s_keywords[] = {"null", "true", "false"};
            auto _res  = io::load_file(filename.data());
            
            if (_res.is_ok() == false)
            {
                return JsonError{"Couldn't open file", static_cast<usize>(-1)};
            }
            
            auto _stream = hsd::move(_res.unwrap());
            _stream.set_stream_capacity(1024);

            while (_stream.is_eof() == false)
            {
                string_view _chunk = _stream.read_chunk().unwrap().get().get_stream_data();
                
                auto _res = lex(_chunk);
                
                if (_res.is_ok() == false)
                {
                    return _res.unwrap_err();
                }
            }

            return {};
        }

        /// @brief Finalize the token stream and flush any pending token.
        /// @details Emits the last in-progress Number token if one exists,
        /// then appends JsonToken::Eof. Call once after all input has been
        /// passed to lex()/lex_file().
        /// @return Empty on success, or a JsonError if input ended in the
        /// middle of an incomplete token (e.g. an unterminated string).
        constexpr option_err<JsonError> push_eot()
        {
            ++_pos;
            
            if (_current_token == JsonToken::Number)
            {
                _tokens.push_back(_current_token);
                
                if (_token_str.find('.') != str::npos)
                {
                    _qtok_number.emplace_back(
                        cstring::template parse<f128>(_token_str.c_str())
                    );
                }
                else
                {
                    _qtok_number.emplace_back(
                        cstring::template parse<i64>(_token_str.c_str())
                    );
                }
                
                _current_token = JsonToken::Empty;
                _token_position = 0;
                _token_str.clear();
            }
            if (_current_token != JsonToken::Empty)
            {
                _tokens.push_back(JsonToken::Error);
                return JsonError{"Syntax error: unexpected end of transmission", _pos};
            }
            
            _tokens.push_back(JsonToken::Eof);
            return {};
        }
    };

    /// @brief Runtime categories represented by a JSON value node.
    enum class JsonValueType
    {
        Null,   ///< JSON `null`.
        True,   ///< JSON `true`.
        False,  ///< JSON `false`.
        Number, ///< Integer or floating-point number.
        String, ///< Quoted string.
        Object, ///< Key/value map (JsonObject).
        Array   ///< Ordered sequence of values (JsonArray).
    };

    /// @brief Polymorphic base for parsed JSON values.
    /// @details Provides checked typed access, object/array indexing, and
    /// numeric/string conversion shared by every concrete JSON node.
    class JsonValue
    {
    public:
        constexpr virtual ~JsonValue() = default;

        /// @brief Get the runtime category of this node.
        /// @return The JsonValueType discriminating the concrete subclass.
        constexpr virtual JsonValueType type() const noexcept = 0;

        /// @brief Whether this node was fully parsed from complete input.
        /// @return `true` for all concrete nodes except JsonPendingValue.
        constexpr virtual bool is_complete() const noexcept { return true; };

        /// @brief Unchecked downcast to a concrete JsonValue subclass.
        /// @tparam Class Concrete subclass to cast to; caller must ensure
        /// type() matches, as no runtime check is performed.
        /// @return Reference to `*this` viewed as `Class`.
        template <typename Class>
        constexpr Class& as() { return static_cast<Class&>(*this); }

        /// @brief Checked downcast to a concrete JsonValue subclass.
        /// @tparam Class Concrete subclass to cast to.
        /// @param t Expected JsonValueType that `Class` represents.
        /// @return Reference to `*this` as `Class` on success, or a
        /// runtime_error if type() does not equal `t`.
        template <typename Class>
        constexpr result<reference<Class>, runtime_error> try_as(JsonValueType t)
        {
            if (type() == t)
            {
                return {static_cast<Class&>(*this)};
            }
            else
            {
                return runtime_error{"Cast to wrong type"};
            }
        }

        /// @brief View this node as a JSON string.
        /// @return The string's contents, or a runtime_error if type() is
        /// not JsonValueType::String.
        constexpr result<string_view, runtime_error> as_str();

        /// @brief View this node as a JSON boolean.
        /// @return `true`/`false`, or a runtime_error if type() is neither
        /// JsonValueType::True nor JsonValueType::False.
        constexpr result<bool, runtime_error> as_bool();

        /// @brief View this node as a numeric value, converting as needed.
        /// @tparam Number Target integral or floating-point type.
        /// @return The value converted to `Number`, or a runtime_error if
        /// type() is not JsonValueType::Number, the stored alternative
        /// doesn't match `Number`'s category, or the value is out of range
        /// for `Number`.
        template <NumericType Number>
        constexpr result<Number, runtime_error> as_num();

        /// @brief View this node as a JSON object's underlying map.
        /// @return Reference to the key/value storage. Aborts (via
        /// unwrap()) if type() is not JsonValueType::Object.
        constexpr auto& as_object();

        /// @brief View this node as a JSON array's underlying storage.
        /// @return Reference to the element vector. Aborts (via
        /// unwrap()) if type() is not JsonValueType::Array.
        constexpr auto& as_array();

        /// @brief Look up a member of this node treated as a JSON object.
        /// @param key Member name to look up.
        /// @return Reference to the member value, or a runtime_error if
        /// this node is not an object or `key` is not present.
        constexpr result<reference<JsonValue>, runtime_error> access(const string_view& key);

        /// @brief Look up an element of this node treated as a JSON array.
        /// @param index Zero-based element index.
        /// @return Reference to the element, or a runtime_error if this
        /// node is not an array or `index` is out of bounds.
        constexpr result<reference<JsonValue>, runtime_error> access(usize index);

        /// @brief Unchecked member lookup for a JSON object.
        /// @param key Member name to look up.
        /// @return Reference to the member value.
        /// @note Aborts (via unwrap()) if this node is not an object or
        /// `key` is not present; prefer access() to handle failure.
        constexpr JsonValue& operator[](const string_view& key);

        /// @brief Unchecked element lookup for a JSON array.
        /// @param index Zero-based element index.
        /// @return Reference to the element.
        /// @note Aborts (via unwrap()) if this node is not an array or
        /// `index` is out of bounds; prefer access() to handle failure.
        constexpr JsonValue& operator[](usize index);
    };

    /// @brief Placeholder for a JSON value whose input is incomplete.
    /// @details Reserved for streaming/partial parsing support; not yet
    /// constructed or exposed by JsonParser (work in progress).
    class JsonPendingValue : public JsonValue
    {
        /// @brief Always reports incompleteness.
        /// @return `false`.
        constexpr bool is_complete() const noexcept override { return false; }
    };

    /// @brief Null, true, or false JSON primitive.
    /// @details Represents the three keyword literals as a single node type
    /// discriminated by an internally-stored JsonValueType; construct via
    /// the mk_null()/mk_true()/mk_false() factories.
    class JsonPrimitive : public JsonValue
    {
        JsonValueType _t; ///< Which literal this node represents (Null, True, or False only).

        /// @brief Construct a primitive of the given kind.
        /// @param t Must be JsonValueType::Null, True, or False.
        constexpr JsonPrimitive(JsonValueType t) : _t(t) {}

    public:
        /// @return The stored primitive kind (Null, True, or False).
        constexpr JsonValueType type() const noexcept override
        {
            return _t;
        }

        /// @brief Create a `null` primitive node.
        /// @return A JsonPrimitive with type() == JsonValueType::Null.
        static constexpr JsonPrimitive mk_null() { return {JsonValueType::Null}; }

        /// @brief Create a `true` primitive node.
        /// @return A JsonPrimitive with type() == JsonValueType::True.
        static constexpr JsonPrimitive mk_true() { return {JsonValueType::True}; }

        /// @brief Create a `false` primitive node.
        /// @return A JsonPrimitive with type() == JsonValueType::False.
        static constexpr JsonPrimitive mk_false() { return {JsonValueType::False}; }
    };

    /// @brief JSON integer or floating-point number node.
    class JsonNumber : public JsonValue
    {
        variant<i64, f128> _value; ///< Owning storage for either the integer or float alternative.

    public:
        /// @brief Construct from an already-parsed numeric variant.
        /// @param v Integer or floating-point value to take ownership of.
        constexpr explicit JsonNumber(variant<i64, f128>&& v)
            : _value{hsd::move(v)}
        {}

        /// @return JsonValueType::Number.
        constexpr JsonValueType type() const noexcept override
        {
            return JsonValueType::Number;
        }

        /// @brief Access the stored numeric variant.
        /// @return Reference to the `i64`/`f128` variant.
        constexpr auto& value() const { return _value; }
    };

    /// @brief Owning JSON string node.
    class JsonString : public JsonValue
    {
        string _value; ///< Owning storage for the decoded string contents.

    public:
        /// @brief Take ownership of an already-decoded string.
        /// @param v String contents to move from.
        explicit JsonString(string&& v)
            : _value{hsd::move(v)}
        {}

        /// @return JsonValueType::String.
        JsonValueType type() const noexcept override
        {
            return JsonValueType::String;
        }

        /// @brief View the stored string contents.
        /// @return Non-owning view valid for the lifetime of this node.
        string_view value() const
        {
            return static_cast<string_view>(_value);
        }
    };

    /// @brief Owning ordered JSON array node.
    class JsonArray : public JsonValue
    {
        vector<unique_ptr<JsonValue>> _values; ///< Owning, index-ordered storage of child nodes.

    public:
        /// @brief Take ownership of an already-built element vector.
        /// @param v Vector of owned child nodes, in array order.
        explicit JsonArray(vector<unique_ptr<JsonValue>>&& v)
            : _values{hsd::move(v)}
        {}

        /// @return JsonValueType::Array.
        JsonValueType type() const noexcept override
        {
            return JsonValueType::Array;
        }

        /// @brief Access the underlying element storage.
        /// @return Reference to the owning vector of child nodes.
        auto& values()
        {
            return _values;
        }
    };

    /// @brief Owning JSON object node keyed by strings.
    class JsonObject : public JsonValue
    {
        unordered_map<string, unique_ptr<JsonValue>> _values; ///< Owning storage mapping member names to child nodes.

    public:
        /// @brief Take ownership of an already-built member map.
        /// @param v Map of owned child nodes keyed by member name.
        explicit JsonObject(unordered_map<string, unique_ptr<JsonValue>>&& v)
            : _values{hsd::move(v)}
        {}

        /// @return JsonValueType::Object.
        JsonValueType type() const noexcept override
        {
            return JsonValueType::Object;
        }

        /// @brief Access the underlying member storage.
        /// @return Reference to the owning name-to-node map.
        auto& values()
        {
            return _values;
        }
    };

    constexpr auto JsonValue::as_str()
        -> result<string_view, runtime_error>
    {
        auto _res = try_as<JsonString>(JsonValueType::String);

        if (_res)
        {
            return _res.unwrap().get().value();
        }
        else
        {
            return _res.unwrap_err();
        }
    }

    constexpr result<bool, runtime_error> JsonValue::as_bool()
    {
        if (type() == JsonValueType::True)
        {
            return true;
        }
        else if (type() == JsonValueType::False)
        {
            return false;
        }

        // throw
        return runtime_error{"Cast to wrong type"};
    }

    template <NumericType Number>
    constexpr result<Number, runtime_error> JsonValue::as_num()
    {
        auto _res = try_as<JsonNumber>(JsonValueType::Number);

        if (_res)
        {
            if constexpr (IsIntegral<Number>)
            {
                auto _val = _res.unwrap().get().value().template get<i64>();

                if (!_val)
                {
                    return runtime_error{"Wrong numeric type selected"};
                }
                
                auto _num = _val.unwrap();

                if (_num < limits<Number>::min or _num > limits<Number>::max)
                {
                    return runtime_error{"Number out of range"};
                }

                return static_cast<Number>(_num);
            }
            else if constexpr (IsFloat<Number>)
            {
                auto _val = _res.unwrap().get().value().template get<f128>();

                if (!_val)
                {
                    return runtime_error{"Wrong numeric type selected"};
                }

                auto _num = _val.unwrap();

                if (_num < limits<Number>::min or _num > limits<Number>::max)
                {
                    return runtime_error{"Number out of range"};
                }

                return static_cast<Number>(_num);
            }
            else
            {
                return runtime_error{"Wrong numeric type selected"};
            }
        }
        else
        {
            return _res.unwrap_err();
        }
    }

    constexpr auto& JsonValue::as_array()
    {
        return try_as<JsonArray>(JsonValueType::Array).unwrap().get().values();
    }

    constexpr auto& JsonValue::as_object()
    {
        return try_as<JsonObject>(JsonValueType::Object).unwrap().get().values();
    }

    constexpr auto JsonValue::access(const string_view& key)
        -> result<reference<JsonValue>, runtime_error>
    {
        auto _res = try_as<JsonObject>(JsonValueType::Object);

        if (_res)
        {
            auto _value = _res.unwrap().get().values().at(key);
            if (_value.is_ok())
                return {*_value.unwrap().get()};

            return runtime_error{"JSON object member not found"};
        }

        return _res.unwrap_err();
    }

    constexpr auto JsonValue::access(usize index)
        -> result<reference<JsonValue>, runtime_error>
    {
        auto _res = try_as<JsonArray>(JsonValueType::Array);

        if (_res)
        {
            auto _value = _res.unwrap().get().values().at(index);
            if (_value.is_ok())
                return {*_value.unwrap().get()};

            return runtime_error{"JSON array index out of range"};
        }

        return _res.unwrap_err();
    }

    constexpr JsonValue& JsonValue::operator[](const string_view& key)
    {
        return access(key).unwrap();
    }

    constexpr JsonValue& JsonValue::operator[](usize index)
    {
        return access(index).unwrap();
    }

    /// @brief Adapter that consumes tokens from a JsonStream.
    /// @details Provides a cursor-style view (next/peek/skip/empty) over the
    /// token queue and forwards payload retrieval to the underlying stream;
    /// used by JsonParser to avoid depending on JsonStream's storage directly.
    class JsonTokenIterator
    {
        JsonStream& _stream; ///< Non-owning reference to the token/payload source.

    public:
        /// @brief Wrap a stream for sequential token consumption.
        /// @param stream Stream to read from; must outlive this iterator.
        constexpr JsonTokenIterator(JsonStream& stream)
            : _stream(stream) {}

        /// @brief Remove and return the next token.
        /// @return The token that was at the front of the queue.
        constexpr JsonToken next()
        {
            JsonToken tk = _stream.get_tokens().front();
            _stream.get_tokens().pop_front();
            return tk;
        }

        /// @brief Look at the next token without consuming it.
        /// @return The token at the front of the queue.
        constexpr JsonToken peek()
        {
            return _stream.get_tokens().front();
        }

        /// @brief Discard the next token without returning it.
        constexpr void skip()
        {
            _stream.get_tokens().pop_front();
        }

        /// @return `true` if no tokens remain in the queue.
        constexpr bool empty() const
        {
            return _stream.get_tokens().empty();
        }

        /// @brief Remove and return the next buffered number payload.
        /// @details Must only be called immediately after consuming a
        /// JsonToken::Number via next().
        /// @return The parsed integer or floating-point value.
        constexpr auto next_number()
        {
            return _stream.pop_number();
        }

        /// @brief Remove and return the next buffered string payload.
        /// @details Must only be called immediately after consuming a
        /// JsonToken::String via next().
        /// @return The decoded string contents.
        constexpr auto next_string()
        {
            return _stream.pop_string();
        }
    };

    /// @brief Parser that builds a JSON value tree from lexer tokens.
    /// @details Consumes tokens on demand via a JsonTokenIterator and
    /// recursively builds one owned JsonValue subtree per call to
    /// parse_next(); does not buffer the whole document ahead of time.
    class JsonParser
    {
        JsonTokenIterator _stream; ///< Cursor over the token queue supplied by the associated JsonStream.

    public:
        /// @brief Construct a parser reading from an existing token stream.
        /// @param s Stream to consume tokens from; must outlive this parser.
        JsonParser(JsonStream& s) : _stream(s) {}

        /// @brief Parse the next complete JSON value from the token stream.
        /// @details Recursively parses nested arrays/objects; each call
        /// consumes exactly the tokens belonging to one value.
        /// @return An owned JsonValue tree on success, or a JsonError if the
        /// stream is empty, ends unexpectedly, or contains a syntax error.
        /// @note Streaming/partial values (JsonPendingValue) and objects or
        /// arrays split across lex() calls are not yet supported; such
        /// cases currently report a "Work in progress" JsonError instead.
        result<unique_ptr<JsonValue>, JsonError> parse_next()
        {
            if (_stream.empty())
                //return JsonPendingValue::make();
                return JsonError{"Work in progress: pending values not supported", 0};

            JsonToken _tok = _stream.next();
            switch (_tok)
            {
                case JsonToken::Eof:
                    return JsonError{"Unexpected EOF", 0};
                case JsonToken::Null:
                    return {make_unique<JsonPrimitive>(JsonPrimitive::mk_null())};
                case JsonToken::True:
                    return {make_unique<JsonPrimitive>(JsonPrimitive::mk_true())};
                case JsonToken::False:
                    return {make_unique<JsonPrimitive>(JsonPrimitive::mk_false())};
                case JsonToken::Number:
                    return {make_unique<JsonNumber>(_stream.next_number())};
                case JsonToken::String:
                    return {make_unique<JsonString>(_stream.next_string())};
                case JsonToken::BArray:
                {
                    vector<unique_ptr<JsonValue>> _array;
                    
                    if (_stream.empty())
                    {
                        _partial_arr:
                        return JsonError{"Work in progress: partial arrays", 0};
                    }

                    if (_stream.peek() != JsonToken::EArray)
                    {
                        while (true)
                        {
                            auto _res = parse_next();
                            
                            if (!_res)
                            {
                                return _res;
                            }
                            
                            _array.push_back(_res.expect());
                            
                            if (_stream.empty())
                            {
                                goto _partial_arr;
                            }
                            if (_stream.peek() == JsonToken::EArray)
                            {
                                _stream.skip();
                                break;
                            }
                            if (_stream.next() != JsonToken::Comma)
                                return JsonError{"Syntax error: expected a comma while parsing array", 0};
                        }
                    }
                    else
                    {
                        _stream.skip();
                    }
                    
                    return {make_unique<JsonArray>(hsd::move(_array))};
                }
                case JsonToken::BObject:
                {
                    unordered_map<string, unique_ptr<JsonValue>> _map;
                    
                    if (_stream.empty())
                    {
                        _partial_obj:
                        return JsonError{"Work in progress: partial objects", 0};
                    }
                    if (_stream.peek() != JsonToken::EObject)
                    {
                        while (true)
                        {
                            if (_stream.next() != JsonToken::String)
                            {
                                return JsonError{"Syntax error: expected string name", 0};
                            }
                           
                            auto _name = _stream.next_string();
                            
                            if (_stream.empty())
                            {
                                goto _partial_obj;
                            }
                            if (_stream.next() != JsonToken::Colon)
                            {
                                return JsonError{"Syntax error: expected a colon", 0};
                            }
                            
                            auto _res = parse_next();

                            if (!_res)
                            {
                                return _res;
                            }

                            _map.emplace(hsd::move(_name), _res.expect());
                            
                            if (_stream.peek() == JsonToken::EObject)
                            {
                                _stream.skip();
                                break;
                            }
                            if (_stream.next() != JsonToken::Comma)
                            {
                                return JsonError{"Syntax error: expected a comma while parsing object", 0};
                            }
                            if (_stream.empty())
                            {
                                goto _partial_obj;
                            }
                        }
                    }
                    else
                    {
                        _stream.skip();
                    }
                    return {make_unique<JsonObject>(hsd::move(_map))};
                }
                default:
                    return JsonError{"Syntax error: unexpected token", static_cast<usize>(_tok)};
            }
        }
    };
}
