#pragma once

#include <concepts>
#include <tuple>

#include "isr.hpp"

namespace nyan::interrupt {

template <typename T>
concept is_syscall_frame = std::same_as<T, SyscallFrame*>;

template <typename T>
inline T castArg(uint32_t val) {
    if constexpr (std::is_pointer_v<T>) {
        return reinterpret_cast<T>(val);
    } else {
        return static_cast<T>(val);
    }
}

template <typename T>
inline uint32_t castRet(T val) {
    if constexpr (std::is_pointer_v<T>) {
        return reinterpret_cast<uint32_t>(val);
    } else {
        return static_cast<uint32_t>(val);
    }
}

template <typename Ret, typename... Args>
inline void call(SyscallFrame* frame, Ret (*func)(Args...)) {
    using argsType = std::tuple<Args...>;
    constexpr size_t argsCount = sizeof...(Args);

    if constexpr (argsCount == 0) {
        if constexpr (std::same_as<Ret, void>) {
            frame->eax = 0;
            func();
        } else {
            frame->eax = castRet(func());
        }
        return;
    } else {
        constexpr bool withFrame = is_syscall_frame<std::tuple_element_t<argsCount - 1, argsType>>;
        constexpr size_t realArgsCount = withFrame ? argsCount - 1 : argsCount;

        constexpr uint32_t SyscallFrame::* regAddrs[] = {
            &SyscallFrame::ebx, &SyscallFrame::ecx, &SyscallFrame::edx,
            &SyscallFrame::esi, &SyscallFrame::edi, &SyscallFrame::ebp,
        };

        static_assert(realArgsCount <= 6);

        std::tuple<Args...> args;
        [&]<size_t... Is>(std::index_sequence<Is...>) {
            ((std::get<Is>(args) = castArg<std::tuple_element_t<Is, argsType>>(frame->*(regAddrs[Is]))), ...);
        }(std::make_index_sequence<realArgsCount>());

        if constexpr (withFrame) {
            std::get<argsCount - 1>(args) = frame;
        }

        if constexpr (std::same_as<Ret, void>) {
            if constexpr (!withFrame) {
                frame->eax = 0;
            }
            std::apply(func, args);
        } else {
            frame->eax = castRet(std::apply(func, args));
        }
    }
}

}  // namespace nyan::interrupt
