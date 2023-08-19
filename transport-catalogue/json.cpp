#include "json.h"

#include <variant>
#include <iostream>
#include <string>
#include <utility>
#include <stdexcept>
#include <iomanip>
#include <istream>

using namespace std::literals;

namespace json {

namespace {

Node LoadNode(std::istream& input);

Node LoadArray(std::istream& input) {
    Array result;
    
    char c;
    while (input >> c && c != ']') {
        if (c != ',') {
            input.putback(c);
        }

        result.push_back(LoadNode(input));
    }
    
    if (c != ']') {
        throw ParsingError("Array parsing error"s);
    }

    return Node(std::move(result));
}

Node LoadNumber(std::istream& input) {
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
    } else {
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
                return Node(std::stoi(parsed_num));
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node(std::stod(parsed_num));
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
Node LoadString(std::istream& input) {
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
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
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return Node(std::move(s));
}


Node LoadDict(std::istream& input) {
    Dict result;
    
    char c;
    while (input >> c && c != '}') {
        if (c == ',') {
            input >> std::ws >> c;
        }

        std::string key = LoadString(input).AsString();
        input >> c;
        result.insert({std::move(key), LoadNode(input)});
    }
    
    if (c != '}') {
        throw ParsingError("Dict parsing error"s);
    }

    return Node(std::move(result));
}
    
std::string LoadChars(int count, std::istream& input) {
    std::string str;
    for (int i = 0; i < count; ++i) {
        str += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Parsing error"s);
        }
    }
    return str;
}
    
Node LoadBool(std::istream& input) {
    auto count = input.peek() == 't' ? 4 : 5;
    auto str = LoadChars(count, input);
    if (str == "true"s) {
        return Node(true);
    } else if (str == "false"s) {
        return Node(false);
    } else {
        throw ParsingError("Bool parsing error"s);
    }
}

Node LoadNull(std::istream& input) {
    int count = 4;
    auto str = LoadChars(count, input);
    if (str == "null"s) {
        return Node(nullptr);
    } else {
        throw ParsingError("Null parsing error"s);
    }
}

Node LoadNode(std::istream& input) {
    char c;
    input >> std::ws >> c;

    switch (c){
    case '[':
        return LoadArray(input);
    case '{':
        return LoadDict(input);
    case '"':
        return LoadString(input);
    case 't':
    case 'f':
        input.putback(c);
        return LoadBool(input);
    case 'n':
        input.putback(c);
        return LoadNull(input);
    default:
        input.putback(c);
        return LoadNumber(input);
    }
}

}  // namespace
    
Node::Node(std::nullptr_t value) 
    : value_(value) {
}
    
Node::Node(Array value) 
    : value_(std::move(value)) {
}
  
Node::Node(Dict value) 
    : value_(std::move(value)) {
}
    
Node::Node(bool value) 
    : value_(value) {
}
    
Node::Node(int value) 
    : value_(value) {
}
   
Node::Node(double value) 
    : value_(value) {
}
    
Node::Node(std::string value) 
    : value_(std::move(value)) {
}
    
bool Node::IsInt() const {
    return std::holds_alternative<int>(value_);
}
    
bool Node::IsDouble() const {
   return IsPureDouble() || IsInt(); 
}
    
bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(value_);
}
    
bool Node::IsBool() const {
    return std::holds_alternative<bool>(value_);
}
    
bool Node::IsString() const {
    return std::holds_alternative<std::string>(value_);
}
    
bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(value_);
}
bool Node::IsArray() const {
    return std::holds_alternative<Array>(value_);
}
    
bool Node::IsMap() const {
    return std::holds_alternative<Dict>(value_);
}

int Node::AsInt() const {
    if (IsInt()) {
        return std::get<int>(value_);
    }else {
        throw std::logic_error("Wrong type"s);
    }
}
    
bool Node::AsBool() const {
    if (IsBool()) {
        return std::get<bool>(value_);
    }else {
        throw std::logic_error("Wrong type"s);
    }
}
double Node::AsDouble() const {
    if (IsPureDouble()) {
        return std::get<double>(value_);
    } else if (IsInt()) {
        return std::get<int>(value_);
    } else {
        throw std::logic_error("Wrong type"s);
    }
}
    
const std::string& Node::AsString() const{
     if (IsString()) {
        return std::get<std::string>(value_);
    }else {
        throw std::logic_error("Wrong type"s);
    }
}
    
const Array& Node::AsArray() const {
     if (IsArray()) {
        return std::get<Array>(value_);
    }else {
        throw std::logic_error("Wrong type"s);
    }
}
const Dict& Node::AsMap() const {
     if (IsMap()) {
        return std::get<Dict>(value_);
    }else {
        throw std::logic_error("Wrong type"s);
    }
}
    
const Node::Value& Node::GetValue() const {
    return value_;
}
    
bool operator==(const Node& lhs, const Node& rhs) {
    return lhs.GetValue() == rhs.GetValue();
}
    
bool operator!=(const Node& lhs, const Node& rhs) { 
    return !(lhs == rhs);
}
    
void NodePrinter::operator()(bool value) const {
    ctx.out << std::boolalpha <<value;
}
void NodePrinter::operator()(std::nullptr_t) const {
    ctx.out << "null"s;
}
    
void NodePrinter::operator()(Dict map) const {
    ctx.out << "{\n"s;
    bool first = true;
    for (const auto& [key, node]: map) {
        if (!first){
            ctx.out << ",\n"s;
        } else {
            first = false;
        }
        ctx.Indented().PrintIndent();
        ctx.out <<"\""s << key << "\""s << ": "s;
        PrintNode(node, ctx.Indented());
    }
    ctx.out << "\n"s;
    ctx.PrintIndent();
    ctx.out << "}"s;
}    
void NodePrinter::operator()(Array array) const {
    ctx.out << "[\n"s;
    bool first = true;
    for (const auto& node: array) {
        if (!first) {
            ctx.out << ",\n"s;
        } else {
            first = false;
        }
        ctx.Indented().PrintIndent();
        PrintNode(node, ctx.Indented());
        
    }
    ctx.out << "\n"s;
    ctx.PrintIndent();
    ctx.out << "]"s;
}

void NodePrinter::operator()(std::string str) const {
    ctx.out << "\""s;
    for (const auto& ch: str) {
        if (ch == '\r') {
            ctx.out << "\\r"s;
        } else if (ch == '\n') {
            ctx.out << "\\n"s;
        } else if (ch == '\t') {
            ctx.out << "\t"s;
        } else if (ch == '\"') {
            ctx.out << "\\\""s;
        } else if (ch == '\\') {
            ctx.out << "\\\\"s;
        } else {
            ctx.out << ch;
        }
    }
    ctx.out << "\""s;
}
    
Document::Document(Node root)
    : root_(std::move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}
    
bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}
    
bool operator!=(const Document& lhs, const Document& rhs) {
    return !(lhs == rhs);
}

Document Load(std::istream& input) {
    return Document{LoadNode(input)};
}

void PrintNode(const Node& node, const PrintContext& ctx) {
    std::visit(NodePrinter{ctx}, node.GetValue());
}

void Print(const Document& doc, std::ostream& output) {
    auto ctx = PrintContext{output};
    PrintNode(doc.GetRoot(), ctx);
}
}  // namespace json