#include "json.h"

using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        using Number = std::variant<int, double>;

        Number LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
                };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
                };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        // Считывает содержимое строкового литерала JSON-документа
        // Функцию следует использовать после считывания открывающего символа ":
        std::string LoadString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error"s);
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error"s);
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }

        Dict LoadDict(istream& input) {
            Dict result;
            using namespace std::literals;
            char ch = ' ';
            while (true) {
                if (input >> ch) {
                    if (ch == '}') {
                        break;
                    }
                    if (ch == ',') {
                        input >> ch;
                    }

                    string key = LoadString(input);
                    input >> ch;
                    result.insert({ move(key), LoadNode(input) });
                }
                else {
                    throw ParsingError("Map parsing error"s);
                }
            }
            return result;
        }

        Array LoadArray(istream& input) {
            Array result;
            using namespace std::literals;
            char ch = ' ';
            while (true) {
                if (input >> ch) {
                    if (ch == ']') {
                        break;
                    }
                    if (ch != ',') {
                        input.putback(ch);
                    }

                    result.push_back(LoadNode(input));
                }
                else {
                    throw ParsingError("Array parsing error"s);
                }
            }
            return result;
        }

        using BoolOrNull = std::variant<bool, std::nullptr_t>;

        BoolOrNull LoadBool(istream& input) {
            string line;
            char ch = ' ';
            nullptr_t null;
            while (input >> ch) {
                if (ch == ']' || ch == '}' || ch == ',') {
                    input.putback(ch);
                    break;
                }
                line.push_back(ch);
            }
            if (line == "false"sv) {
                return false;
            }
            else if (line == "true"sv) {
                return true;
            }
            else if (line == "null"sv) {
                return null;
            }
            throw ParsingError("Bool parsing error"s);
        }

        Node LoadNode(istream& input) {
            char ch;
            input >> ch;

            switch (ch)
            {
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
            case '-':
            {
                input.putback(ch);
                Number num = LoadNumber(input);
                if (std::holds_alternative<double>(num)) {
                    return Node(get<double>(num));
                }
                return Node(get<int>(num));
            }
            case 'f': case 't': case 'n':
            {
                input.putback(ch);
                BoolOrNull val = LoadBool(input);
                if (std::holds_alternative<bool>(val)) {
                    return Node(get<bool>(val));
                }
                return Node();
            }
            case '[':
            {
                return Node(LoadArray(input));
            }
            case '{':
            {
                return Node(LoadDict(input));
            }
            case '"':
            {
                return Node(LoadString(input));
            }
            default:
                throw ParsingError("Parsing error"s);
            }
        }

    }  // namespace

    bool Node::IsDouble() const {
        if (IsPureDouble()) {
            return true;
        }
        return IsInt();
    }

    int Node::AsInt() const {
        if (!IsInt()) {
            throw std::logic_error("wrong type"s);
        }
        return get<int>(value_);
    }
    bool Node::AsBool() const {
        if (!IsBool()) {
            throw std::logic_error("wrong type"s);
        }
        return get<bool>(value_);
    }

    double Node::AsDouble() const {
        if (!IsDouble()) {
            throw std::logic_error("wrong type"s);
        }
        return (IsPureDouble()) ? get<double>(value_)
            : static_cast<double>(get<int>(value_));
    }

    const string& Node::AsString() const {
        if (!IsString()) {
            throw std::logic_error("wrong type"s);
        }
        return get<string>(value_);
    }

    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw std::logic_error("wrong type"s);
        }
        return get<Array>(value_);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap()) {
            throw std::logic_error("wrong type"s);
        }
        return get<Dict>(value_);
    }

    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        PrintContext Indented() const {
            return { out, indent_step, indent_step + indent };
        }
    };

    void PrintNode(const Node& value, const PrintContext& ctx);

    template <typename Value>
    void PrintValue(const Value& value, const PrintContext& ctx) {
        ctx.out << value;
    }

    void PrintString(const std::string& value, std::ostream& out) {
        out.put('"');
        for (const char c : value) {
            switch (c) {
            case '\r':
                out << "\\r"sv;
                break;
            case '\n':
                out << "\\n"sv;
                break;
            case '\t':
                out << "\\t"sv;
                break;
            case '"':
                // Символы " и \ выводятся как \" или \\, соответственно
                [[fallthrough]];
            case '\\':
                out.put('\\');
                [[fallthrough]];
            default:
                out.put(c);
                break;
            }
        }
        out.put('"');
    }

    template <>
    void PrintValue<std::string>(const std::string& value, const PrintContext& ctx) {
        PrintString(value, ctx.out);
    }

    template <>
    void PrintValue<std::nullptr_t>(const std::nullptr_t&, const PrintContext& ctx) {
        ctx.out << "null"sv;
    }

    // В специализации шаблона PrintValue для типа bool параметр value передаётся
    // по константной ссылке, как и в основном шаблоне.
    // В качестве альтернативы можно использовать перегрузку:
    // void PrintValue(bool value, const PrintContext& ctx);
    template <>
    void PrintValue<bool>(const bool& value, const PrintContext& ctx) {
        ctx.out << (value ? "true"sv : "false"sv);
    }

    template <>
    void PrintValue<Array>(const Array& nodes, const PrintContext& ctx) {
        std::ostream& out = ctx.out;
        out << "[\n"sv;
        bool first = true;
        auto inner_ctx = ctx.Indented();
        for (const Node& node : nodes) {
            if (first) {
                first = false;
            }
            else {
                out << ",\n"sv;
            }
            inner_ctx.PrintIndent();
            PrintNode(node, inner_ctx);
        }
        out.put('\n');
        ctx.PrintIndent();
        out.put(']');
    }

    template <>
    void PrintValue<Dict>(const Dict& nodes, const PrintContext& ctx) {
        std::ostream& out = ctx.out;
        out << "{\n"sv;
        bool first = true;
        auto inner_ctx = ctx.Indented();
        for (const auto& [key, node] : nodes) {
            if (first) {
                first = false;
            }
            else {
                out << ",\n"sv;
            }
            inner_ctx.PrintIndent();
            PrintString(key, ctx.out);
            out << ": "sv;
            PrintNode(node, inner_ctx);
        }
        out.put('\n');
        ctx.PrintIndent();
        out.put('}');
    }

    void PrintNode(const Node& node, const PrintContext& ctx) {
        std::visit(
            [&ctx](const auto& value) {
                PrintValue(value, ctx);
            },
            node.GetValue());
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), PrintContext{ output });
    }

}  // namespace json
