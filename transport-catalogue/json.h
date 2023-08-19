#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

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

// Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext Indented() const {
        return {out, indent_step, indent_step + indent};
    }
};

class Node {
public:
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    Node() = default;
    
    Node(std::nullptr_t value);
    Node(Array value);
    Node(Dict value);
    Node(bool value);
    Node(int value);
    Node(double value);
    Node(std::string value);
    
    bool IsInt() const;
    bool IsDouble() const; 
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;
    
    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;
    
    const Value& GetValue() const;
    
    void PrintNode(const PrintContext& ctx) const;

private:
    Value value_;
};
    
bool operator==(const Node& lhs, const Node& rhs);    
bool operator!=(const Node& lhs, const Node& rhs);

struct NodePrinter {
    PrintContext ctx;
    
    template <typename Value>
    void operator()(Value value) const {
        ctx.out << value;
    }
    
    void operator()(std::nullptr_t value) const;
    void operator()(bool value) const;
    void operator()(Dict map) const;
    void operator()(Array array) const;
    void operator()(std::string str) const;
};

class Document {
public:
    Document() = default;
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

bool operator==(const Document& lhs, const Document& rhs);
bool operator!=(const Document& lhs, const Document& rhs);
Document Load(std::istream& input);

void PrintNode(const Node& node, const PrintContext& ctx);

void Print(const Document& doc, std::ostream& output);

}  // namespace json