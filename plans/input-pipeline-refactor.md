# 输入管线重构方案

## 一、当前问题总结

### 问题 1: ICRNL / ONLCR 的 `\n` 与 `\r\n` 混淆

**现状**：
- `renderChar()` 中 `'\n'` 直接执行 `putLF`（row++, col=0），即 `\n` 在渲染层被当作 `\r\n` 处理。
- `'\r'` 单独只设置 `col=0`。
- 输出侧 `termios` 配置了 `OPOST | ONLCR`（即 output 时 `\n` → `\r\n`），但 `puts`/`putcImpl` 的实现中没有任何 ONLCR 转换逻辑。实际的 `\n` → `\r\n` 行为完全由 `renderChar` 硬编码实现。
- 输入侧 `termios` 配置了 `ICRNL`（即 input 时 `\r` → `\n`），但 `Tty::input()` 中同样没有 ICRNL 转换逻辑。用户按回车产生 `\n`（查表结果），所以目前"碰巧"能工作，但语义不正确——如果未来查表改为 `\r`（更符合终端惯例），就会出问题。

**风险**：
- 无法通过 `tcsetattr` 关闭 ONLCR / ICRNL。
- 渲染层隐式假设了输出转换，输入层缺失显式转换。

### 问题 2: Backspace 键原来输出 `\b`

**现状**（已部分修复）：
- 用户已将 `SC_to_char` 查表中 backspace 对应的值从 `\b` 改为 `0x7f`。
- `termios.c_cc[VERASE]` = `0x7f`。
- 需要确认整条通路上 `0x7f` 被正确处理。

### 问题 3: Ctrl 组合键逻辑不一致（前序讨论已识别）

**现状**：
- `merge()` 不做 Ctrl 转换，只设置 `F_Ctrl` flag。
- `Tty::input()` 中 canonical 模式根本没有处理 `F_Ctrl`。
- Non-canonical 模式有 `msg.ch & 0x1F`，但 ISIG 信号检查（VINTR=0x03 等）在 canonical 分支内、且在 `F_Ctrl` 无关的路径上。
- **结果**：canonical 模式下 Ctrl+C 不会产生 SIGINT。

---

## 二、目标架构

```
键盘中断
  → keyboard::push()
    → keyboard::merge(scan)          [Layer 1: 扫描码 → Message]
      → keyboard::handle(msg)
        → console::handleInput(msg)
          → Tty::input(msg)          [Layer 2: 过滤 + 分派]
            → Tty::translateKey(msg)  [Layer 3: 功能键/Alt → 字节序列]
            → Tty::feedChar(c)       [Layer 4: 逐字节输入处理]
```

---

## 三、各层详细设计

### Layer 1: `keyboard::merge()` — `kernel/keyboard/message.cpp`

**职责**：将扫描码 + 当前修饰键状态合成为一个 `Message`，包含可直接使用的 `ch`。

#### 改动点

在现有查表逻辑之后、`return msg` 之前，添加 Ctrl 转换：

```cpp
// 在 msg.flag |= __flags; 之后，return msg; 之前
if ((msg.flag & F_Ctrl) && msg.ch) {
    // Ctrl 转换：只对字母和部分符号生效
    // 标准行为：Ctrl + [a-z] → 0x01-0x1A
    //           Ctrl + [@[\]^_] → 0x00, 0x1B-0x1F
    // 统一用 & 0x1F 处理（对字母和 @[\]^_ 等均正确）
    char raw = msg.ch;
    if (raw >= 'a' && raw <= 'z') {
        msg.ch = raw & 0x1F;
    } else if (raw >= '@' && raw <= '_') {
        msg.ch = raw & 0x1F;
    } else if (raw == '?') {
        msg.ch = 0x7F;  // Ctrl+? → DEL
    }
    // 其他字符（数字、标点等）：Ctrl 无效果，保持原 ch
}
```

> **注意**：Shift 查表已经在 `SC_to_char[(scan << 1) | shift]` 中处理。
> Ctrl 转换在 Shift 之后执行，这意味着 Ctrl+Shift+A 的 ch 是 `'A' & 0x1F = 0x01`，与 Ctrl+A 相同。这是标准行为。

**不改动的部分**：
- `msg.flag` 仍然包含 `F_Ctrl` 标志（下游可能需要）。
- `msg.code` / `msg.key` 不变。
- 对于功能键（方向键等），`ch` 已经是 0，Ctrl 转换不会触发。

