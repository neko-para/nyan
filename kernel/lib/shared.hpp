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
    Shared* __ptr{};

    Ref() noexcept = default;
    Ref(T* ptr) noexcept : __ptr(ptr) {}
    Ref(const Ref& ref) noexcept : __ptr(ref.__ptr) {
        if (__ptr) {
            __ptr->ref();
        }
    }
    Ref(Ref&& ref) noexcept : __ptr(ref.__ptr) { ref.__ptr = nullptr; }
    ~Ref() {
        if (__ptr) {
            __ptr->unref();
        }
    }
    Ref& operator=(const Ref& ref) noexcept {
        if (this == &ref) {
            return *this;
        }
        if (__ptr) {
            __ptr->Shared::unref();
        }
        __ptr = ref.__ptr;
        if (__ptr) {
            __ptr->Shared::ref();
        }
        return *this;
    }
    Ref& operator=(Ref&& ref) noexcept {
        if (this == &ref) {
            return *this;
        }
        if (__ptr) {
            __ptr->Shared::unref();
        }
        __ptr = ref.__ptr;
        ref.__ptr = nullptr;
        return *this;
    }

    operator bool() const noexcept { return __ptr; }
    T* get() const noexcept { return static_cast<T*>(__ptr); }
    T* operator->() const noexcept { return static_cast<T*>(__ptr); }
    template <typename U>
    U* as() const noexcept {
        return static_cast<U*>(__ptr);
    }
    // TODO: cast ref itself

    template <typename U>
        requires std::is_convertible_v<T*, U*>
    operator Ref<U>() const noexcept {
        __ptr->ref();
        return Ref<U>{static_cast<T*>(__ptr)};
    }
};

template <typename T, typename... Args>
inline Ref<T> makeRef(Args&&... args) {
    auto ref = Ref<T>{new T(std::forward<Args>(args)...)};
    ref->onRefInc();
    return ref;
}

}  // namespace nyan::lib
