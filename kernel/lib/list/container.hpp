#pragma once

#include <variant>

#include "concepts.hpp"
#include "node.hpp"

namespace nyan::lib::__list {

template <typename Tag, bool Const>
struct ListIterator {
    using __value = tag_target_t<Tag>;
    using value_type = std::conditional_t<Const, const __value, __value>;

    __value* __ptr{};

    auto& __next() const noexcept { return __ptr->ListNode<Tag>::next; }
    auto& __prev() const noexcept { return __ptr->ListNode<Tag>::prev; }
    bool __is_begin() const noexcept { return !__prev(); }
    bool __is_end() const noexcept { return !__ptr; }
    ListIterator<Tag, false> __mutable() const noexcept { return {__ptr}; }

    ListIterator& operator++() noexcept {
        __ptr = __next();
        return *this;
    }

    ListIterator operator++(int) noexcept {
        auto old = *this;
        __ptr = __next();
        return old;
    }

    ListIterator& operator--() noexcept {
        __ptr = __prev();
        return *this;
    }

    ListIterator operator--(int) noexcept {
        auto old = *this;
        __ptr = __prev();
        return old;
    }

    template <bool OtherConst>
    bool operator==(const ListIterator<Tag, OtherConst>& iter) const noexcept {
        if (__ptr == iter.__ptr) {
            return true;
        }
        if (__is_end() && iter.__is_end()) {
            return true;
        }
        return false;
    }

    value_type& operator*() const { return *__ptr; }
    value_type* const operator->() const { return __ptr; }

    operator ListIterator<Tag, true>() const noexcept
        requires(!Const)
    {
        return {__ptr};
    }
};

template <typename Tag, bool WithTail>
struct List {
    using __value = tag_target_t<Tag>;
    using __tail_type = std::conditional_t<WithTail, __value*, std::monostate>;

    using value_type = __value;
    using reference = value_type&;
    using const_reference = const value_type&;
    using iterator = ListIterator<Tag, false>;
    using const_iterator = ListIterator<Tag, true>;
    using difference_type = std::ptrdiff_t;
    using size_type = size_t;

    value_type* __head{};
    [[no_unique_address]] __tail_type __tail{};

    void clear() noexcept {
        __head = nullptr;
        if constexpr (WithTail) {
            __tail = nullptr;
        }
    }
    bool empty() const noexcept { return !__head; }

    iterator begin() noexcept { return {__head}; }
    iterator end() noexcept { return {nullptr}; }
    const_iterator begin() const noexcept { return {__head}; }
    const_iterator end() const noexcept { return {nullptr}; }
    const_iterator cbegin() const noexcept { return {__head}; }
    const_iterator cend() const noexcept { return {nullptr}; }

    void __insert_inclusive(const_iterator pos, value_type* from, value_type* to) {
        if (empty()) {
            from->ListNode<Tag>::prev = nullptr;
            to->ListNode<Tag>::next = nullptr;
            __head = from;
            if constexpr (WithTail) {
                __tail = to;
            }
        } else if (pos.__is_end()) {
            if constexpr (WithTail) {
                from->ListNode<Tag>::prev = __tail;
                to->ListNode<Tag>::next = nullptr;
                __tail->ListNode<Tag>::next = from;
                __tail = to;
            } else {
                __builtin_unreachable();
            }
        } else if (pos.__is_begin()) {
            from->ListNode<Tag>::prev = nullptr;
            to->ListNode<Tag>::next = __head;
            __head->ListNode<Tag>::prev = to;
            __head = from;
        } else {
            from->ListNode<Tag>::prev = pos.__prev();
            to->ListNode<Tag>::next = pos.__ptr;
            pos.__prev()->ListNode<Tag>::next = from;
            pos.__prev() = to;
        }
    }

    void __extract_inclusive(value_type* from, value_type* to) {
        if (__head == from) {
            if (!to || !to->ListNode<Tag>::next) {
                clear();
            } else {
                __head = to->ListNode<Tag>::next;
                to->ListNode<Tag>::next = nullptr;
                __head->ListNode<Tag>::prev = nullptr;
            }
        } else {
            if (!to || !to->ListNode<Tag>::next) {
                if constexpr (WithTail) {
                    __tail = from->ListNode<Tag>::prev;
                    __tail->ListNode<Tag>::next = nullptr;
                } else {
                    auto tail = from->ListNode<Tag>::prev;
                    tail->ListNode<Tag>::next = nullptr;
                }
                from->ListNode<Tag>::prev = nullptr;
            } else {
                from->ListNode<Tag>::prev->ListNode<Tag>::next = to->ListNode<Tag>::next;
                to->ListNode<Tag>::next->ListNode<Tag>::prev = from->ListNode<Tag>::prev;
                from->ListNode<Tag>::prev = nullptr;
                to->ListNode<Tag>::next = nullptr;
            }
        }
    }

    const value_type* front() const noexcept { return __head; }
    value_type* front() noexcept { return __head; }
    const value_type* back() const noexcept
        requires WithTail
    {
        return __tail;
    }
    value_type* back() noexcept
        requires WithTail
    {
        return __tail;
    }

    void push_front(value_type* item) noexcept { __insert_inclusive(begin(), item, item); }

    void pop_front() noexcept { __extract_inclusive(__head, __head); }

    void push_back(value_type* item) noexcept
        requires WithTail
    {
        __insert_inclusive(end(), item, item);
    }

    void pop_back() noexcept
        requires WithTail
    {
        __extract_inclusive(__tail, __tail);
    }

    iterator erase(const_iterator pos) noexcept {
        auto next = pos.__next();
        __extract_inclusive(pos.__ptr, pos.__ptr);
        return {next};
    }

    iterator erase(const_iterator from, const_iterator to) noexcept {
        if (from == to) {
            return to.__mutable();
        }
        if (to.__is_end()) {
            __extract_inclusive(from.__ptr, nullptr);
            return {nullptr};
        } else {
            __extract_inclusive(from.__ptr, to.__prev());
            return to.__mutable();
        }
    }

    iterator insert(const_iterator pos, value_type* item) {
        __insert_inclusive(pos, item, item);
        return {item};
    }

    void splice(const_iterator pos, List<Tag, true>& list) noexcept {
        if (list.empty()) {
            return;
        }
        __insert_inclusive(pos, list.__head, list.__tail);
        list.clear();
    }

    void splice(const_iterator pos, List<Tag, true>& list, const_iterator item) noexcept {
        list.__extract_inclusive(item.__ptr, item.__ptr);
        __insert_inclusive(pos, item.__ptr, item.__ptr);
    }

    void splice(const_iterator pos, List<Tag, true>& list, const_iterator from, const_iterator to) noexcept {
        list.__extract_inclusive(from.__ptr, to.__ptr);
        __insert_inclusive(pos, from.__ptr, to.__ptr);
    }
};

}  // namespace nyan::lib::__list
