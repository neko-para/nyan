#pragma once

#include "../paging/translator.hpp"

namespace nyan::task {

struct Stack {
    paging::VirtualAddress userBase;
    uint32_t* current;
    paging::Translator translator;

    Stack() noexcept {
        userBase = paging::VirtualAddress{allocator::frameAlloc()};
        current = userBase.nextPage().as<uint32_t>();
    }
    Stack(paging::UserDirectory& pageDir, paging::VirtualAddress userTop) noexcept {
        userBase = userTop.prevPage();
        auto stackBase = translator.allocEntry<uint32_t>(pageDir, userBase, true);
        current = paging::VirtualAddress{stackBase}.nextPage().as<uint32_t>();
    }

    paging::VirtualAddress esp() const noexcept { return paging::VirtualAddress{current}; }
    paging::VirtualAddress userEsp() const noexcept { return translator.toUser(esp()); }
    paging::VirtualAddress pushString(lib::string_view str) noexcept {
        size_t size = str.size() + 1;
        size = (size + 3) >> 2;
        current -= size;
        auto ptr = std::copy(str.begin(), str.end(), reinterpret_cast<char*>(current));
        std::fill_n(ptr, (size << 2) - str.size(), 0);
        return paging::VirtualAddress{current};
    }

    void pushVal(uint32_t value) noexcept { *--current = value; }

    template <typename T>
        requires std::is_pointer_v<T>
    void pushPtr(T ptr) noexcept {
        *--current = reinterpret_cast<uint32_t>(ptr);
    }

    // 没有考虑align问题
    template <typename T>
    T* pushAny(const T& val) {
        current = current - (sizeof(T) >> 2);
        auto ptr = reinterpret_cast<T*>(current);
        *ptr = val;
        return ptr;
    }
};

}  // namespace nyan::task
