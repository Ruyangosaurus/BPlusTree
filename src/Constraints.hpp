#ifndef CONSTRAINTS_HEADER_DEFINED
#define CONSTRAINTS_HEADER_DEFINED

#include <concepts>

/// @brief A concept that ensures a type is move-constructible, copy-constructible, and destructible.
///
/// @requirements:
/// * `T` must be move-constructible with no exceptions.
/// * `T` must be move-assignable with no exceptions.
/// * `T` must be destructible.
template<typename T>
concept Storable = std::destructible<T> && requires (){
    std::is_nothrow_move_constructible<T>::value && std::is_nothrow_move_assignable<T>::value;
    };

/// @brief A concept that ensures a type is both storable and totally ordered.
///
/// @requirements:
/// * `T` must be move-constructible with no exceptions.
/// * `T` must be move-assignable with no exceptions.
/// * `T` must be destructible.
/// * `Key` must supports the comparison operations '<' and '=='.
template<typename Key>
concept OrderedKey = Storable<Key> && std::totally_ordered<Key>;

/// @brief A concept that ensures a type has a method named allocate.
///
/// @requirements:
/// * `T` must have a method named allocate that takes no arguments and returns a pointer.
template<class Allocator>
concept has_allocate = requires (Allocator a){
    {a.allocate()} -> std::same_as<typename Allocator::pointer>;
};


/// @brief A concept that ensures a type has a method named deallocate.
///
/// @requirements:
/// * `T` must have a method named deallocate that takes a pointer.
template<class Allocator>
concept has_deallocate = requires (Allocator a, typename Allocator::pointer p){
    {a.deallocate(p)};
};


/// @brief A concept that ensures a type is fit to be a single element allocator.
///
/// @requirements:
/// * `T` must have a method named allocate that takes no arguments and returns a pointer.
/// * `T` must have a method named deallocate that takes a pointer.
template<typename Alloc>
concept SingleElementAllocator = has_deallocate<Alloc> && has_allocate<Alloc>;

#endif
