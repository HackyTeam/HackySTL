#pragma once

/// @brief JSON stream parser

#include "../List.hpp"
#include "../String.hpp"
#include "../UniquePtr.hpp"
#include "../UnorderedMap.hpp"
#include "../Variant.hpp"
#include "Reference.hpp"
#include "Result.hpp"
#include "Types.hpp"

namespace hsd
{
    namespace json_detail
    {
        template <typename CharT, auto GetC>
        inline auto read_file_impl(FILE* stream)
        {
            constexpr usize _buf_size = 1024;
            static CharT _s_buf[_buf_size];

            usize _size;
            for (_size = 0; _size < _buf_size;)
            {
                auto _ch = GetC(stream);
                
                if (_ch == static_cast<decltype(_ch)>(EOF))
                    break;

                _s_buf[_size++] = static_cast<CharT>(_ch);
            }
            return basic_string_view<CharT>(_s_buf, _size);
        }

        template <typename CharT>
        inline auto read_file(FILE* stream)
        {
            if constexpr (is_same<CharT, char>::value)
                return read_file_impl<CharT, &fgetc>(stream);
            #if defined(HSD_PLATFORM_POSIX)
            else if constexpr (is_same<CharT, wchar>::value)
                return read_file_impl<CharT, &fgetwc>(stream);
            else
                static_assert(
                    is_same<CharT, char>::value or 
                    is_same<CharT, wchar>::value, 
                    "File IO only implemented for char & wchar"
                );
            #else
            else
                static_assert(
                    is_same<CharT, char>::value, 
                    "File IO only implemented for char on windows"
                );
            #endif
        }
    } // namespace json_detail

    enum class JsonToken
    {
        Null, True, False, Number, String, BArray, 
        EArray, BObject, EObject, Comma, Colon,

        Eof = -1,
        Error = -2,
        Empty = -3 // For internal purposes
    };

    class JsonError : public runtime_error
    {
        hsd::string _str;

    public:
        usize position;

        JsonError(const char* msg, usize pos)
            : runtime_error{msg}, position{pos} {}

        const char* operator()()
        {
            if (_str.size() != 0)
                return _str.c_str();

            return (_str = _err + (" " + to_string(position))).c_str();
        }
    };

    template <typename CharT>
    class JsonStream
    {
        using vstr = basic_string_view<CharT>;
        using str = basic_string<CharT>;
        using num = variant<i64, f128>;

        list<pair<JsonToken, usize>> _tokens;
        list<str> _qtok_string;
        list<num> _qtok_number;
        JsonToken _current_token = JsonToken::Empty;
        const char* _token_kw = nullptr;
        usize _token_position = 0;
        str _token_str;

        usize _pos = 0;
        usize _begin_pos = 0;

    public:
        auto& get_tokens() { return _tokens; }

        str pop_string()
        {
            str _s = move(_qtok_string.front());
            _qtok_string.pop_front();
            return _s;
        }

        num pop_number()
        {
            num _n = move(_qtok_number.front());
            _qtok_number.pop_front();
            return _n;
        }

