# nyan — hobby OS kernel

> i686 (x86-32) hobby 操作系统内核，使用 C++20 编写，Clang/LLVM 工具链交叉编译，Multiboot 引导。

## 目录结构总览

```
nyan/
├── CMakeLists.txt              # 顶层 CMake，定义 kernel_header INTERFACE 库，子目录 user/ 和 kernel/
├── CMakePresets.json            # MacNinja / WinNinja 两种预设 (Ninja + LLVM toolchain)
├── i686-elf-mac-with-sysroot.cmake   # macOS 交叉编译工具链 (clang target=i686-elf, sysroot)
├── i686-elf-win-with-sysroot.cmake   # Windows 交叉编译工具链
│
├── include/                    # ★ 内核/用户态共享 ABI 头文件
│   ├── nyan/
│   │   ├── types.h             # __NYAN_BEGIN__/__NYAN_END__ 宏；__NYAN_SYSCALL__ 宏
│   │   │                       #   内核态展开为 namespace nyan::syscall，用户态展开为 extern "C" sys_xxx
│   │   ├── syscall.h           # 所有系统调用声明 (exit/fork/read/write/close/waitpid/execve/brk/pipe/dup/signal...)
│   │   ├── errno.h             # SYS_Exxx 错误码 (Linux 兼容编号)
│   │   └── ioctls.h            # ioctl 请求号
│   ├── sys/
│   │   ├── types.h             # pid_t, ssize_t, time_t, timespec
│   │   ├── signal.h            # sigaction, sigset_t, NSIG, sighandler_t
│   │   ├── wait.h              # WIFEXITED / WTERMSIG 等宏
│   │   └── ioctl.h
│   └── bits/
│       ├── fcntl.h             # O_RDONLY / O_WRONLY / O_RDWR
│       └── signum.h            # SIGINT / SIGKILL / SIGTERM 等
│
├── kernel/                     # ★ 内核实现
│   ├── CMakeLists.txt          # glob *.cpp *.s；链接 kernel_header, kernel_embed, libc, clang_rt
│   ├── linker.ld               # 链接脚本：加载地址 1M, 虚拟地址 0xC0000000+ (higher-half kernel)
│   ├── boot.s                  # Multiboot 入口 _start → 建立初始分页 → 跳转 kmain
│   ├── main.cpp                # kmain(): SSE → 清恒等映射 → 初始化各子系统 → task::yield → 收养孤儿循环
│   │
│   ├── arch/                   # x86 底层：SSE 使能、端口 IO、CR3/INVLPG、kprint 格式化打印
│   ├── boot/                   # multiboot.hpp: BootInfo / MMapEntry 结构
│   │
│   ├── gdt/                    # GDT + TSS
│   │   ├── entry.hpp           # GDT 表项结构
│   │   ├── load.hpp            # kernelCs=0x08, kernelDs=0x10, userCs=0x1B, userDs=0x23
│   │   ├── load.cpp            # GDT 加载
│   │   └── tss.hpp             # TSS 结构
│   │
│   ├── interrupt/              # IDT + ISR + PIC + 系统调用入口
│   │   ├── entry.hpp           # IDT 表项
│   │   ├── load.hpp            # PIC mask/unmask/end
│   │   ├── load.cpp            # IDT 初始化, PIC remap
│   │   ├── isr.hpp             # SyscallFrame 结构
│   │   ├── isr.cpp             # ISR 分发
│   │   ├── impl.cpp            # 具体中断处理
│   │   └── syscall.s           # int 0x80 入口汇编, 保存/恢复用户态寄存器
│   │
│   ├── paging/                 # 分页子系统 (二级页表, higher-half)
│   │   ├── address.hpp         # PhysicalAddress / VirtualAddress 强类型地址, 互转偏移 0xC0000000
│   │   ├── entry.hpp           # PDE/PTE 属性位
│   │   ├── table.hpp           # Page Table 操作
│   │   ├── directory.hpp       # KernelDirectory (直接映射) / UserDirectory (需 MapperGuard 临时映射)
│   │   ├── directory.cpp       # UserDirectory::fork, forkCOW, alloc, handleCOW
│   │   ├── translator.hpp      # 虚拟→物理地址翻译
│   │   ├── kernel.hpp/cpp      # clearIdentityPaging, 初始页目录 (boot 阶段 identity map → higher-half)
│   │
│   ├── allocator/              # 内存分配器
│   │   ├── alloc.hpp           # API: physicalFrameAlloc/Release, virtualFrameAlloc/Free, frameAlloc/Free, slabAlloc/Free
│   │   ├── frame.hpp           # 物理帧分配器 (bitmap)
│   │   ├── physicalFrame.hpp   # 物理帧引用计数 (COW 支持)
│   │   ├── pool.hpp            # 虚拟地址帧池
│   │   ├── slab.hpp/cpp        # Slab 分配器 (16/32/64/128/256/512 字节)
│   │   ├── stl.hpp             # SlabAllocator<T> — 兼容 STL 的分配器适配
│   │   └── load.hpp/cpp        # 初始化: 根据 multiboot mmap 确定物理内存上限
│   │
│   ├── task/                   # 进程管理
│   │   ├── forward.hpp         # State (Ready/Running/Exited/Blocked), BlockReason, WakeReason, KnownPid
│   │   ├── tcb.hpp             # TaskControlBlock: cr3, 内核栈, 用户栈, pid, 父子关系, 信号, fd表, brk, pages
│   │   ├── task.hpp            # API: createTask, createElfTask, execTask, forkTask, exitTask, yield, block, unblock
│   │   ├── task.cpp            # 调度器实现, ELF 加载, fork
│   │   ├── pid.hpp/cpp         # PID 分配
│   │   ├── signal.hpp          # SignalFrame
│   │   ├── stack.hpp           # 内核栈布局
│   │   ├── switch.hpp/s        # 上下文切换汇编
│   │   ├── trampoline.h/s      # Ring0→Ring3 跳板代码 (拷贝到用户空间执行 sigreturn)
│   │   ├── wait.hpp/cpp        # WaitList 等待队列
│   │
│   ├── syscall/                # 系统调用实现 (每个文件对应一个 syscall)
│   │   ├── brk.cpp             # brk (堆扩展)
│   │   ├── close.cpp           # close
│   │   ├── dup.cpp / dup2.cpp  # dup / dup2
│   │   ├── execve.cpp          # execve (从内嵌 ELF 按名查找)
│   │   ├── exit.cpp            # exit
│   │   ├── fork.cpp            # fork (COW)
│   │   ├── getpid.cpp          # getpid
│   │   ├── ioctl.cpp           # ioctl
│   │   ├── kill.cpp            # kill (发送信号)
│   │   ├── nanosleep.cpp       # nanosleep
│   │   ├── pipe.cpp            # pipe
│   │   ├── read.cpp / write.cpp # read / write
│   │   ├── signal.cpp          # signal (注册信号处理器)
│   │   ├── sigreturn.cpp       # sigreturn
│   │   ├── spawn.cpp           # spawn (非标准, 直接创建 ELF 进程)
│   │   └── waitpid.cpp         # waitpid
│   │
│   ├── fs/                     # 文件系统抽象
│   │   ├── file.hpp            # FileObj (虚基类: read/write/ioctl)
│   │   ├── fd.hpp/cpp          # FdObj (文件描述符, 持有 Ref<FileObj> + 打开模式)
│   │   ├── pipe.hpp/cpp        # PipeFile (管道实现, 环形缓冲区 + 等待队列)
│   │
│   ├── console/                # 终端 / TTY
│   │   ├── forward.hpp         # 前置声明
│   │   ├── buffer.hpp/cpp      # ScreenBuffer (80×25 VGA 文本缓冲)
│   │   ├── tty.hpp/cpp         # Tty (继承 ScreenBuffer, 行编辑, 输入缓冲, 前台进程组)
│   │   ├── file.hpp/cpp        # ConsoleFile (FileObj 实现, 连接 tty)
│   │   ├── entry.cpp           # 控制台守护进程入口
│   │   └── load.hpp/cpp        # 初始化: 2 个 tty, F1/F2 切换
│   │
│   ├── keyboard/               # PS/2 键盘驱动
│   │   ├── message.hpp/cpp     # 按键消息 / 扫描码→字符转换
│   │   └── load.hpp/cpp        # IRQ1 处理
│   │
│   ├── timer/                  # PIT 定时器
│   │   └── load.hpp/cpp        # IRQ0, tick 计数, 抢占式调度触发
│   │
│   ├── vga/                    # VGA 硬件
│   │   ├── buffer.hpp          # VGA 文本模式地址 (0xC03FF000, 由 linker.ld 映射)
│   │   ├── cursor.hpp          # 硬件光标控制
│   │   └── entry.hpp           # VGA 颜色常量
│   │
│   ├── elf/                    # ELF 加载器
│   │   └── entry.hpp           # ELF32 头/段解析
│   │
│   ├── data/                   # 内嵌用户程序数据
│   │   ├── embed.hpp           # EmbedProgram 结构 (data, size, name)
│   │   └── macro.hpp           # DECLARE_PROGRAM_DATA / DECLARE_PROGRAM 宏
│   │
│   ├── lib/                    # 内核库
│   │   ├── containers.hpp      # lib::string, lib::vector, lib::deque, lib::unique_ptr (均使用 Slab 分配器)
│   │   ├── list.hpp            # 侵入式双向链表 (tagged, 支持多链表节点)
│   │   ├── shared.hpp          # 引用计数基类 Shared + Ref<T> 智能指针
│   │   ├── function.hpp        # lib::function (类似 std::function, Slab 分配)
│   │   ├── queue.hpp           # 队列
│   │   ├── concepts.hpp        # C++20 concepts
│   │   └── format/             # 内核 kprint 格式化实现
│   │
│   └── runtime/                # freestanding 运行时补丁
│       ├── clang.cpp           # __cxa_atexit, __cxa_guard 等
│       ├── libc.cpp            # memset/memcpy 等 (调用 llvm-libc)
│       └── libcxx.cpp          # operator new/delete → slabAlloc/slabFree
│
├── user/                       # ★ 用户态
│   ├── CMakeLists.txt          # 子目录 lib/ 和 entry/
│   │
│   ├── include/                # 用户态公开头文件 (POSIX-like)
│   │   ├── stdio.h             # printf, fprintf, fgets, fputc, fflush, FILE*, stdin/stdout/stderr
│   │   ├── stdlib.h            # exit, malloc/free (通过 brk)
│   │   ├── string.h            # strlen, strcmp, strchr, memset, memcpy...
│   │   ├── unistd.h            # fork, execve, read, write, close, pipe, dup, dup2, getpid, sleep, brk
│   │   ├── signal.h            # signal()
│   │   ├── ctype.h             # isdigit, isalpha...
│   │   ├── errno.h             # errno → SYS_Exxx
│   │   └── fcntl.h             # O_RDONLY 等
│   │
│   ├── lib/                    # 用户态 libc 实现
│   │   ├── CMakeLists.txt      # 静态库 "user", 链接 kernel_header
│   │   ├── include/
│   │   │   └── stdio_impl.h    # FILE 内部结构
│   │   └── src/
│   │       ├── start.s         # _start → 调用 main → sys_exit
│   │       ├── syscall.cpp     # int 0x80 系统调用封装
│   │       ├── libc.cpp        # __libc_init_array 等
│   │       ├── stdio.cpp       # printf / FILE 缓冲 IO
│   │       ├── stdlib.cpp      # exit, malloc/free (brk)
│   │       ├── string.cpp      # 字符串函数
│   │       ├── unistd.cpp      # POSIX 函数封装
│   │       ├── signal.cpp      # signal() 封装
│   │       ├── ctype.cpp       # ctype 函数
│   │       ├── errno.cpp       # errno 全局变量
│   │       └── ioctl.cpp       # ioctl 封装
│   │
│   └── entry/                  # ★ 内嵌用户态程序 (编译为独立 ELF → objcopy 嵌入内核 .rodata)
│       ├── CMakeLists.txt      # 自动枚举 *.cpp, 每个编译为独立 ELF, objcopy 成 .o 嵌入 kernel_embed 库
│       ├── utils.hpp           # 用户程序工具函数
│       ├── sh.cpp              # shell: 读行 → fork+execve, 支持 exec 内建命令
│       ├── cat.cpp             # cat
│       ├── echo.cpp            # echo
│       ├── sleep.cpp           # sleep
│       ├── kill.cpp            # kill
│       ├── test.cpp            # 测试程序
│       ├── true.cpp            # true (exit 0)
│       └── false.cpp           # false (exit 1)
│
└── sysroot/                    # 预编译的 freestanding sysroot
    └── usr/
        ├── include/            # LLVM libc + libc++ 头文件
        └── lib/                # libc.a, libc++.a, clang_rt.builtins 等静态库
```

