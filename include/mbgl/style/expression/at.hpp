#pragma once

#include <mbgl/style/expression/expression.hpp>
#include <mbgl/style/conversion.hpp>

namespace mbgl {
namespace style {
namespace expression {

class At : public Expression {
public:
    At(std::unique_ptr<Expression> index_, std::unique_ptr<Expression> input_) :
        Expression(input_->getType().get<type::Array>().itemType),
        index(std::move(index_)),
        input(std::move(input_))
    {}
    
    static ParseResult parse(const mbgl::style::conversion::Value& value, ParsingContext ctx);
    
    EvaluationResult evaluate(const EvaluationParameters& params) const override;
    void accept(std::function<void(const Expression*)>) const override;

private:
    std::unique_ptr<Expression> index;
    std::unique_ptr<Expression> input;
};

} // namespace expression
} // namespace style
} // namespace mbgl
