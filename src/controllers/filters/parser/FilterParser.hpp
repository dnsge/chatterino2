#pragma once

#include "controllers/filters/parser/Tokenizer.hpp"
#include "controllers/filters/parser/Types.hpp"

namespace filterparser {

class FilterParser
{
public:
    FilterParser(const QString &text);
    bool execute(const MessagePtr &message) const;

private:
    Expression *parseExpression();
    Expression *parseAnd();
    Expression *parseUnary();
    Expression *parseCondition();
    Expression *parseValue();

    QString text_;
    Tokenizer tokenizer_;
    Expression *builtExpression_;
};
}  // namespace filterparser