## 关键架构细节

### 内存布局 (linker.ld)
- **物理加载地址**: 1MB (0x100000)
- **虚拟地址空间**: Higher-half kernel, 偏移 0xC0000000
  - 内核代码/数据: 0xC0100000+
  - VGA 文本缓冲区: 0xC03FF000 (映射自物理 0xB8000)
  - 内核堆上限: 0xC0400000
- **用户空间**: 0x00000000 ~ 0xBFFFFFFF
- **Multiboot**: `.multiboot.data` / `.multiboot.text` 在低地址运行 (identity map 阶段)
- **Trampoline**: 信号返回跳板代码 `.trampoline` 段

### 分页
- 二级页表 (Page Directory → Page Table), 4KB 页
- `KernelDirectory`: 静态分配, 直接通过 kernelToVirtual 访问
- `UserDirectory`: 通过 `MapperGuard` RAII 临时映射物理页来操作
- 支持 COW (Copy-on-Write) fork

### 进程模型
- `TaskControlBlock` 包含: cr3, 内核/用户栈, pid, 父子链表, 信号, fd 表 (最多 16 个), brk, 页列表
- PID 分配: 1=init, 2=idle, ≥16 为用户进程
- 调度: 简单轮转, PIT 定时器触发抢占
- 阻塞原因: Sleep / WaitInput / WaitTask
- fork 使用 COW, execve 从内嵌 ELF 按名查找

