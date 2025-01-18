#ifndef CONSTRAINTS_HEADER_DEFINED
#define CONSTRAINTS_HEADER_DEFINED

#include <concepts>

/// @brief A concept that ensures a type is move-constructible, copy-constructible, and destructible.
///
/// @requirements:
/// * `T` must be move-constructible (`std::move_constructible<T>`).
/// * `T` must be copy-constructible (`std::copy_constructible<T>`).
/// * `T` must be destructible (`std::destructible<T>`).
template<typename T>
concept Storable = std::move_constructible<T> && std::copy_constructible<T> && std::destructible<T>;

/// @brief A concept that ensures a type is both storable and totally ordered.
///
/// @requirements:
/// * `Key` must be move-constructible (`std::move_constructible<Key>`).
/// * `Key` must be copy-constructible (`std::copy_constructible<Key>`).
/// * `Key` must be destructible (`std::destructible<Key>`).
/// * `Key` must supports the comparison operations '<' and '==' (`std::totally_ordered<Key>`).
template<typename Key>
concept OrderedKey = Storable<Key> && std::totally_ordered<Key>;

template<class Allocator>
concept has_allocate = requires (Allocator a){
    {a.allocate()} -> std::same_as<typename Allocator::pointer>;
};

template<class Allocator>
concept has_deallocate = requires (Allocator a, typename Allocator::pointer p){
    {a.deallocate(p)};
};

template<typename Alloc>
concept SingleElementAllocator = has_deallocate<Alloc> && has_allocate<Alloc>;

#endif
