#pragma once

#import "controllers/filters/parser/Types.hpp"

namespace filterparser {

static const QStringList validIdentifiers = {
    "message.content",             // String
    "message.length",              // Int
    "author.name",                 // String
    "author.subscribed",           // Bool
    "author.subscription_length",  // Int
    "author.color",                // QColor
    "author.no_color",             // Bool
    "author.badges"};              // String list

// clang-format off
static const QRegularExpression tokenRegex(
    QString("\\\"((\\\\\")|[^\\\"])*\\\"|") + // String literal
    QString("[\\w\\.]+|") +                   // Identifier or reserved keyword
    QString("(<=?|>=?|!=?|==|\\|\\||&&)+|") + // Operator
    QString("[\\(\\)]")                       // Parenthesis
);
// clang-format on

class Tokenizer
{
public:
    Tokenizer(const QString &text)
    {
        QRegularExpressionMatchIterator i = tokenRegex.globalMatch(text);
        while (i.hasNext())
        {
            auto text = i.next().captured();
            this->tokens_ << text;
            this->tokenTypes_ << tokenize(text);
        }
    }

    bool hasNext() const
    {
        return this->i_ < this->tokens_.length();
    }

    QString next()
    {
        this->i_++;
        return this->tokens_.at(this->i_ - 1);
    }

    TokenType nextTokenType()
    {
        return this->tokenTypes_.at(this->i_);
    }

    TokenType tokenType()
    {
        return this->tokenTypes_.at(this->i_ - 1);
    }

    bool nextTokenIsBinaryOp()
    {
        auto next = this->nextTokenType();
        return this - typeIsBinaryOp(next);
    }

    bool typeIsBinaryOp(TokenType token)
    {
        return token > TokenType::BINARY_START && token < TokenType::BINARY_END;
    }

    bool nextTokenIsUnaryOp()
    {
        auto next = this->nextTokenType();
        return this->typeIsUnaryOp(next);
    }

    bool typeIsUnaryOp(TokenType token)
    {
        return token > TokenType::UNARY_START && token < TokenType::UNARY_END;
    }

    void debug()
    {
        if (this->i_ > 0)
        {
            qDebug() << "= current" << this->tokens_.at(this->i_ - 1);
            qDebug() << "= current type" << this->tokenTypes_.at(this->i_ - 1);
        }
        else
        {
            qDebug() << "= no current";
        }
        if (this->hasNext())
        {
            qDebug() << "= next" << this->tokens_.at(this->i_);
            qDebug() << "= next type" << this->tokenTypes_.at(this->i_);
        }
        else
        {
            qDebug() << "= no next";
        }
    }

    const QStringList allTokens()
    {
        return this->tokens_;
    }

private:
    int i_ = 0;
    QStringList tokens_;
    QList<TokenType> tokenTypes_;

    TokenType tokenize(const QString &text)
    {
        if (text == "&&")
            return TokenType::AND;
        else if (text == "||")
            return TokenType::OR;
        else if (text == "(")
            return TokenType::LP;
        else if (text == ")")
            return TokenType::RP;
        else if (text == "==")
            return TokenType::EQ;
        else if (text == "!=")
            return TokenType::NEQ;
        else if (text == "<")
            return TokenType::LT;
        else if (text == ">")
            return TokenType::GT;
        else if (text == "<=")
            return TokenType::LTE;
        else if (text == ">=")
            return TokenType::GTE;
        else if (text == "contains")
            return TokenType::CONTAINS;
        else if (text == "!")
            return TokenType::NOT;
        else
        {
            if (text.front() == '"' && text.back() == '"')
                return TokenType::STRING;

            if (validIdentifiers.contains(text))
                return TokenType::IDENTIFIER;

            bool flag;
            if (text.toInt(&flag); flag)
                return TokenType::INT;
        }

        return TokenType::NONE;
    }
};
}  // namespace filterparser
