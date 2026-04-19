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

template <typename Base>
struct List {
    Base* head{};

    operator bool() const noexcept { return head; }

    auto operator->() const noexcept { return head; }

    template <same_as_any_in_tuple<typename Base::AllTags> Tag = Base::TheTag>
    void pushFront(tag_target_t<Tag>* item) noexcept {
        item->ListNode<Tag>::next = head;
        if constexpr (tag_is_bidi<Tag>) {
            if (head) {
                head->ListNode<Tag>::prev = item;
            }
            item->ListNode<Tag>::prev = nullptr;
        }
        head = item;
    }

    template <same_as_any_in_tuple<typename Base::AllTags> Tag = Base::TheTag>
    tag_target_t<Tag>* popFront() noexcept {
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

    template <same_as_any_in_tuple<typename Base::AllTags> Tag = Base::TheTag>
        requires tag_is_bidi<Tag>
    tag_target_t<Tag>* take(tag_target_t<Tag>* item) noexcept {
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

template <typename Base>
struct TailList : public List<Base> {
    Base* tail{};

    template <same_as_any_in_tuple<typename Base::AllTags> Tag = Base::TheTag>
    void pushFront(tag_target_t<Tag>* item) noexcept {
        if (!tail) {
            tail = item;
        }
        List<Base>::template pushFront<Tag>(item);
    }

    template <same_as_any_in_tuple<typename Base::AllTags> Tag = Base::TheTag>
    tag_target_t<Tag>* popFront() noexcept {
        if (List<Base>::head == tail) {
            tail = nullptr;
        }
        return List<Base>::template popFront<Tag>();
    }

    template <same_as_any_in_tuple<typename Base::AllTags> Tag = Base::TheTag>
        requires tag_is_bidi<Tag>
    tag_target_t<Tag>* take(tag_target_t<Tag>* item) noexcept {
        if (tail == item) {
            tail = item->ListNode<Tag>::prev;
        }
        return List<Base>::template take<Tag>(item);
    }

    template <same_as_any_in_tuple<typename Base::AllTags> Tag = Base::TheTag>
    void pushBack(tag_target_t<Tag>* item) noexcept {
        if (tail) {
            tail->ListNode<Tag>::next = item;
        }
        item->ListNode<Tag>::next = nullptr;
        if constexpr (tag_is_bidi<Tag>) {
            item->ListNode<Tag>::prev = tail;
        }
        if (!tail) {
            List<Base>::head = item;
        }
        tail = item;
    }

    template <same_as_any_in_tuple<typename Base::AllTags> Tag = Base::TheTag>
        requires tag_is_bidi<Tag>
    tag_target_t<Tag>* popBack() noexcept {
        if (!tail) {
            return nullptr;
        }
        if (List<Base>::head == tail) {
            auto item = tail;
            List<Base>::head = tail = nullptr;
            return item;
        }
        auto item = tail;
        tail->ListNode<Tag>::prev->ListNode<Tag>::next = nullptr;
        tail = tail->ListNode<Tag>::prev;
        return item;
    }

    template <same_as_any_in_tuple<typename Base::AllTags> Tag = Base::TheTag, typename Base2>
        requires same_as_any_in_tuple<Tag, typename Base2::AllTags>
    void appendBack(TailList<Base2>& list) noexcept {
        if (!tail) {
            List<Base>::head = list.head;
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
