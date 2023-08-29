#pragma once

#include "json.h"

#include <stdexcept>
#include <optional>

using namespace std::literals;

namespace json {
class BaseContext;
class KeyItemContext;
class DictItemContext;
class ArrayItemContext;
    
class Builder {
public:
    KeyItemContext Key(std::string key);
    BaseContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    BaseContext EndDict();
    BaseContext EndArray();
    Node Build() const;
    
private:
    Node root_;
    std::vector<Node*> nodes_stack_;
    std::optional<std::string> last_key_;
    bool untouched_ = true; 
    
    void ThrowIfReady() const;
};
    
class BaseContext {
public:
    BaseContext(Builder& builder) : builder_(builder) {}
    
    KeyItemContext Key(std::string key);
    BaseContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    BaseContext EndDict();
    BaseContext EndArray();
    Node Build();
    
protected:
    Builder& builder_;
};

class DictItemContext: public BaseContext {
public:
    DictItemContext(Builder& builder) : BaseContext(builder) {}
    
    BaseContext Value(Node::Value value) = delete;
    DictItemContext StartDict() = delete;
    ArrayItemContext StartArray() = delete;
    BaseContext EndArray() = delete;
    Node Build() = delete;
};

class ArrayItemContext: public BaseContext {
public:
    ArrayItemContext(Builder& builder) : BaseContext(builder) {}
    
    ArrayItemContext Value(Node::Value value);
    KeyItemContext Key() = delete;
    BaseContext EndDict() = delete;
    Node Build() = delete;
};

class KeyItemContext: public BaseContext {
public:
    KeyItemContext(Builder& builder) : BaseContext(builder) {}
    
    DictItemContext Value(Node::Value value);
    DictItemContext Key() = delete;
    BaseContext EndDict() = delete;
    BaseContext EndArray() = delete;
    Node Build() = delete;
};
}