---

### Layer 2: `Tty::input()` — `kernel/console/tty.cpp`

**职责**：接收 `Message`，过滤修饰键本身，然后分派到 `translateKey` + `feedChar`。

#### 完整重写

```cpp
void Tty::input(const keyboard::Message& msg) noexcept {
    using namespace keyboard;

    // 1. 忽略释放事件
    if (msg.flag & F_Release) return;

    // 2. 忽略纯修饰键按下（Ctrl/Shift/Alt/CapsLock/NumLock/ScrollLock）
    switch (msg.key) {
        case SC_LCTRL: case SC_RCTRL:
        case SC_LSHIFT: case SC_RSHIFT:
        case SC_LALT: case SC_RALT:
        case SC_CAPLOCKS:
        case SC_NUMLOCK:
        case SC_SCROLLLOCK:
            return;
    }

    arch::InterruptGuard guard;

    // 3. 将 Message 翻译为字节序列
    char buf[8];
    int len = translateKey(msg, buf);

    // 4. 逐字节送入 feedChar
    for (int i = 0; i < len; i++) {
        feedChar(buf[i]);
    }

    // 5. 唤醒等待读取的进程
    if (len > 0) {
        __wait_list.wakeOne(task::WakeReason::WR_Normal);
    }
}
```

---

### Layer 3: `Tty::translateKey()` — `kernel/console/tty.cpp` (新增私有方法)

**职责**：将一个按键 Message 转为 0~N 个字节。不做任何输入处理逻辑。

```cpp
// 返回写入 buf 的字节数
int Tty::translateKey(const keyboard::Message& msg, char* buf) noexcept {
    using namespace keyboard;

    // --- 功能键 → ANSI 转义序列 ---
    switch (msg.code) {
        case SC_UP:       buf[0]='\033'; buf[1]='['; buf[2]='A'; return 3;
        case SC_DOWN:     buf[0]='\033'; buf[1]='['; buf[2]='B'; return 3;
        case SC_RIGHT:    buf[0]='\033'; buf[1]='['; buf[2]='C'; return 3;
        case SC_LEFT:     buf[0]='\033'; buf[1]='['; buf[2]='D'; return 3;
        case SC_HOME:     buf[0]='\033'; buf[1]='['; buf[2]='H'; return 3;
        case SC_END:      buf[0]='\033'; buf[1]='['; buf[2]='F'; return 3;
        case SC_DELETE:   buf[0]='\033'; buf[1]='['; buf[2]='3'; buf[3]='~'; return 4;
        case SC_INSERT:   buf[0]='\033'; buf[1]='['; buf[2]='2'; buf[3]='~'; return 4;
        case SC_PAGEUP:   buf[0]='\033'; buf[1]='['; buf[2]='5'; buf[3]='~'; return 4;
        case SC_PAGEDOWN: buf[0]='\033'; buf[1]='['; buf[2]='6'; buf[3]='~'; return 4;
        // F1-F4
        case SC_F1:       buf[0]='\033'; buf[1]='O'; buf[2]='P'; return 3;
        case SC_F2:       buf[0]='\033'; buf[1]='O'; buf[2]='Q'; return 3;
        case SC_F3:       buf[0]='\033'; buf[1]='O'; buf[2]='R'; return 3;
        case SC_F4:       buf[0]='\033'; buf[1]='O'; buf[2]='S'; return 3;
    }

    // --- 无字符的键（ESC/F5-F12 等暂不处理）→ 忽略 ---
    if (!msg.ch) {
        return 0;
    }

    // --- Alt+字符 → ESC 前缀 ---
    if (msg.flag & F_Alt) {
        buf[0] = '\033';
        buf[1] = msg.ch;
        return 2;
    }

    // --- 普通字符 / Ctrl 字符（ch 已经由 merge 转换好）---
    buf[0] = msg.ch;
    return 1;
}
```

---

### Layer 4: `Tty::feedChar()` — `kernel/console/tty.cpp` (新增私有方法)

**职责**：对单个字节执行输入处理（ICRNL、ISIG、canonical 编辑、echo、推入缓冲区）。

