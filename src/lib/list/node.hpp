#pragma once

#include <tuple>

#include "concepts.hpp"

namespace nyan::lib::__list {

template <typename Tag>
struct ListNode {
    tag_target_t<Tag>* next{};
    tag_target_t<Tag>* prev{};

    void __clear() noexcept {
        next = nullptr;
        prev = nullptr;
    }
};

template <typename... Tags>
struct ListNodes : public ListNode<Tags>... {
    ListNodes() = default;
    ListNodes(const ListNodes&) = delete;
    ListNodes(ListNodes&&) = delete;
    ListNodes& operator=(const ListNodes&) = delete;
    ListNodes& operator=(ListNodes&&) = delete;
};

}  // namespace nyan::lib::__list
