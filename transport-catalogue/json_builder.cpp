
#include "json_builder.h"

#include <string>

namespace json {

	using namespace std;

	Node GetNode(Node::Value value) {
		Node result;
		if (holds_alternative<int>(value)) {
			result = get<int>(value);
		}
		else if (holds_alternative<double>(value)) {
			result = get<double>(value);
		}
		else if (holds_alternative<bool>(value)) {
			result = get<bool>(value);
		}
		else if (holds_alternative<string>(value)) {
			result = move(get<string>(value));
		}
		else if (holds_alternative<Array>(value)) {
			result = move(get<Array>(value));
		}
		else if (holds_alternative<Dict>(value)) {
			result = move(get<Dict>(value));
		}
		return result;
	}

	// ------------------------------------------------------------------------
	
	// BaseBuilder
	
	Builder& Builder::BaseBuilder::Value(Builder& builder, Node::Value value) {
		if (builder.root_.has_value()) {
			throw std::logic_error("Twice build"s);
		}
		builder.root_ = GetNode(value);
		return builder;
	}
	
	Builder& Builder::BaseBuilder::StartDict(Builder& builder) {
		builder.SetDictBuilder();
		return builder;
	}
	Builder& Builder::BaseBuilder::StartArray(Builder& builder) {
		builder.SetArrayBuilder();
		return builder;
	}

	Node::Value Builder::BaseBuilder::GetValue() {
		return 0;
	}

	Node Builder::BaseBuilder::Build(Builder& builder) {
		
		if (!builder.root_.has_value()) {
			throw std::logic_error("Nothing to build"s);
		}
		return builder.root_.value();
	}

	// ------------------------------------------------------------------------
	
	// ArrayBuilder

	Builder& Builder::ArrayBuilder::Value(Builder& builder, Node::Value value) {
		arr_.push_back(GetNode(value));
		return builder;
	}

	Builder& Builder::ArrayBuilder::StartDict(Builder& builder) {
		builder.builders_stack_.push_back(move(builder.builder_));
		builder.SetDictBuilder();
		return builder;
	}
	Builder& Builder::ArrayBuilder::StartArray(Builder& builder) {
		builder.builders_stack_.push_back(move(builder.builder_));
		builder.SetArrayBuilder();
		return builder;
	}
	
	Builder& Builder::ArrayBuilder::EndArray(Builder& builder) {
		if (builder.builders_stack_.empty()) {
			builder.root_ = move(arr_);
			builder.SetBaseBuilder();
		}
		else {
			builder.SetPrevBuilder();
		}
		return builder;
	}

	Node::Value Builder::ArrayBuilder::GetValue() {
		return arr_;
	}
	
	// ------------------------------------------------------------------------

	// DictBuilder

	Builder& Builder::DictBuilder::Key(Builder& builder, std::string key) {
		key_ = move(key);
		return builder;
	}
	Builder& Builder::DictBuilder::Value(Builder& builder, Node::Value value) {
		dict_.insert({key_, GetNode(value)});
		return builder;
	}

	Builder& Builder::DictBuilder::StartDict(Builder& builder) {
		builder.builders_stack_.push_back(move(builder.builder_));
		builder.SetDictBuilder();
		return builder;
	}
	Builder& Builder::DictBuilder::StartArray(Builder& builder) {
		builder.builders_stack_.push_back(move(builder.builder_));
		builder.SetArrayBuilder();
		return builder;
	}

	Builder& Builder::DictBuilder::EndDict(Builder& builder) {
		if (builder.builders_stack_.empty()) {
			builder.root_ = move(dict_);
			builder.SetBaseBuilder();
		}
		else {
			builder.SetPrevBuilder();
		}
		return builder;
	}

	Node::Value Builder::DictBuilder::GetValue() {
		return dict_;
	}

	// ---------------------------------------------------------

	// Builder
	
	void Builder::SetBaseBuilder() {
		builder_.reset(new BaseBuilder);
	}

	void Builder::SetArrayBuilder() {
		builder_.reset(new ArrayBuilder);
	}

	void Builder::SetDictBuilder() {
		builder_.reset(new DictBuilder);
	}

	void Builder::SetPrevBuilder() {
		Node::Value temp = move(builder_->GetValue());
		builder_ = move(builders_stack_.back());
		builders_stack_.pop_back();
		Value(std::move(temp));
	}

}
