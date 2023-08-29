#include "json.h"
#include "json_builder.h"

#include <stdexcept>
#include <variant>
#include <optional>
#include <iostream>

using namespace std::literals;

namespace json{
KeyItemContext Builder::Key(std::string key) {
    ThrowIfReady();
    
    if (!nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Key() invokation outside the Dict"s);
    } else if (last_key_) {
        throw std::logic_error("The key is already set"s);
    }
    last_key_.emplace(key);
    return KeyItemContext(*this);
}
    
BaseContext Builder::Value(Node::Value value) {
    ThrowIfReady();
    
    if (untouched_) {
        root_ = std::visit([](auto v) {return Node(v);}, value);
        untouched_ = false;
        return BaseContext(*this);
    } else if (nodes_stack_.back()->IsDict()) {
        if (last_key_) {
            auto& dict = const_cast<Dict&>(nodes_stack_.back()->AsDict());
            dict.emplace(*last_key_, std::visit([](auto v) {return Node(v);}, value));
            
            if (dict.at(*last_key_).IsArray() || dict.at(*last_key_).IsDict()) {
                nodes_stack_.push_back(&dict.at(*last_key_));
            }
            
            last_key_.reset();
        } else {
            throw std::logic_error("Invoke Key() function first"s);
        }
        return DictItemContext(*this);
    } else if (nodes_stack_.back()->IsArray()) {
        auto& array = const_cast<Array&>(nodes_stack_.back()->AsArray());
        array.emplace_back(std::visit([](auto v) {return Node(v);}, value));
        
        if (array.back().IsArray() || array.back().IsDict()) {
            nodes_stack_.push_back(&array.back());
        }
        
        return ArrayItemContext(*this);
    } else {
        throw std::logic_error("Cannot add value"s);
    }
}

DictItemContext Builder::StartDict() {
    ThrowIfReady();
    
    if (untouched_) {
        root_ = Node(Dict());
        nodes_stack_.push_back(&root_);
        untouched_ = false;
    } else {
        Value(Dict());
    }
    return DictItemContext(*this);
}

ArrayItemContext Builder::StartArray() {
    ThrowIfReady();
    
    if (untouched_) {
        root_ = Node(Array());
        nodes_stack_.push_back(&root_);
        untouched_ = false;
        return ArrayItemContext(*this);
    }
       
    Value(Array());
    return ArrayItemContext(*this);
}

BaseContext Builder::EndDict() {
    ThrowIfReady();
    
    if (nodes_stack_.back()->IsDict() && !last_key_) {
        nodes_stack_.pop_back();
    } else {
        throw std::logic_error("Before end the Dict you should start it"s);
    }
    return BaseContext(*this);
}
BaseContext Builder::EndArray() {
    ThrowIfReady();
    
    if (nodes_stack_.back()->IsArray()) {
        nodes_stack_.pop_back();
    } else {
        throw std::logic_error("Before end the Array you should start it"s);
    }
    return BaseContext(*this);
}

Node Builder::Build() const {
    if (untouched_ || !nodes_stack_.empty()) {
        throw std::logic_error("The object is not ready"s);
    }
    return root_;
}
    
void Builder::ThrowIfReady() const {
    if (!untouched_ && nodes_stack_.empty()) {
        throw std::logic_error("The object is ready"s);
    } 
}
    
KeyItemContext BaseContext::Key(std::string key) {
    return builder_.Key(key);
    
}
    
BaseContext BaseContext::Value(Node::Value value) {
    return builder_.Value(value);
}
    
DictItemContext BaseContext::StartDict() {
    return builder_.StartDict();
}
    
ArrayItemContext BaseContext::StartArray() {
    return builder_.StartArray();
}
    
BaseContext BaseContext::EndDict() {
    return builder_.EndDict();
}
    
 BaseContext BaseContext::EndArray() {
    return builder_.EndArray();
}
    
Node BaseContext::Build() {
    return builder_.Build();
}
    
ArrayItemContext ArrayItemContext::Value(Node::Value value) {
    builder_.Value(value);
    return ArrayItemContext(builder_);
}
    
DictItemContext KeyItemContext::Value(Node::Value value) {
    builder_.Value(value);
    return DictItemContext(builder_);
}
}
  