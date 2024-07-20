#pragma once

#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <variant>
#include <vector>

namespace json {

    class Node;
    // Сохраните объявления Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

        Node() = default;

        Node(Value node)
            : value_(std::move(node))
        {
        }

        Node(std::nullptr_t null)
            : value_(null)
        {
        }

        Node(int val)
            : value_(val)
        {
        }

        Node(double val)
            : value_(val)
        {
        }

        Node(bool val)
            : value_(val)
        {
        }

        Node(std::string val)
            : value_(std::move(val))
        {
        }

        Node(Array val)
            : value_(std::move(val))
        {
        }

        Node(Dict val)
            : value_(std::move(val))
        {
        }

        const Value& GetValue() const { return value_; }

        bool IsNull() const { return std::holds_alternative<std::nullptr_t>(value_); }
        bool IsPureDouble() const { return std::holds_alternative<double>(value_); }
        bool IsBool() const { return std::holds_alternative<bool>(value_); }
        bool IsInt() const { return std::holds_alternative<int>(value_); }
        bool IsString() const { return std::holds_alternative<std::string>(value_); }
        bool IsArray() const { return std::holds_alternative<Array>(value_); }
        bool IsMap() const { return std::holds_alternative<Dict>(value_); }
        bool IsDouble() const;


        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        [[nodiscard]] bool operator==(const Node& rhs) const noexcept = default;
        [[nodiscard]] bool operator!=(const Node& rhs) const noexcept = default;

    private:
        Value value_;
    };

    class Document {
    public:

        Document() = default;

        explicit Document(Node root);

        const Node& GetRoot() const;
        [[nodiscard]] bool operator==(const Document& rhs) const noexcept = default;
        [[nodiscard]] bool operator!=(const Document& rhs) const noexcept = default;
    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json