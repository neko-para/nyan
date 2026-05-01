#pragma once

#include <stdint.h>
#include <type_traits>

namespace nyan::lib {

struct Shared {
    int32_t __ref_count{1};

    Shared() = default;
    Shared(const Shared&) = delete;
    virtual ~Shared() = default;
    Shared& operator=(const Shared&) = delete;

    void ref() noexcept {
        __ref_count++;
        onRefInc();
    }
    void unref() noexcept {
        __ref_count--;
        onRefDec();
        if (!__ref_count) {
            release();
        }
    }
    void release() noexcept { delete this; }

    virtual void onRefInc() noexcept {}
    virtual void onRefDec() noexcept {}
};

template <typename T>
struct Ref {
    T* ptr{};

    Ref() noexcept = default;
    Ref(T* ptr) noexcept : ptr(ptr) {}
    Ref(const Ref& ref) noexcept : ptr(ref.ptr) {
        if (ptr) {
            ptr->Shared::ref();
        }
    }
    Ref(Ref&& ref) noexcept : ptr(ref.ptr) { ref.ptr = nullptr; }
    ~Ref() {
        if (ptr) {
            ptr->Shared::unref();
        }
    }
    Ref& operator=(const Ref& ref) noexcept {
        if (this == &ref) {
            return *this;
        }
        if (ptr) {
            ptr->Shared::unref();
        }
        ptr = ref.ptr;
        if (ptr) {
            ptr->Shared::ref();
        }
        return *this;
    }
    Ref& operator=(Ref&& ref) noexcept {
        if (this == &ref) {
            return *this;
        }
        if (ptr) {
            ptr->Shared::unref();
        }
        ptr = ref.ptr;
        ref.ptr = nullptr;
        return *this;
    }

    operator bool() const noexcept { return ptr; }
    T* get() const noexcept { return ptr; }
    T* operator->() const noexcept { return ptr; }
    template <typename U>
    U* as() const noexcept {
        return static_cast<U*>(ptr);
    }
    // TODO: cast ref itself

    template <typename U>
        requires std::is_convertible_v<T*, U*>
    operator Ref<U>() const noexcept {
        ptr->Shared::ref();
        return Ref<U>{ptr};
    }
};

template <typename T, typename... Args>
inline Ref<T> makeRef(Args&&... args) {
    auto ref = Ref<T>{new T(std::forward<Args>(args)...)};
    ref->onRefInc();
    return ref;
}

}  // namespace nyan::lib
