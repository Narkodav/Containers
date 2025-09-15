#pragma once
#include <stdexcept>
#include <cassert>
#include <concepts>

namespace Containers {
	template <typename Key, typename Val>
	class MapPair
	{
	private:
		Key key;
		Val val;

	public:
		using key_type = Key;
		using mapped_type = Val;
		using value_type = std::pair<const Key, Val>;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = const value_type*;

		MapPair() = default;

		MapPair(const Key& k)
			requires std::is_copy_constructible_v<Key>
		: key(k) {
		}

		MapPair(Key&& k)
			requires std::is_move_constructible_v<Key>
		: key(std::move(k)) {
		}

		template<typename K, typename V>
		MapPair(K&& k, V&& v)
			requires (std::is_convertible_v<K, Key>&&
		std::is_convertible_v<V, Val>&&
			std::is_constructible_v<Key, K&&>&&
			std::is_constructible_v<Val, V&&>)
			: key(std::forward<K>(k))
			, val(std::forward<V>(v))
		{
		}

		// Explicit conversion to std::pair
		explicit operator std::pair<const Key&, Val&>() noexcept {
			return { key, val };
		}

		explicit operator std::pair<const Key&, const Val&>() const noexcept {
			return { key, val };
		}

		// Explicit conversion from std::pair
		template<typename K, typename V>
		explicit MapPair(std::pair<K, V>&& p)
			requires (std::conditional_t<std::is_lvalue_reference_v<K>,
		std::is_copy_constructible<Key>,
			std::is_move_constructible<Key>>::value&&
			std::conditional_t<std::is_lvalue_reference_v<V>,
			std::is_copy_constructible<Val>,
			std::is_move_constructible<Val>>::value&&
			std::is_convertible_v<std::decay_t<K>, Key>&&
			std::is_convertible_v<std::decay_t<V>, Val>)
			: key(std::forward<K>(p.first))
			, val(std::forward<V>(p.second))
		{
		}

		// Helper methods to get as std::pair
		std::pair<const Key&, Val&> asPair() noexcept {
			return { key, val };
		}

		std::pair<const Key&, const Val&> asPair() const noexcept {
			return { key, val };
		}

		MapPair(const MapPair&) requires std::is_copy_constructible_v<Key>&& std::is_copy_constructible_v<Val> ||
			std::is_trivially_copy_constructible_v<Key> && std::is_trivially_copy_constructible_v<Val> = default;
		MapPair& operator=(const MapPair&) requires std::is_copy_assignable_v<Key>&& std::is_copy_assignable_v<Val> ||
			std::is_trivially_copy_assignable_v<Key> && std::is_trivially_copy_assignable_v<Val> = default;

		MapPair(MapPair&&) requires std::is_move_constructible_v<Key>&& std::is_move_constructible_v<Val> ||
			std::is_trivially_move_constructible_v<Key> && std::is_trivially_move_constructible_v<Val> = default;
		MapPair& operator=(MapPair&&) requires std::is_move_assignable_v<Key>&& std::is_move_assignable_v<Val> ||
			std::is_trivially_move_assignable_v<Key> && std::is_trivially_move_assignable_v<Val> = default;

		const Key& getKey() const noexcept { return key; }
		const Val& getValue() const noexcept { return val; }
		Val& getValue() noexcept { return val; }
	};

	template<typename K, typename V>
	MapPair(K, V) -> MapPair<std::decay_t<K>, std::decay_t<V>>;

}