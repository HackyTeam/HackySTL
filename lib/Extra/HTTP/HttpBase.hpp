#pragma once

/// @file HttpBase.hpp
/// @brief HTTP protocol enums and validation helpers.
/// @details Defines protocol versions, request methods, response statuses, and
/// shared parsing/status-message logic used by HTTP request and response types.

#include "../../Base/Switch.hpp"
#include "../../Containers/StringView.hpp"

namespace hsd::http
{
    /** @def HTTP_STATUS_CASE
     *  @brief Local helper expanding a response status into a switch case.
     */
    /// @brief HTTP protocol version negotiated between a request and a response.
    enum class protocol_version
    {
        ver_0_9, ///< HTTP/0.9 (original, header-less protocol).
        ver_1_0, ///< HTTP/1.0.
        ver_1_1, ///< HTTP/1.1.
        ver_2_0, ///< HTTP/2.0.
        ver_3_0  ///< HTTP/3.0.
    };

    /// @brief HTTP request method (verb) carried on a request line.
    enum class request_method : u8
    {
        get,     ///< `GET`: retrieve a resource.
        head,    ///< `HEAD`: retrieve headers only, no message body.
        post,    ///< `POST`: submit data to be processed by the resource.
        put,     ///< `PUT`: replace the target resource with the request body.
        del,     ///< `DELETE`: remove the target resource (named `del`, delete is a reserved keyword).
        connect, ///< `CONNECT`: establish a tunnel to the server.
        options, ///< `OPTIONS`: describe communication options for the resource.
        trace,   ///< `TRACE`: perform a message loop-back test along the path.
        patch    ///< `PATCH`: apply partial modifications to a resource.
    };

    /// @brief HTTP response status code.
    /// @details Values follow the standard HTTP status registry and are
    /// grouped by leading digit: 1xx informational, 2xx success, 3xx
    /// redirection, 4xx client error, 5xx server error. Numeric values match
    /// the wire status code, so this enum can be cast directly to/from `u16`.
    enum class response_status : u16
    {
        cont                   = 100,
        switching_protocols         ,
        processing                  ,
        early_hints                 ,
        ok                     = 200,
        created                     ,
        accepted                    ,
        non_author_info             ,
        no_content                  ,
        reset_content               ,
        partial_content             ,
        multi_status                ,
        already_reported            ,
        im_used                = 226,
        multiple_choices       = 300,
        moved_permanently           ,
        found                       ,
        see_other                   ,
        not_modified                ,
        use_proxy                   ,
        unused                      , // Does nothing
        temporary_redirect          ,
        permanent_redirect          ,
        bad_request            = 400,
        unauthorized                ,
        payment_required            , // Mozilla, WHY?
        forbidden                   ,
        not_found                   ,
        method_not_allowed          ,
        not_acceptable              ,
        proxy_auth_req              ,
        request_timeout             ,
        conflict                    ,
        gone                        ,
        length_required             ,
        precondition_failed         ,
        payload_too_large           ,
        uri_too_long                ,
        unsupported_media_type      ,
        range_not_satisfiable       ,
        expectation_failed          ,
        im_a_teapot                 , // Nice Easter egg
        misdirected_request    = 421,
        unprocessable_entity        ,
        locked                      ,
        failed_dependency           ,
        too_early                   ,
        upgrade_required            ,
        precondition_required  = 428,
        too_many_requests           ,
        req_header_too_large   = 431,
        unavailable_legal      = 451,
        internal_server_error  = 500,
        not_implemented             ,
        bad_gateway                 ,
        service_unavailable         ,
        gateway_timeout             ,
        http_ver_not_supported      ,
        var_also_negotiates         ,
        insufficient_storage        ,
        loop_detected               ,
        not_extended           = 510,
        network_auth_req
    };