### 系统调用
- 通过 `int 0x80` 进入, 参数在寄存器中 (Linux i386 ABI 风格)
- `include/nyan/syscall.h` 中声明共享: 内核态展开为 `nyan::syscall::xxx(frame)`, 用户态展开为 `extern "C" sys_xxx()`
- 已实现: exit, fork, read, write, close, waitpid, execve, getpid, kill, dup, pipe, brk, signal, ioctl, dup2, sigreturn, nanosleep, spawn

### 文件系统
- 无磁盘文件系统; 仅有 `FileObj` 虚基类抽象
- 实现: ConsoleFile (TTY 读写), PipeFile (管道)
- FdObj 持有 `Ref<FileObj>` + 打开模式

### 终端
- 2 个 TTY (F1/F2 切换), 80×25 VGA 文本模式
- 行编辑模式 (canonical), 前台进程组, Ctrl+C → SIGINT

### 用户程序嵌入机制
- `user/entry/` 中每个 `.cpp` 编译为独立 i686-elf ELF 可执行文件
- 通过 `llvm-objcopy -I binary -O elf32-i386` 转为 `.o` 嵌入内核
- 链接为 `kernel_embed` 静态库, 符号 `_binary_<name>_bin_start/end`
- `nyan::data::programs[]` 数组记录所有嵌入程序, execve 时按名查找

### 工具链
- Clang/LLVM (路径 `/opt/llvm-22`), target `i686-elf`
- Freestanding: `-ffreestanding -nostdlib -fno-rtti -fno-exceptions -stdlib=libc++`
- C++20, sysroot 提供 LLVM libc + libc++ 头文件和静态库
- 构建系统: CMake + Ninja

### 内核库 (`kernel/lib/`)
- STL 容器通过 `SlabAllocator<T>` 适配使用 slab 分配器
- 侵入式双向链表 (`lib::List`) 使用 tag 区分不同链表
- 引用计数智能指针 (`lib::Ref<T>` + `lib::Shared` 基类)

### 启动流程
1. `boot.s` `_start`: 设置初始栈 → `preparePaging` (建立 identity + higher-half 映射) → 跳转高半
2. `kmain`: enableSse → clearIdentityPaging → __libc_init_array → 解析 multiboot mmap
3. 初始化: allocator → console → GDT → IDT/PIC → timer → keyboard → task
4. `console::loadDeamons`: 创建控制台守护进程 (每个 tty 一个, 运行 sh)
5. `task::yield` 开始调度 → kmain 本身成为 init 进程, 循环 waitpid 收养孤儿