```cpp
void Tty::feedChar(char c) noexcept {
    // ─── 输入预处理 ───
    if ((__config.c_iflag & ICRNL) && c == '\r') {
        c = '\n';
    }

    // ─── ISIG：信号检查（canonical 和 non-canonical 均生效）───
    if (__config.c_lflag & ISIG) {
        if (c == __config.c_cc[VINTR]) {     // 0x03 (Ctrl+C)
            echoCtrl(c);
            __line_buffer.clear();
            task::kill(-__foreground_pgid, SIGINT) | __ignore;
            return;
        }
        if (c == __config.c_cc[VSUSP]) {     // 0x1A (Ctrl+Z)
            echoCtrl(c);
            task::kill(-__foreground_pgid, SIGTSTP) | __ignore;
            return;
        }
        if (c == __config.c_cc[VQUIT]) {     // 0x1C (Ctrl+\)
            echoCtrl(c);
            __line_buffer.clear();
            task::kill(-__foreground_pgid, SIGQUIT) | __ignore;
            return;
        }
    }

    // ─── Canonical 模式 ───
    if (__config.c_lflag & ICANON) {
        if (c == __config.c_cc[VERASE]) {     // 0x7F (Backspace/DEL)
            if (!__line_buffer.empty()) {
                __line_buffer.pop_back();
                if (__config.c_lflag & ECHO) {
                    putcImpl('\b');
                    putcImpl(' ');
                    putc('\b');
                }
            }
            return;
        }

        if (c == __config.c_cc[VEOF]) {       // 0x04 (Ctrl+D)
            // 将行缓冲区内容推入 input_buffer，不附加任何字符
            __input_buffer.append(__line_buffer);
            __line_buffer.clear();
            __pending_eof = true;
            return;
        }

        if (c == '\n') {
            __line_buffer.push_back('\n');
            __input_buffer.append(__line_buffer);
            __line_buffer.clear();
            if (__config.c_lflag & (ECHO | ECHONL)) {
                putc('\n');   // 输出侧走 ONLCR（见 Layer 5）
            }
            return;
        }

        // 普通字符 → 行缓冲
        __line_buffer.push_back(c);
        if (__config.c_lflag & ECHO) {
            echoChar(c);
        }
        return;
    }

    // ─── Non-canonical 模式 ───
    __input_buffer.push_back(c);
    if (__config.c_lflag & ECHO) {
        echoChar(c);
    }
}
```

#### Echo 辅助方法

```cpp
void Tty::echoChar(char c) noexcept {
    if (static_cast<unsigned char>(c) < 0x20 && c != '\n' && c != '\t') {
        // 控制字符回显为 ^X 形式
        putcImpl('^');
        putc(c + '@');
    } else {
        putc(c);
    }
}

void Tty::echoCtrl(char c) noexcept {
    // 信号字符的回显：^C, ^Z, ^\, 并换行
    if (__config.c_lflag & ECHO) {
        putcImpl('^');
        putc(c + '@');
        putc('\n');
    }
}
```

---

### Layer 5: 输出侧 ONLCR — `kernel/console/buffer.cpp`

**现状问题**：
- `renderChar('\n')` 硬编码执行 `\r\n` 效果（col=0, row++）。
- `termios.c_oflag` 中的 `OPOST | ONLCR` 标志未被检查。
- 这意味着即使关闭 ONLCR，`\n` 仍然会回车——无法真正控制输出处理。

**方案**：将 ONLCR 处理从 `renderChar` 上移到 `putcImpl` 中（或新增一个输出预处理层），使 `renderChar` 成为纯渲染原语。

#### 改动 1: `renderChar` — 分离 `\n` 和 `\r`

将 `renderChar` 中的 `'\n'` 行为改为**纯 LF**（仅 row++，不设 col=0）：

```cpp
void ScreenBuffer::renderChar(char ch) noexcept {
    if (ch == '\n') {
        // 纯 LF：仅换行，不回车
        if (++__row_ptr == __height) {
            scroll(1);
            __row_ptr = __height - 1;
            flushBuffer();
        }
        return;
    } else if (ch == '\r') {
        __col_ptr = 0;
        return;
    } else if (ch == '\b') {
        // ... 不变
    }
    // ... 其余不变
}
```

#### 改动 2: `putcImpl` — 添加 ONLCR 转换

在 VT 状态机的 Idle 状态中，进入 `renderChar` 之前插入输出处理：