    /// @brief Resolves a request-line method token into a `request_method`.
    /// @details Performs a case-sensitive match against the known HTTP verb
    /// tokens (`GET`, `HEAD`, `POST`, `PUT`, `DELETE`, `CONNECT`, `OPTIONS`,
    /// `TRACE`, `PATCH`). An unrecognized token reaches the default case,
    /// whose returned error is unwrapped by `switch_cases` and therefore
    /// panics (aborts) instead of being reported to the caller.
    /// @param req_method Method token as it appears on the request line (e.g. "GET").
    /// @param method Output parameter set to the resolved method on a match.
    static constexpr void check_request_method(
        const string_view& req_method, request_method& method)
    {
        using namespace string_view_literals;

        switch_cases(
            req_method,
            equal_case{"GET"_sv,     [&method]{method = request_method::get;    }},
            equal_case{"HEAD"_sv,    [&method]{method = request_method::head;   }},
            equal_case{"POST"_sv,    [&method]{method = request_method::post;   }},
            equal_case{"PUT"_sv,     [&method]{method = request_method::put;    }},
            equal_case{"DELETE"_sv,  [&method]{method = request_method::del;    }},
            equal_case{"CONNECT"_sv, [&method]{method = request_method::connect;}},
            equal_case{"OPTIONS"_sv, [&method]{method = request_method::options;}},
            equal_case{"TRACE"_sv,   [&method]{method = request_method::trace;  }},
            equal_case{"PATCH"_sv,   [&method]{method = request_method::patch;  }},
            default_case{
                []() -> option_err<runtime_error>
                {
                    return runtime_error{
                        "Invalid HTTP method, valid methods are:\nGET, HEAD, "
                        "POST, PUT, DELETE, CONNECT, OPTIONS, TRACE, PATCH"
                    };
                }
            }
        );
    }

    /// @brief Looks up the standard reason phrase for a response status.
    /// @details Maps every `response_status` enumerator to its textual reason
    /// phrase (e.g. `ok` to "OK"). Used as the fallback message returned by
    /// `response::get_message()` when no custom message has been set.
    /// @param status Status code whose reason phrase is looked up.
    /// @param _message Output parameter set to a string literal view of the
    /// reason phrase on success; left unchanged on failure.
    /// @return An empty `option_err` on success, or a `runtime_error` if
    /// `status` does not match any known `response_status` value.
    static constexpr auto set_status_message_spec(
        response_status status, string_view& _message) -> option_err<runtime_error>
    {
        #define HTTP_STATUS_CASE(code_enum, message)\
        case response_status::code_enum:\
        {\
            _message = #message;\
            break;\
        }

        switch (status)
        {
            HTTP_STATUS_CASE(cont, Continue)
            HTTP_STATUS_CASE(switching_protocols, Switching Protocols)
            HTTP_STATUS_CASE(processing, Processing)
            HTTP_STATUS_CASE(early_hints, Early Hints)
            HTTP_STATUS_CASE(ok, OK)
            HTTP_STATUS_CASE(created, Created)
            HTTP_STATUS_CASE(accepted, Accepted)
            HTTP_STATUS_CASE(non_author_info, Non-Authoritative Information)
            HTTP_STATUS_CASE(no_content, No Content)
            HTTP_STATUS_CASE(reset_content, Reset Content)
            HTTP_STATUS_CASE(partial_content, Partial Content)
            HTTP_STATUS_CASE(multi_status, Multi-Status)
            HTTP_STATUS_CASE(already_reported, Already Reported)
            HTTP_STATUS_CASE(im_used, IM Used)
            HTTP_STATUS_CASE(multiple_choices, Multiple Choices)
            HTTP_STATUS_CASE(moved_permanently, Moved Permanently) 
            HTTP_STATUS_CASE(found, Found)
            HTTP_STATUS_CASE(see_other, See Other)
            HTTP_STATUS_CASE(not_modified, Not Modified)
            HTTP_STATUS_CASE(use_proxy, Use Proxy)
            HTTP_STATUS_CASE(unused, unused) // Still not used
            HTTP_STATUS_CASE(temporary_redirect, Temporary Redirect)
            HTTP_STATUS_CASE(permanent_redirect, Permanent Redirect)
            HTTP_STATUS_CASE(bad_request, Bad Request)
            HTTP_STATUS_CASE(unauthorized, Unauthorized)
            HTTP_STATUS_CASE(payment_required, Payment Required) // WHY!?
            HTTP_STATUS_CASE(forbidden, Forbidden)
            HTTP_STATUS_CASE(not_found, Not Found)
            HTTP_STATUS_CASE(method_not_allowed, Method Not Allowed)
            HTTP_STATUS_CASE(not_acceptable, Not Acceptable)
            HTTP_STATUS_CASE(proxy_auth_req, Proxy Authentication Required)
            HTTP_STATUS_CASE(request_timeout, Request Timeout)
            HTTP_STATUS_CASE(conflict, Conflict)
            HTTP_STATUS_CASE(gone, Gone)
            HTTP_STATUS_CASE(length_required, Length Required)
            HTTP_STATUS_CASE(precondition_failed, Precondition Failed)
            HTTP_STATUS_CASE(payload_too_large, Payload Too Large)
            HTTP_STATUS_CASE(uri_too_long, URI Too Long)
            HTTP_STATUS_CASE(unsupported_media_type, Unsupported Media Type)
            HTTP_STATUS_CASE(range_not_satisfiable, Range Not Satisfiable)
            HTTP_STATUS_CASE(expectation_failed, Expectation Failed)
            case response_status::im_a_teapot:
            {
                _message = "I'm a teapot";
                break;
            }
            HTTP_STATUS_CASE(misdirected_request, Misdirect Request)
            HTTP_STATUS_CASE(unprocessable_entity, Unprocessable Entity)
            HTTP_STATUS_CASE(locked, Locked)
            HTTP_STATUS_CASE(failed_dependency, Failed Dependency)
            HTTP_STATUS_CASE(too_early, Too Early)
            HTTP_STATUS_CASE(upgrade_required, Upgrade Required)
            HTTP_STATUS_CASE(precondition_required, Precondition Required)
            HTTP_STATUS_CASE(too_many_requests, Too Many Requests)
            HTTP_STATUS_CASE(req_header_too_large, Request Header Fields Too Large)
            HTTP_STATUS_CASE(unavailable_legal, Unavailable For Legal Reasons)
            HTTP_STATUS_CASE(internal_server_error, Internal Server Error)
            HTTP_STATUS_CASE(not_implemented, Not Implemented)
            HTTP_STATUS_CASE(bad_gateway, Bad Gateway)
            HTTP_STATUS_CASE(service_unavailable, Service Unavailable)
            HTTP_STATUS_CASE(gateway_timeout, Gateway Timeout)
            HTTP_STATUS_CASE(http_ver_not_supported, HTTP Version Not Supported)
            HTTP_STATUS_CASE(var_also_negotiates, Variant Also Negotiates)
            HTTP_STATUS_CASE(insufficient_storage, Insufficient Storage)
            HTTP_STATUS_CASE(loop_detected, Loop Detected)
            HTTP_STATUS_CASE(not_extended, Not Extended)
            HTTP_STATUS_CASE(network_auth_req, Network Authentication Required)
            default:
            {
                return runtime_error{
                    "Unknown HTTP response code"
                };
            }
        }

        return {};
    }