        // If an error occurs, the rest of buffer can be passed in
        Result<void, JsonError> lex(vstr frag)
        {
            static const char* const s_keywords[] = {"null", "true", "false"};
            for (bool _floating = false; CharT _ch : frag)
            {
                ++_pos;
                
                _reparse:
                if (_current_token == JsonToken::Empty)
                {
                    if (basic_cstring<CharT>::iswhitespace(_ch))
                        continue;
                    
                    switch (_ch)
                    {
                        #define CASE_CH(ch, tok)    \
                        case static_cast<CharT>(ch):\
                        {                           \
                            _tokens.emplace_back(tok, _pos - 1);  \
                            break;                  \
                        }

                        CASE_CH('[', JsonToken::BArray)
                        CASE_CH(']', JsonToken::EArray)
                        CASE_CH('{', JsonToken::BObject)
                        CASE_CH('}', JsonToken::EObject)
                        CASE_CH(',', JsonToken::Comma)
                        CASE_CH(':', JsonToken::Colon)
                        #undef CASE_CH

                        case static_cast<CharT>('n'):
                        {
                            _current_token = JsonToken::Null;
                            _token_kw = s_keywords[0];
                            ++_token_position;
                            _begin_pos = _pos - 1;
                            break;
                        }
                        case static_cast<CharT>('t'):
                        {
                            _current_token = JsonToken::True;
                            _token_kw = s_keywords[1];
                            ++_token_position;
                            _begin_pos = _pos - 1;
                            break;
                        }
                        case static_cast<CharT>('f'):
                        {
                            _current_token = JsonToken::False;
                            _token_kw = s_keywords[2];
                            ++_token_position;
                            _begin_pos = _pos - 1;
                            break;
                        }
                        case static_cast<CharT>('"'):
                        {
                            _current_token = JsonToken::String;
                            _begin_pos = _pos - 1;
                            break;
                        }
                        default:
                        {
                            // Only ASCII numbers supported
                            if (_ch == static_cast<CharT>('-') or _ch == static_cast<CharT>('+') or
                                (_ch >= static_cast<CharT>('0') and _ch <= static_cast<CharT>('9')))
                            {
                                _current_token = JsonToken::Number;
                                _begin_pos = _pos - 1;
                                
                                if (_ch != static_cast<CharT>('+') and _ch != static_cast<CharT>('-'))
                                    _token_str.push_back('+'); // Explicit sign for parse<i32>
                                
                                _token_str.push_back(_ch);
                                ++_token_position;
                            }
                            else
                            {
                                _tokens.emplace_back(JsonToken::Error, _pos);
                                return JsonError{"Syntax error: unexpected character", _pos};
                            }
                        }
                    }
                }
                else
                {
                    if (_token_kw)
                    {
                        if (_ch != static_cast<CharT>(_token_kw[_token_position]))
                        {
                            // Error and recover
                            _tokens.emplace_back(JsonToken::Error, _pos);
                            _current_token = JsonToken::Empty;
                            _token_position = 0;
                            _token_kw = nullptr;
                            return JsonError{"Syntax error: unexpected character", _pos};
                        }
                        
                        ++_token_position;
                        
                        if (_token_kw[_token_position] == 0)
                        {
                            _tokens.emplace_back(_current_token, _begin_pos);
                            _current_token = JsonToken::Empty;
                            _token_position = 0;
                            _token_kw = nullptr;
                        }
                    }
                    else if (_current_token == JsonToken::String)
                    {
                        // handle escape sequences
                        if (_ch == static_cast<CharT>('"'))
                        {
                            _tokens.emplace_back(_current_token, _begin_pos);
                            _qtok_string.push_back(move(_token_str));
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
                        if (_ch >= static_cast<CharT>('0') and _ch <= static_cast<CharT>('9'))
                        {
                            _token_position++;
                            _token_str.push_back(_ch);
                        }
                        else if (_ch == static_cast<CharT>('.'))
                        {
                            if (_floating == true)
                            {
                                return JsonError{"Syntax error: unexpected character", _pos};
                            }

                            _floating = true;
                            _token_position++;
                            _token_str.push_back(_ch);
                        }
                        else if (_ch == static_cast<CharT>('e') or _ch == static_cast<CharT>('E'))
                        {
                            return JsonError{"Unimplemented: exponential notation", _pos};
                        }
                        else
                        {
                            _tokens.emplace_back(_current_token, _begin_pos);
                            
                            if (_floating == true)
                            {
                                if (_token_str.back() == static_cast<CharT>('.'))
                                    _token_str.push_back(static_cast<CharT>('0'));

                                _floating = false;
                                _qtok_number.emplace_back(
                                    basic_cstring<CharT>::template parse<f128>(_token_str.c_str())
                                );
                            }
                            else
                            {
                                _qtok_number.emplace_back(
                                    basic_cstring<CharT>::template parse<i64>(_token_str.c_str())
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

        // If an error occurs, the rest of buffer can be passed in
        Result<void, JsonError> lex_file(string_view filename)
        {
            //static const char* const s_keywords[] = {"null", "true", "false"};
            auto* _stream = fopen(filename.data(), "r");
            
            if (!_stream)
                return JsonError{"Couldn't open file", static_cast<usize>(-1)};
            
            auto& _read_chunk = json_detail::read_file<CharT>;

            while (true)
            {
                auto _chunk = _read_chunk(_stream);
                
                if (_chunk.size() == 0)
                    break;
                
                auto _res = lex(_chunk);
                
                if (!_res)
                {
                    fclose(_stream);
                    return _res.unwrap_err();
                }
            }
            
            fclose(_stream);
            return {};
        }

        // End the stream of tokens
        Result<void, JsonError> push_eot()
        {
            ++_pos;
            
            if (_current_token == JsonToken::Number)
            {
                _tokens.emplace_back(_current_token, _begin_pos);
                
                if (_token_str.find(static_cast<CharT>('.')) != str::npos)
                {
                    _qtok_number.emplace_back(
                        basic_cstring<CharT>::template parse<f128>(_token_str.c_str())
                    );
                }
                else
                {
                    _qtok_number.emplace_back(
                        basic_cstring<CharT>::template parse<i64>(_token_str.c_str())
                    );
                }
                
                _current_token = JsonToken::Empty;
                _token_position = 0;
                _token_str.clear();
            }
            if (_current_token != JsonToken::Empty)
            {
                _tokens.emplace_back(JsonToken::Error, _begin_pos);
                return JsonError{"Syntax error: unexpected end of transmission", _pos};
            }
            
            _tokens.emplace_back(JsonToken::Eof, _pos);
            return {};
        }
    };

    enum class JsonValueType
    {
        Null, True, False, Number,
        String, Array, Object,
        StreamingArray, StreamingObject,
    };

    template <typename CharT>
    class JsonParser;

    class JsonValue
    {
    public:
        virtual ~JsonValue() = default;
        virtual JsonValueType type() const noexcept = 0;
        virtual bool is_complete() const noexcept { return true; };

        template <typename Class>
        Class& as() { return static_cast<Class&>(*this); }

        template <typename Class>
        Result<reference<Class>, runtime_error> try_as(JsonValueType t)
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

        template <typename CharT>
        Result<basic_string_view<CharT>, runtime_error> as_str();
        Result<bool, runtime_error> as_bool();

        template <NumericType Number>
        Result<Number, runtime_error> as_num();

        template <typename CharT>
        auto& as_object();
        auto& as_array();

        template <typename CharT>
        Result<reference<JsonValue>, runtime_error> access(const basic_string_view<CharT>& key);
        Result<reference<JsonValue>, runtime_error> access(usize index);

        template <typename CharT>
        JsonValue& operator[](const basic_string_view<CharT>& key);
        JsonValue& operator[](usize index);

        // Recursively finish the partial value
        template <typename CharT>
        static Result<void, runtime_error> finish(unique_ptr<JsonValue>& value);
    };

    // Streaming pending value, todo
    class JsonPendingValue : public JsonValue
    {
        protected:
        bool _complete = false;

        public:
        bool is_complete() const noexcept override final { return _complete; }
    };

    class JsonPrimitive : public JsonValue
    {
        JsonValueType _t; // only Null, True & False allowed

        JsonPrimitive(JsonValueType t) : _t(t) {}

    public:
        JsonValueType type() const noexcept override
        {
            return _t;
        }

        static JsonPrimitive mk_null() { return {JsonValueType::Null}; }
        static JsonPrimitive mk_true() { return {JsonValueType::True}; }
        static JsonPrimitive mk_false() { return {JsonValueType::False}; }
    };

    class JsonNumber : public JsonValue
    {
        variant<i64, f128> _value;

    public:
        explicit JsonNumber(variant<i64, f128>&& v)
            : _value{move(v)}
        {}

        JsonValueType type() const noexcept override
        {
            return JsonValueType::Number;
        }

        const auto& value() const { return _value; }
    };

    template <typename CharT>
    class JsonString : public JsonValue
    {
        basic_string<CharT> _value;

    public:
        explicit JsonString(basic_string<CharT>&& v)
            : _value{move(v)}
        {}

        JsonValueType type() const noexcept override
        {
            return JsonValueType::String;
        }

        basic_string_view<CharT> value() const
        {
            return static_cast<basic_string_view<CharT>>(_value);
        }
    };

    class JsonArray : public JsonValue
    {
        vector<unique_ptr<JsonValue>> _values;

    public:
        explicit JsonArray(vector<unique_ptr<JsonValue>>&& v)
            : _values{move(v)}
        {}

        JsonValueType type() const noexcept override
        {
            return JsonValueType::Array;
        }

        auto& values()
        {
            return _values;
        }
    };

    template <typename CharT>
    class JsonObject : public JsonValue
    {
        unordered_map<basic_string<CharT>, unique_ptr<JsonValue>> _values;

    public:
        explicit JsonObject(unordered_map<basic_string<CharT>, unique_ptr<JsonValue>>&& v)
            : _values{move(v)}
        {}

        JsonValueType type() const noexcept override
        {
            return JsonValueType::Object;
        }

        auto& values()
        {
            return _values;
        }
    };

    // Streaming values
    class JsonStreamingArray : public JsonPendingValue
    {
        vector<unique_ptr<JsonValue>> _values;

    public:
        JsonStreamingArray() = default;

        JsonValueType type() const noexcept override
        {
            return JsonValueType::StreamingArray;
        }

        usize current_size() const
        {
            return _values.size();
        }

        Result<reference<unique_ptr<JsonValue>>, runtime_error> access(usize idx)
        {
            if (idx >= current_size())
                return runtime_error{"Streaming array access out of bounds"};
            return _values[idx];
        }

        // Turns this instance into a complete JsonArray, if finished parsing
        // This object may not be used after successful completion
        Result<unique_ptr<JsonArray>, runtime_error> complete()
        {
            if (!_complete)
                return runtime_error{"Array couldn't be completed: end not reached"};
            return make_unique<JsonArray>(move(_values));
        }

    private:
        template <typename CharT>
        friend class JsonParser;

        void insert_element(unique_ptr<JsonValue>&& el)
        {
            _values.push_back(move(el));
        }

        void parse_done()
        {
            _complete = true;
        }
    };

    template <typename CharT>
    class JsonStreamingObject : public JsonPendingValue
    {
        unordered_map<basic_string<CharT>, unique_ptr<JsonValue>> _values;

    public:
        JsonStreamingObject() = default;

        JsonValueType type() const noexcept override
        {
            return JsonValueType::StreamingObject;
        }

        // Checks if a given key is currently in the object
        auto lookup(const basic_string<CharT>& key) const
        {
            return _values.find(key);
        }

        usize current_size() const
        {
            return _values.size();
        }

        // Turns this instance into a complete JsonObject, if finished parsing
        // This object may not be used after successful completion
        Result<unique_ptr<JsonObject<CharT>>, runtime_error>
        complete()
        {
            if (!_complete)
                return runtime_error{"Object couldn't be completed: end not reached"};
            return make_unique<JsonObject<CharT>>(move(_values));
        }

    private:
        friend class JsonParser<CharT>;

        bool insert_element(basic_string<CharT>&& key, unique_ptr<JsonValue>&& val)
        {
            return _values.emplace(move(key), move(val)).second;
        }

        void parse_done()
        {
            _complete = true;
        }
    };

    template <typename CharT>
    inline auto JsonValue::as_str()
        -> Result<basic_string_view<CharT>, runtime_error>
    {
        auto _res = try_as<JsonString<CharT>>(JsonValueType::String);

        if (_res)
        {
            return _res.unwrap().value();
        }
        else
        {
            return _res.unwrap_err();
        }
    }

    inline Result<bool, runtime_error> JsonValue::as_bool()
    {
        if (type() == JsonValueType::True)
        {
            return true;
        }
        else if (type() == JsonValueType::False)
        {
            return false;
        }

        return runtime_error{"Cast to wrong type"};
    }

    template <NumericType Number>
    Result<Number, runtime_error> JsonValue::as_num()
    {
        auto _res = try_as<JsonNumber>(JsonValueType::Number);

        if (_res)
        {
            if constexpr (IsIntegral<Number>)
            {
                auto _val = _res.unwrap().value().template get<i64>();

                if (!_val)
                {
                    return runtime_error{"Wrong numeric type selected"};
                }
                
                if (_val.unwrap() < std::numeric_limits<Number>::min() or
                    _val.unwrap() > std::numeric_limits<Number>::max())
                {
                    return runtime_error{"Number out of range"};
                }

                return static_cast<Number>(_val.unwrap());
            }
            else
            {
                auto _val = _res.unwrap().value().template get<f128>();

                if (!_val)
                {
                    return runtime_error{"Wrong numeric type selected"};
                }
                
                if (_val.unwrap() < std::numeric_limits<Number>::min() or
                    _val.unwrap() > std::numeric_limits<Number>::max())
                {
                    return runtime_error{"Number out of range"};
                }

                return static_cast<Number>(_val.unwrap());
            }
        }
        else
        {
            return _res.unwrap_err();
        }
    }

    inline auto& JsonValue::as_array()
    {
        return try_as<JsonArray>(JsonValueType::Array).unwrap().values();
    }

    template <typename CharT>
    inline auto& JsonValue::as_object()
    {
        return try_as<JsonObject<CharT>>(JsonValueType::Object).unwrap().values();
    }

    template <typename CharT>
    inline auto JsonValue::access(const basic_string_view<CharT>& key)
        -> Result<reference<JsonValue>, runtime_error>
    {
        auto _res = try_as<JsonObject<CharT>>(JsonValueType::Object);

        if(_res)
        {
            return *_res.unwrap().values().at(key).unwrap();
        }

        return _res.unwrap_err();
    }

    inline auto JsonValue::access(usize index)
        -> Result<reference<JsonValue>, runtime_error>
    {
        auto _res = try_as<JsonArray>(JsonValueType::Array);

        if(_res)
        {
            return *_res.unwrap().values().at(index).unwrap();
        }

        return _res.unwrap_err();
    }

    template <typename CharT>
    inline JsonValue& JsonValue::operator[](const basic_string_view<CharT>& key)
    {
        return access(key).unwrap();
    }

    inline JsonValue& JsonValue::operator[](usize index)
    {
        return access(index).unwrap();
    }

    template <typename CharT>
    inline Result<void, runtime_error> JsonValue::finish(unique_ptr<JsonValue>& value)
    {
        switch (value->type())
        {
            case JsonValueType::StreamingArray:
            {
                auto res = value->as<JsonStreamingArray>().complete();
                if (!res)
                    return res.unwrap_err();
                value = res.unwrap();
                for (auto& el : value->as_array())
                    if (auto res = finish<CharT>(el); !res)
                        return res.unwrap_err();
                break;
            }
            case JsonValueType::StreamingObject:
            {
                auto res = value->as<JsonStreamingObject<CharT>>().complete();
                if (!res)
                    return res.unwrap_err();
                value = res.unwrap();
                for (auto& el : value->as_object<CharT>())
                    if (auto res = finish<CharT>(el.second); !res)
                        return res.unwrap_err();
                break;
            }
            default:
                // Do nothing
                ;
        }
        return {};
    }

    template <typename CharT>
    class JsonTokenIterator
    {
        JsonStream<CharT>& _stream;

    public:
        JsonTokenIterator(JsonStream<CharT>& stream)
            : _stream(stream) {}

        JsonToken next(usize* pos = nullptr)
        {
            if (pos)
                *pos = _stream.get_tokens().front().second;
            JsonToken tk = _stream.get_tokens().front().first;
            _stream.get_tokens().pop_front();
            return tk;
        }

        JsonToken peek(usize* pos = nullptr)
        {
            if (pos)
                *pos = _stream.get_tokens().front().second;
            return _stream.get_tokens().front().first;
        }

        void skip()
        {
            _stream.get_tokens().pop_front();
        }

        bool empty() const
        {
            return _stream.get_tokens().empty();
        }

        auto next_number()
        {
            return _stream.pop_number();
        }

        auto next_string()
        {
            return _stream.pop_string();
        }
    };

    // Idk what I'm doing anymore ¯\_(ツ)_/¯
    struct JsonParsingToken
    {
        enum
        {
            TopValue, // Placeholder for root value (null target)
            //Value, // Placeholder for parsing a value
            Token, // Expecting a token `token` on next step
            ArrElem, // Array element or ']'
            ObjElem, // Object element or '}'
        } tag;
        union
        {
            JsonToken token;
            int state = 0;
        };
        JsonValue* target = nullptr;
    };

    template <typename CharT>
    class JsonParser
    {
        JsonTokenIterator<CharT> _stream;
        vector<JsonParsingToken> _stack;
        unique_ptr<JsonValue> _top;
        unique_ptr<JsonValue> _last_result;
        usize _pos = 0;
        basic_string<CharT> _cur_key;

    public:
        JsonParser(JsonStream<CharT>& s) : _stream(s) {}

        // Begins a partial parse. This means that parsing incomplete values (arrays and objects) will be possible,
        // and allows updates to be retrieved between parses
        unique_ptr<JsonValue>& parse_begin()
        {
            _stack.push_back(JsonParsingToken{JsonParsingToken::TopValue});
            return _top;
        }

        // Parses the whole document, without uncomplete nodes
        // The parser cannot be used after this operation
        // If `parse_begin` was called upon calling this function, all previous streaming references will be invalidated
        Result<unique_ptr<JsonValue>, JsonError> parse_all()
        {
            if (!_top)
                if (auto res = parse_resume(); !res)
                    return res.unwrap_err();

            if (auto res = JsonValue::finish<CharT>(_top); !res)
                return JsonError{res.unwrap_err()(), _pos};
            return move(_top);
        }

        // On success, returns whether the parsing is complete
        Result<bool, JsonError> parse_resume()
        {
            if (_stream.empty())
                return JsonError{"No tokens to parse", _pos};

            do
            {
                JsonToken _tok = _stream.next(&_pos);
                JsonParsingToken* ptok = nullptr;
                if (_stack.filled())
                {
                    ptok = &_stack.back();
                    switch (ptok->tag)
                    {
                        case JsonParsingToken::TopValue:
                            if (ptok->state == 1)
                                if (_tok != JsonToken::Eof)
                                    return JsonError{"Expected EOF", _pos};
                            [[fallthrough]];
                        case JsonParsingToken::Token:
                            if (_tok != ptok->token)
                                return JsonError{"Expected a different token", _pos};
                            _stack.pop_back();
                            continue;
                        case JsonParsingToken::ArrElem:
                            // WHYY
                            if (_tok == JsonToken::EArray and ptok->state != 2)
                            {
                                ptok->target->as<JsonStreamingArray>().parse_done();
                                _stack.pop_back();
                            }
                            else if (ptok->state == 1 and _tok != JsonToken::Comma)
                                return JsonError{"Expected a comma", _pos};
                            else
                                goto _parse_token;
                            ptok->state = 2;
                            continue;
                        case JsonParsingToken::ObjElem:
                            // WWHHYYYYYYYY
                            if (ptok->state == 0 or ptok->state == 3)
                            {
                                if (_tok == JsonToken::EObject)
                                {
                                    ptok->target->as<JsonStreamingObject<CharT>>().parse_done();
                                    _stack.pop_back();
                                }
                                else if (_tok != JsonToken::String)
                                    return JsonError{"Syntax error: expected string key", _pos};
                                _cur_key = _stream.next_string();
                                ptok->state = 1;
                            }
                            else if (ptok->state == 1)
                            {
                                if (_tok != JsonToken::Colon)
                                    return JsonError{"Expected a colon", _pos};
                                ptok->state = 2;
                                continue;
                            }
                            else if (ptok->state == 2)
                                goto _parse_token;
                            break;
                    }
                }
                else if (_tok == JsonToken::Eof)
                {
                    if (_top)
                        return true;
                    else
                        return JsonError{"Unexpected EOF", _pos};
                }
                else if (_top)
                {
                    return JsonError{"Expected EOF", _pos};
                }
                _parse_token:
                switch (_tok)
                {
                    case JsonToken::Eof:
                        break;
                    case JsonToken::Null:
                        _last_result = make_unique<JsonPrimitive>(JsonPrimitive::mk_null());
                        break;
                    case JsonToken::True:
                        _last_result = make_unique<JsonPrimitive>(JsonPrimitive::mk_true());
                        break;
                    case JsonToken::False:
                        _last_result = make_unique<JsonPrimitive>(JsonPrimitive::mk_false());
                        break;
                    case JsonToken::Number:
                        _last_result = make_unique<JsonNumber>(_stream.next_number());
                        break;
                    case JsonToken::String:
                        _last_result = make_unique<JsonString<CharT>>(_stream.next_string());
                        break;
                    case JsonToken::BArray:
                    {
                        if (_stack.empty())
                        {
                            vector<unique_ptr<JsonValue>> _array;
                            
                            if (_stream.empty())
                                goto _exhaust;

                            if (_stream.peek() != JsonToken::EArray)
                            {
                                while (true)
                                {
                                    if (auto _res = parse_resume(); !_res)
                                        return _res.unwrap_err();
                                    
                                    _array.push_back(move(_last_result));
                                    
                                    if (_stream.empty())
                                        goto _exhaust;
                                    if (_stream.peek() == JsonToken::EArray)
                                    {
                                        _stream.skip();
                                        break;
                                    }
                                    if (_stream.next(&_pos) != JsonToken::Comma)
                                        return JsonError{"Syntax error: expected a comma while parsing array", _pos};
                                }
                            }
                            return make_unique<JsonArray>(move(_array));
                        }
                        else
                        {
                            _last_result = make_unique<JsonStreamingArray>();
                            JsonParsingToken ptok{JsonParsingToken::ArrElem};
                            ptok.target = _last_result.get();
                            _stack.push_back(ptok);
                            break;
                        }
                    }
                    case JsonToken::BObject:
                    {
                        if (_stack.empty())
                        {
                            unordered_map<basic_string<CharT>, unique_ptr<JsonValue>> _map;
                            
                            if (_stream.empty())
                                goto _exhaust;
                            if (_stream.peek() != JsonToken::EObject)
                            {
                                while (true)
                                {
                                    if (_stream.next(&_pos) != JsonToken::String)
                                    {
                                        return JsonError{"Syntax error: expected string key", _pos};
                                    }
                                
                                    auto _name = _stream.next_string();
                                    
                                    if (_stream.empty())
                                        goto _exhaust;
                                    if (_stream.next(&_pos) != JsonToken::Colon)
                                        return JsonError{"Syntax error: expected a colon", _pos};
                                    
                                    if (auto _res = parse_resume(); !_res)
                                        return _res.unwrap_err();

                                    if (_map.emplace(move(_name), move(_last_result)).second)
                                        return JsonError{"Syntax error: duplicate keys", _pos};

                                    if (_stream.peek() == JsonToken::EObject)
                                    {
                                        _stream.skip();
                                        break;
                                    }
                                    if (_stream.next(&_pos) != JsonToken::Comma)
                                        return JsonError{"Syntax error: expected a comma while parsing object", _pos};

                                    if (_stream.empty())
                                        goto _exhaust;
                                }
                            }
                            return make_unique<JsonObject<CharT>>(move(_map));
                        }
                        else
                        {
                            _last_result = make_unique<JsonStreamingObject<CharT>>();
                            JsonParsingToken ptok{JsonParsingToken::ObjElem};
                            ptok.target = _last_result.get();
                            _stack.push_back(ptok);
                            break;
                        }
                    }
                    default:
                        return JsonError("Syntax error: unexpected token", static_cast<usize>(_tok));
                }
                if (ptok)
                {
                    switch (ptok->tag) {
                        case JsonParsingToken::TopValue:
                            _top = move(_last_result);
                            ptok->state = 1;
                            break;
                        case JsonParsingToken::ArrElem:
                            ptok->target->as<JsonStreamingArray>().insert_element(move(_last_result));
                            ptok->state = 1;
                            break;
                        case JsonParsingToken::ObjElem:
                            ptok->target->as<JsonStreamingObject<CharT>>().insert_element(move(_cur_key), move(_last_result));
                            ptok->state = 1;
                            break;
                        case JsonParsingToken::Token:
                            break;
                    }
                }
            } while (!_stream.empty());
            return false;

            _exhaust:
            return JsonError{"Token stream exhausted while parsing", _pos};
        }
    };

    class JsonArrayStreamIterator
    {
        JsonStreamingArray& arr;
        usize offs = 0;
    public:
        JsonArrayStreamIterator(JsonStreamingArray& arr)
            : arr(arr)
        {}

        unique_ptr<JsonValue>* next()
        {
            if (arr.current_size() > offs)
                return &arr.access(offs++).unwrap();
            return nullptr;
        }
    };
}
