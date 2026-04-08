#include "load.hpp"

#include "../arch/io.hpp"
#include "entry.hpp"
#include "isr.hpp"

#pragma clang diagnostic ignored "-Wunused-const-variable"

namespace nyan::interrupt {

constexpr uint16_t PIC1 = 0x20;
constexpr uint16_t PIC2 = 0xA0;
constexpr uint16_t PIC1_COMMAND = PIC1;
constexpr uint16_t PIC1_DATA = (PIC1 + 1);
constexpr uint16_t PIC2_COMMAND = PIC2;
constexpr uint16_t PIC2_DATA = (PIC2 + 1);

constexpr uint16_t PIC1_ICW1 = 0x0020;
constexpr uint16_t PIC1_OCW2 = 0x0020;
constexpr uint16_t PIC1_IMR = 0x0021;
constexpr uint16_t PIC1_ICW2 = 0x0021;
constexpr uint16_t PIC1_ICW3 = 0x0021;
constexpr uint16_t PIC1_ICW4 = 0x0021;
constexpr uint16_t PIC2_ICW1 = 0x00a0;
constexpr uint16_t PIC2_OCW2 = 0x00a0;
constexpr uint16_t PIC2_IMR = 0x00a1;
constexpr uint16_t PIC2_ICW2 = 0x00a1;
constexpr uint16_t PIC2_ICW3 = 0x00a1;
constexpr uint16_t PIC2_ICW4 = 0x00a1;

constexpr uint16_t ICW1_ICW4 = 0x01;
constexpr uint16_t ICW1_SINGLE = 0x02;
constexpr uint16_t ICW1_INTERVAL4 = 0x04;
constexpr uint16_t ICW1_LEVEL = 0x08;
constexpr uint16_t ICW1_INIT = 0x10;

constexpr uint16_t ICW4_8086 = 0x01;
constexpr uint16_t ICW4_AUTO = 0x02;
constexpr uint16_t ICW4_BUF_SLAVE = 0x08;
constexpr uint16_t ICW4_BUF_MASTER = 0x0C;
constexpr uint16_t ICW4_SFNM = 0x10;

constexpr uint16_t PIC_READ_IRR = 0x0a;
constexpr uint16_t PIC_READ_ISR = 0x0b;

alignas(16) static Entry entries[256];

static Descriptor desc;

static void init_pic() {
    arch::outb(PIC1_IMR, 0xff);  // disable interrupts
    arch::outb(PIC2_IMR, 0xff);

    arch::outb(PIC1_ICW1, 0x11);    // edge
    arch::outb(PIC1_ICW2, 0x20);    // remap 0 ~ 7 to 0x20 ~ 0x27
    arch::outb(PIC1_ICW3, 1 << 2);  // IRQ2
    arch::outb(PIC1_ICW4, 0x01);    // no buffer

    arch::outb(PIC2_ICW1, 0x11);  // edge
    arch::outb(PIC2_ICW2, 0x28);  // remap 8 ~ 15 to 0x28 ~ 0x2f
    arch::outb(PIC2_ICW3, 2);     // IRQ2
    arch::outb(PIC2_ICW4, 0x01);  // no buffer
}

void load() {
    init_pic();

    desc.size = sizeof(entries) - 1;
    desc.offset = reinterpret_cast<uint32_t>(entries);

    fillEntries(entries);

    asm volatile("lidt %0" ::"m"(desc) : "memory");
}

void mask(uint8_t irq) {
    uint16_t port;
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    arch::outb(port, arch::inb(port) | (1 << irq));
}

void unmask(uint8_t irq) {
    uint16_t port;
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    arch::outb(port, arch::inb(port) & ~(1 << irq));
}

void end(uint8_t irq) {
    if (irq >= 8) {
        arch::outb(PIC2_COMMAND, 0x20);
    }
    arch::outb(PIC1_COMMAND, 0x20);
}

}  // namespace nyan::interrupt