```cpp
void ScreenBuffer::putcImpl(char ch) noexcept {
    switch (__vtstate) {
        case VTS_Idle:
            if (ch == '\e') {
                __vtstate = VTS_Esc;
            } else {
                // ─── 输出后处理 (OPOST) ───
                if (__config.c_oflag & OPOST) {
                    if ((__config.c_oflag & ONLCR) && ch == '\n') {
                        renderChar('\r');  // ONLCR: \n → \r\n
                    }
                }
                renderChar(ch);
            }
            break;
        // ... VTS_Esc, VTS_EscBrc 不变
    }
}
```

> **效果**：
> - 默认（OPOST | ONLCR 开启）：`putcImpl('\n')` → `renderChar('\r')` + `renderChar('\n')` = `\r\n` 行为，与之前相同。
> - 关闭 ONLCR：`putcImpl('\n')` → 仅 `renderChar('\n')` = 纯换行不回车。
> - 单独输出 `'\r'`：不受影响。

---

### `tty.hpp` 声明更新

```cpp
struct Tty : public ScreenBuffer {
    pid_t __foreground_pgid{task::KP_Invalid};

    std::string __line_buffer;
    std::string __input_buffer;
    task::WaitList __wait_list;

    bool __pending_eof{};

    void activate() noexcept;
    void deactivate() noexcept;

    void input(const keyboard::Message& msg) noexcept;

    bool inputEmpty() noexcept;
    Result<arch::InterruptGuard> syncWaitInput() noexcept;

private:
    int translateKey(const keyboard::Message& msg, char* buf) noexcept;
    void feedChar(char c) noexcept;
    void echoChar(char c) noexcept;
    void echoCtrl(char c) noexcept;
};
```

---

## 四、查表修正确认

### `SC_to_char` 的 Backspace 位置

```
SC_BACKSPACE = 0x0E
```

查表数组第一行对应 SC 0x01 起：

```
"\0\0"          SC_ESCAPE (0x01)
"1!2@3#4$5%6^"  SC_1 ~ SC_6
"7&8*9(0)"      SC_7 ~ SC_0
"-_=+"          SC_MINUS, SC_EQUAL
"\x7f\x7f"      SC_BACKSPACE (0x0E)  ← 用户已改为 0x7F ✓
```

用户已将此处从 `\b\b` 改为 `\x7f\x7f`，两个值分别对应无 Shift 和有 Shift 的情况。**确认正确**。

> 验证路径：`merge()` 查表得到 `ch = 0x7F` → `translateKey()` 直接返回 `0x7F` → `feedChar(0x7F)` 匹配 `c_cc[VERASE]` → 执行行缓冲删除。✓

### 回车键

```
SC_ENTER = 0x1C
```

查表数组中 SC_ENTER 对应 `\n\n`。

> **建议**：考虑将其改为 `\r\r`，这更符合终端硬件惯例（硬件终端发送 CR）。改为 `\r` 后，由 `feedChar` 中的 ICRNL 将其转换为 `\n`。这使得 ICRNL 开关真正有效。
>
> **但这不是必须的**：如果暂时不改查表，目前 `\n` 也能工作，只是 ICRNL 开关对回车键无效（因为键盘直接产生 `\n` 而非 `\r`）。如果以后需要严格的 termios 语义，再改即可。

---

## 五、文件修改清单

| 文件 | 改动类型 | 说明 |
|------|----------|------|
| `kernel/keyboard/message.cpp` | 修改 `merge()` | 在 `msg.flag \|= __flags` 之后添加 Ctrl ch 转换 |
| `kernel/console/tty.hpp` | 修改 `Tty` 结构体 | 添加 `translateKey`, `feedChar`, `echoChar`, `echoCtrl` 私有声明 |
| `kernel/console/tty.cpp` | **重写** `input()` | 改为过滤 + translateKey + feedChar 流程 |
| `kernel/console/tty.cpp` | **新增** `translateKey()` | 功能键转义序列 + Alt 前缀 + 普通字符直通 |
| `kernel/console/tty.cpp` | **新增** `feedChar()` | ICRNL + ISIG + canonical/non-canonical 处理 |
| `kernel/console/tty.cpp` | **新增** `echoChar()` / `echoCtrl()` | 控制字符回显辅助 |
| `kernel/console/buffer.cpp` | 修改 `renderChar()` | `'\n'` 改为纯 LF（不再隐式 CR） |
| `kernel/console/buffer.cpp` | 修改 `putcImpl()` | 在 VTS_Idle 中添加 OPOST/ONLCR 检查 |
| `kernel/keyboard/message.cpp` | *(已完成)* | `SC_to_char` backspace 位置已改为 `\x7f` |

