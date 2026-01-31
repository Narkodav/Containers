#pragma once
#include "../Utilities/ByteArray.h"
#include "../Utilities/Concepts.h"
#include "../Utilities/Macros.h"

#include <memory>
#include <stdexcept>
#include <initializer_list>
#include <vector>

#include "InitializerList.h"
#include "Span.h"
#include "Vector.h"

namespace Containers {

	template <typename T, TypedAllocatorType<T> Alloc = TypedAllocator<T>,
		LifetimeManagerType<T> Life = LifetimeManager<T>, size_t s_initialCapacity = 16, float s_growthFactor = 1.618f>
	class Stack
	{
		using Container = Vector<T, Alloc, Life, s_initialCapacity, s_growthFactor>;
	private:
		Container m_data;
	public:		
		using ValueType = T;
		using SizeType = size_t;
		using LifetimeManagerType = Life;
		using AllocatorType = Alloc;

		using ReleasePtr = typename Container::ReleasePtr;

		Stack() = default;
		~Stack() = default;

		Stack(const Stack&) = default;
		Stack(Stack&&) = default;

		Stack& operator=(const Stack&) = default;
		Stack& operator=(Stack&&) = default;

		ReleasePtr release() {
			return m_data.release();
		}

		void reserve(SizeType capacity) {
			m_data.reserve(capacity);
		};

		void shrinkToFit() {
			m_data.shrinkToFit();
		}

		void swap(Stack& other) noexcept {
			std::swap(this->m_data, other.m_data);
		}

		bool operator==(const Stack& other) const {
			return m_data == other.m_data;
		}

		template<typename... Args>
		void push(Args... args) {
			m_data.pushBack(std::forward<Args>(args)...);
		}

		void pop() {
			m_data.popBack();
		}

		const auto& top() {
			return m_data.back();
		}

		auto extract() {
			auto top = this->top();
			m_data.popBack();
			return top;
		}

		bool empty() const { return m_data.empty(); }
		SizeType count() const { return m_data.size(); }
	};
}