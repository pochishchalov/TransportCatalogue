#pragma once

#include "json.h"

#include <optional>
#include <memory>
#include <vector>

namespace json {
	
	class Builder
	{
		// Вспомогательные классы строителей для построения JSON 

		class BaseBuilder;
		class DictBuilder;
		class ArrayBuilder;

		// Классы-контексты для ограничения вызова невалидных методов
		// класса Builder в определенном состоянии
		
		class BaseContext;
		class DictItemContext;
		class ArrayItemContext;
		class DictValueContext;

	public:
		

		Builder()
			:builder_(std::make_unique<BaseBuilder>())
		{
		}
		~Builder() = default;

		Node Build() { return builder_->Build(*this); }

		DictValueContext Key(std::string key) {
			return DictValueContext{ builder_->Key(*this, key) }; 
		}
		BaseContext Value(Node::Value value) {
			return BaseContext{ builder_->Value(*this, value) };
		}
		DictItemContext StartDict() {
			return BaseContext{ builder_->StartDict(*this) };
		}
		ArrayItemContext StartArray() {
			return BaseContext{ builder_->StartArray(*this) };
		}
		BaseContext EndDict() {
			return builder_->EndDict(*this);
		}
		BaseContext EndArray() {
			return builder_->EndArray(*this);
		}

	private:
		std::optional<Node> root_;
		std::unique_ptr<BaseBuilder> builder_;
		std::vector<std::unique_ptr<BaseBuilder>> builders_stack_;

		void SetBaseBuilder();
		void SetArrayBuilder();
		void SetDictBuilder();
		void SetPrevBuilder();

		// Родительский класс "базового" строителя
		class BaseBuilder {
		public:

			virtual ~BaseBuilder() = default;

			// Ничего не делает, служит для переопределения в DictBuilder
			virtual Builder& Key(Builder& builder, std::string) {
				return builder;
			}
			virtual Builder& Value(Builder& builder, Node::Value value);

			// Ничего не делает, служит для переопределения в DictBuilder
			virtual Builder& EndDict(Builder& builder) {
				return builder;
			}

			// Ничего не делает, служит для переопределения в ArrayBuilder
			virtual Builder& EndArray(Builder& builder) {
				return builder;
			}
			virtual Builder& StartDict(Builder& builder);
			virtual Builder& StartArray(Builder& builder);
			virtual Node::Value GetValue();
			virtual Node Build(Builder& builder);
		};

		// Класс для создания словаря json::Dict
		class DictBuilder : public BaseBuilder {
		public:

			~DictBuilder() override = default;

			Builder& Key(Builder& builder, std::string key) override;
			Builder& Value(Builder& builder, Node::Value value) override;
			Builder& EndDict(Builder& builder) override;
			Builder& StartDict(Builder& builder) override;
			Builder& StartArray(Builder& builder) override;
			Node::Value GetValue() override;

		private:
			Dict dict_;
			std::string key_;
		};

		// Класс для создания вектора json::Array
		class ArrayBuilder : public BaseBuilder {
		public:

			~ArrayBuilder() override = default;

			Builder& Value(Builder& builder, Node::Value value) override;
			Builder& EndArray(Builder& builder) override;
			Builder& StartDict(Builder& builder) override;
			Builder& StartArray(Builder& builder) override;
			Node::Value GetValue() override;

		private:
			Array arr_;
		};


		class BaseContext {
		public:
			BaseContext(Builder& builder)
				: builder_(builder)
			{
			}
			Node Build() {
				return builder_.Build();
			}
			DictValueContext Key(std::string key) {
				return builder_.Key(key);
			}
			BaseContext Value(Node::Value value) { 
				return builder_.Value(value);
			}
			DictItemContext StartDict() {
				return builder_.StartDict();
			}
			ArrayItemContext StartArray() {
				return builder_.StartArray();
			}
			BaseContext EndDict() {
				return builder_.EndDict();
			}
			BaseContext EndArray() {
				return builder_.EndArray();
			}

		private:
			Builder& builder_;
		};

		class DictItemContext : public BaseContext {
		public:
			DictItemContext(BaseContext base)
				: BaseContext(base)
			{
			}
			BaseContext Value(Node::Value value) = delete;
			BaseContext EndArray() = delete;
			DictItemContext StartDict() = delete;
			ArrayItemContext StartArray() = delete;
			Node Build() = delete;
		};

		class ArrayItemContext : public BaseContext {
		public:
			ArrayItemContext(BaseContext base)
				: BaseContext(base)
			{
			}
			ArrayItemContext Value(Node::Value value) {
				return BaseContext::Value(std::move(value));
			}

			DictValueContext Key(std::string key) = delete;
			BaseContext EndDict() = delete;
			Node Build() = delete;
		};

		class DictValueContext : public BaseContext {
		public:
			DictValueContext(BaseContext base)
				: BaseContext(base)
			{
			}
			DictItemContext Value(Node::Value value) {
				return BaseContext::Value(std::move(value));
			}

			BaseContext EndArray() = delete;
			BaseContext EndDict() = delete;
			DictValueContext Key(std::string key) = delete;
			Node Build() = delete;
		};
	};
	
} // namespace json