---

## 六、数据流验证

### Case 1: 用户按下 `a`（canonical + ECHO）

```
scan=0x1E → merge: ch='a', flag=0
  → input: 非修饰键，translateKey → buf=['a'], len=1
    → feedChar('a'):
       ICRNL: 'a'≠'\r' → skip
       ISIG: 'a'≠0x03/0x1A/0x1C → skip
       ICANON: 'a'≠VERASE/VEOF/'\n' → __line_buffer.push_back('a')
       ECHO: echoChar('a') → putc('a')
```

### Case 2: 用户按下 `Ctrl+C`（canonical + ISIG）

```
scan=0x2E → merge: ch='c', flag=F_Ctrl → ch = 'c' & 0x1F = 0x03
  → input: 非修饰键，translateKey → buf=[0x03], len=1
    → feedChar(0x03):
       ICRNL: 0x03≠'\r' → skip
       ISIG: 0x03 == c_cc[VINTR] → echoCtrl(^C), clear line, kill(SIGINT), return
```

### Case 3: 用户按下回车（canonical + ECHO）

```
scan=0x1C → merge: ch='\n', flag=0
  → input: translateKey → buf=['\n'], len=1
    → feedChar('\n'):
       ICRNL: '\n'≠'\r' → skip（如果查表改为'\r'则会触发）
       ISIG: '\n'≠信号字符 → skip
       ICANON: '\n' → __line_buffer.push_back('\n'), flush to input_buffer
       ECHO|ECHONL: putc('\n')
         → putcImpl('\n') → ONLCR → renderChar('\r') + renderChar('\n')
```

### Case 4: 用户按下 Backspace（canonical）

```
scan=0x0E → merge: ch=0x7F, flag=0
  → input: translateKey → buf=[0x7F], len=1
    → feedChar(0x7F):
       ICRNL: 0x7F≠'\r' → skip
       ISIG: 0x7F≠信号字符 → skip
       ICANON: 0x7F == c_cc[VERASE] → pop_back + echo(\b, ' ', \b)
```

### Case 5: 用户按下方向键↑（non-canonical）

```
scan=0xE048 → merge: ch=0, flag=0
  → input: 非修饰键，translateKey → SC_UP → buf=['\033','[','A'], len=3
    → feedChar('\033'): non-canon → push to input_buffer
    → feedChar('['): non-canon → push to input_buffer
    → feedChar('A'): non-canon → push to input_buffer
```

### Case 6: 用户按下方向键↑（canonical）— 编辑功能 TODO

```
scan=0xE048 → merge: ch=0, flag=0
  → input: translateKey → SC_UP → buf=['\033','[','A'], len=3
    → feedChar('\033'): ICANON, 不匹配特殊字符 → push to line_buffer
    (后续字节类似)
```

> **注意**：canonical 模式下方向键目前不做行编辑处理（是 TODO），转义序列会被当作普通字符缓冲。
> 如果需要在 canonical 模式下拦截方向键用于行编辑，可以在 `input()` 的 Layer 2 中对 canonical 模式下的特定 `msg.code` 做前置拦截，跳过 translateKey/feedChar 流程。这是后续工作。

### Case 7: 用户按下 `Ctrl+D`（non-canonical + ISIG）

```
scan=0x20 → merge: ch='d', flag=F_Ctrl → ch = 0x04
  → input: translateKey → buf=[0x04], len=1
    → feedChar(0x04):
       ISIG: 0x04≠VINTR/VSUSP/VQUIT → skip
       non-ICANON → push 0x04 to input_buffer
```

> Non-canonical 模式下 Ctrl+D 不触发 EOF，直接作为字节传递给应用。这是标准行为。

---

## 七、未来工作 (不在本次重构范围)

1. **Canonical 模式行编辑**：方向键、Home/End 在 canonical 模式下的行缓冲光标移动。
2. **VKILL**（0x15, Ctrl+U）：清除整行。
3. **ONOCR / OCRNL / ONLRET** 等其他输出标志。
4. **ISTRIP / IUCLC / IXON / IXOFF** 等其他输入标志。
5. **Tab 补全**。
6. **回车键查表改为 `\r`** 以使 ICRNL 完全正确。