    /// @brief Validates that a protocol token matches an expected `protocol_version`.
    /// @details Matches `protocol` against the known `HTTP/x.y` tokens and
    /// compares the resolved version against `version`. Both an unrecognized
    /// token and a version mismatch reach a default/branch case whose
    /// returned error is unwrapped by `switch_cases` and therefore panics
    /// (aborts) instead of being reported to the caller.
    /// @param protocol Protocol token as it appears on the request/status line
    /// (e.g. "HTTP/1.1").
    /// @param version Expected protocol version to validate against.
    static constexpr void check_protocol(
        const string_view& protocol, protocol_version version)
    {
        using namespace string_view_literals;

        switch_cases(
            protocol,
            equal_case{
                "HTTP/0.9"_sv, [version]() -> option_err<runtime_error>
                {
                    if (version != protocol_version::ver_0_9)
                    {
                        return runtime_error{
                            "Incompatible protocol versions between"
                            " the implementation and request"
                        };
                    }
                    else
                    {
                        return {};
                    }
                }
            },
            equal_case{
                "HTTP/1.0"_sv, [version]() -> option_err<runtime_error>
                {
                    if (version != protocol_version::ver_1_0)
                    {
                        return runtime_error{
                            "Incompatible protocol versions between"
                            " the implementation and request"
                        };
                    }
                    else
                    {
                        return {};
                    }
                }
            },
            equal_case{
                "HTTP/1.1"_sv, [version]() -> option_err<runtime_error>
                {
                    if (version != protocol_version::ver_1_1)
                    {
                        return runtime_error{
                            "Incompatible protocol versions between"
                            " the implementation and request"
                        };
                    }
                    else
                    {
                        return {};
                    }
                }
            },
            equal_case{
                "HTTP/2.0"_sv, [version]() -> option_err<runtime_error>
                {
                    if (version != protocol_version::ver_2_0)
                    {
                        return runtime_error{
                            "Incompatible protocol versions between"
                            " the implementation and request"
                        };
                    }
                    else
                    {
                        return {};
                    }
                }
            },
            equal_case{
                "HTTP/3.0"_sv, [version]() -> option_err<runtime_error>
                {
                    if (version != protocol_version::ver_3_0)
                    {
                        return runtime_error{
                            "Incompatible protocol versions between"
                            " the implementation and request"
                        };
                    }
                    else
                    {
                        return {};
                    }
                }
            },
            default_case{
                []() -> option_err<runtime_error>
                {
                    return runtime_error{
                        "Invalid HTTP protocol version, valid versions are:\n"
                        "HTTP/0.9, HTTP/1.0, HTTP/1.1, HTTP/2.0, HTTP/3.0"
                    };
                }
            }
        );
    }
} // namespace hsd::http