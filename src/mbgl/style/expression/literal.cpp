
#include <mbgl/style/expression/literal.hpp>

namespace mbgl {
namespace style {
namespace expression {

template <typename T>
optional<Value> checkNumber(T n) {
    if (n > std::numeric_limits<double>::max()) {
        return {std::numeric_limits<double>::infinity()};
    } else {
        return {static_cast<double>(n)};
    }
}

optional<Value> parseValue(const mbgl::style::conversion::Value& value, ParsingContext ctx) {
    using namespace mbgl::style::conversion;
    if (isUndefined(value)) return {Null};
    if (isObject(value)) {
        std::unordered_map<std::string, Value> result;
        bool error = false;
        eachMember(value, [&] (const std::string& k, const mbgl::style::conversion::Value& v) -> optional<conversion::Error> {
            if (!error) {
                optional<Value> memberValue = parseValue(v, ctx);
                if (memberValue) {
                    result.emplace(k, *memberValue);
                } else {
                    error = true;
                }
            }
            return {};
        });
        return error ? optional<Value>() : optional<Value>(result);
    }
    
    if (isArray(value)) {
        std::vector<Value> result;
        const auto length = arrayLength(value);
        for(std::size_t i = 0; i < length; i++) {
            optional<Value> item = parseValue(arrayMember(value, i), ctx);
            if (item) {
                result.emplace_back(*item);
            } else {
                return optional<Value>();
            }
        }
        return optional<Value>(result);
    }
    
    optional<mbgl::Value> v = toValue(value);
    assert(v);
    
    return v->match(
        [&](uint64_t n) { return checkNumber(n); },
        [&](int64_t n) { return checkNumber(n); },
        [&](double n) { return checkNumber(n); },
        [&](const auto&) {
            return optional<Value>(toExpressionValue(*v));
        }
    );
}

ParseResult Literal::parse(const mbgl::style::conversion::Value& value, ParsingContext ctx) {
    const optional<Value> parsedValue = parseValue(value, ctx);

    if (!parsedValue) {
        return ParseResult();
    }

    // special case: infer the item type if possible for zero-length arrays
    if (
        ctx.expected &&
        ctx.expected->template is<type::Array>() &&
        parsedValue->template is<std::vector<Value>>()
    ) {
        auto type = typeOf(*parsedValue).template get<type::Array>();
        auto expected = ctx.expected->template get<type::Array>();
        if (
            type.N && (*type.N == 0) &&
            (!expected.N || (*expected.N == 0))
        ) {
            return ParseResult(std::make_unique<Literal>(expected, parsedValue->template get<std::vector<Value>>()));
        }
    }
    return ParseResult(std::make_unique<Literal>(*parsedValue));
}

} // namespace expression
} // namespace style
} // namespace mbgl

