#pragma once

#include <string.h>
#include <type_traits>

#include "../arch/print.hpp"

namespace nyan::lib {

namespace __function {

struct base_vtable {
    void (*destroy)(void* obj) = nullptr;
    void (*copy)(void* obj, const void* src) = nullptr;
    void (*move)(void* obj, const void* src) = nullptr;
};

template <size_t N>
inline void trivial_copy(void* obj, const void* src) {
    memcpy(obj, src, N);
}

template <typename T>
inline void fill_base_vtable(base_vtable& vt) {
    if constexpr (std::is_trivially_destructible_v<T>) {
        vt.destroy = nullptr;
    } else {
        vt.destroy = +[](void* obj) { reinterpret_cast<T*>(obj)->~T(); };
    }
    if constexpr (std::is_trivially_copy_constructible_v<T>) {
        vt.copy = trivial_copy<sizeof(T)>;
    } else {
        vt.copy = +[](void* obj, const void* src) { new (obj) T(*reinterpret_cast<const T*>(src)); };
    }
    if constexpr (std::is_trivially_move_constructible_v<T>) {
        vt.move = trivial_copy<sizeof(T)>;
    } else {
        vt.move =
            +[](void* obj, const void* src) { new (obj) T(std::move(*reinterpret_cast<T*>(const_cast<void*>(src)))); };
    }
}

template <typename Ret, typename... Args>
struct callable_vtable : public base_vtable {
    Ret (*call)(const void* obj, Args... args) = nullptr;
};

template <typename T, typename Ret, typename... Args>
inline void fill_callable_vtable(callable_vtable<Ret, Args...>& vt) {
    vt.call = +[](const void* obj, Args... args) -> Ret {
        return (*reinterpret_cast<const T*>(obj))(std::forward<Args>(args)...);
    };
}

}  // namespace __function

template <typename, size_t Size = 3>
struct function;

template <typename Ret, typename... Args, size_t Size>
struct function<Ret(Args...), Size> {
    constexpr static size_t buffer_size = Size * sizeof(uint32_t);

    __function::callable_vtable<Ret, Args...>* vt = nullptr;
    alignas(uint32_t) uint8_t func[buffer_size];

    function() noexcept = default;
    template <typename F>
    function(F&& f) noexcept {
        using T = std::decay_t<F>;
        static_assert(sizeof(T) <= buffer_size);

        static __function::callable_vtable<Ret, Args...> table;
        if (!table.destroy) {
            __function::fill_base_vtable<T>(table);
            __function::fill_callable_vtable<T>(table);
        }

        new (func) T(std::move(f));
        vt = &table;
    }

    function(const function& f) noexcept {
        vt = f.vt;
        if (vt && vt->copy) {
            vt->copy(func, f.func);
        }
    }

    function(function&& f) noexcept {
        vt = f.vt;
        if (vt && vt->move) {
            vt->move(func, f.func);
        }
        f.vt = nullptr;
    }

    ~function() noexcept {
        if (vt && vt->destroy) {
            vt->destroy(func);
        }
    }

    function& operator=(const function& f) noexcept {
        if (this == &f) {
            return *this;
        }
        reset();
        vt = f.vt;
        if (vt && vt->copy) {
            vt->copy(func, f.func);
        }
        return *this;
    }

    function& operator=(function&& f) noexcept {
        if (this == &f) {
            return *this;
        }
        reset();
        vt = f.vt;
        if (vt && vt->move) {
            vt->move(func, f.func);
        }
        f.vt = nullptr;
        return *this;
    }

    Ret operator()(Args... args) const noexcept {
        if (vt && vt->call) {
            return vt->call(func, std::forward<Args>(args)...);
        } else {
            arch::kfatal("function is empty");
        }
    }

    operator bool() const noexcept { return vt; }

    void reset() noexcept {
        if (vt && vt->destroy) {
            vt->destroy(func);
        }
        vt = nullptr;
    }
};

}  // namespace nyan::lib
