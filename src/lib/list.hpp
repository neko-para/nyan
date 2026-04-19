#pragma once

#include <concepts>
#include <tuple>

namespace nyan::lib {

template <typename Tag>
using tag_target_t = Tag::type;

template <typename Tag>
concept tag_has_bidi = requires {
    { Tag::bidi } -> std::convertible_to<bool>;
};

template <typename Tag>
constexpr bool tag_is_bidi_v = false;
template <tag_has_bidi Tag>
constexpr bool tag_is_bidi_v<Tag> = Tag::bidi;

template <typename Tag>
concept tag_is_bidi = tag_is_bidi_v<Tag>;

template <typename T, typename... Ts>
concept same_as_any = (std::same_as<T, Ts> || ...);

template <typename T, typename Tuple>
constexpr bool same_as_any_in_tuple_v = false;
template <typename T, typename... Ts>
constexpr bool same_as_any_in_tuple_v<T, std::tuple<Ts...>> = same_as_any<T, Ts...>;
template <typename T, typename Tuple>
concept same_as_any_in_tuple = same_as_any_in_tuple_v<T, Tuple>;

template <typename Tag>
struct ListNode {
    tag_target_t<Tag>* next{};
};

template <tag_is_bidi Tag>
struct ListNode<Tag> {
    tag_target_t<Tag>* next{};
    tag_target_t<Tag>* prev{};
};

template <typename... Tags>
struct ListBase : public ListNode<Tags>... {
    using AllTags = std::tuple<Tags...>;
    using TheTag = std::tuple_element_t<0, AllTags>;
};

template <typename Tag>
struct List {
    using Base = tag_target_t<Tag>;

    Base* head{};

    operator bool() const noexcept { return head; }

    auto operator->() const noexcept { return head; }

    void pushFront(Base* item) noexcept {
        item->ListNode<Tag>::next = head;
        if constexpr (tag_is_bidi<Tag>) {
            if (head) {
                head->ListNode<Tag>::prev = item;
            }
            item->ListNode<Tag>::prev = nullptr;
        }
        head = item;
    }

    Base* popFront() noexcept {
        if (!head) {
            return nullptr;
        }
        auto item = head;
        head = head->ListNode<Tag>::next;
        if constexpr (tag_is_bidi<Tag>) {
            if (head) {
                head->ListNode<Tag>::prev = nullptr;
            }
        }
        return item;
    }

    Base* take(Base* item) noexcept
        requires tag_is_bidi<Tag>
    {
        if (item->ListNode<Tag>::prev) {
            item->ListNode<Tag>::prev->ListNode<Tag>::next = item->ListNode<Tag>::next;
        } else {
            head = item->ListNode<Tag>::next;
        }
        if (item->ListNode<Tag>::next) {
            item->ListNode<Tag>::next->ListNode<Tag>::prev = item->ListNode<Tag>::prev;
        }
        return item;
    }
};

template <typename Tag>
struct TailList : public List<Tag> {
    using Base = tag_target_t<Tag>;

    Base* tail{};

    void pushFront(Base* item) noexcept {
        if (!tail) {
            tail = item;
        }
        List<Tag>::pushFront(item);
    }

    Base* popFront() noexcept {
        if (List<Tag>::head == tail) {
            tail = nullptr;
        }
        return List<Tag>::popFront();
    }

    Base* take(Base* item) noexcept
        requires tag_is_bidi<Tag>
    {
        if (tail == item) {
            tail = item->ListNode<Tag>::prev;
        }
        return List<Tag>::take(item);
    }

    void pushBack(Base* item) noexcept {
        if (tail) {
            tail->ListNode<Tag>::next = item;
        }
        item->ListNode<Tag>::next = nullptr;
        if constexpr (tag_is_bidi<Tag>) {
            item->ListNode<Tag>::prev = tail;
        }
        if (!tail) {
            List<Tag>::head = item;
        }
        tail = item;
    }

    tag_target_t<Tag>* popBack() noexcept
        requires tag_is_bidi<Tag>
    {
        if (!tail) {
            return nullptr;
        }
        if (List<Tag>::head == tail) {
            auto item = tail;
            List<Tag>::head = tail = nullptr;
            return item;
        }
        auto item = tail;
        tail->ListNode<Tag>::prev->ListNode<Tag>::next = nullptr;
        tail = tail->ListNode<Tag>::prev;
        return item;
    }

    void appendBack(TailList<Tag>& list) noexcept {
        if (!tail) {
            List<Tag>::head = list.head;
            tail = list.tail;
            list.head = nullptr;
            list.tail = nullptr;
            return;
        } else if (!list.head) {
            return;
        }
        tail->ListNode<Tag>::next = list.head;
        list.head->ListNode<Tag>::prev = tail;
        tail = list.tail;
        list.head = nullptr;
        list.tail = nullptr;
    }
};

}  // namespace nyan::lib
