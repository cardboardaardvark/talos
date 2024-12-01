#include <atomic>
#include <cstddef>
#include <cstdint>

#include <driver/timer/pit.hpp>
#include <hal/interrupt.hpp>
#include <libk/assert.hpp>
#include <libk/error.hpp>
#include <libk/logging.hpp>
#include <libk/util.hpp>

#include "idt.hpp"
#include "io.hpp"
#include "memory.hpp"
#include "paging.hpp"

#define IDT_SIZE 256

#define EXCEPTION_MAX_INTERRUPT_NUM 31
#define IRQ_MAX_INTERRUPT_NUM 47

#define NUM_IRQS 16
#define PIT_IRQ 0

#define PIC1_PORT 0x20 /* IO base address for master PIC */
#define PIC2_PORT 0xA0 /* IO base address for slave PIC */
#define PIC_CMD_EOI 0x20 // End of Interrupt

#define EXCEPTION_TYPE_GPF 14

namespace cpu
{

namespace x86
{

__attribute__((aligned(0x10))) static idt_entry_t idt_entries[IDT_SIZE];
static idt_table_t idt_table;
static irq_handler_t irq_handlers[NUM_IRQS];
static bool irq_did_warn[NUM_IRQS];
static hal::tick_handler_t tick_handler = nullptr;
static std::atomic<bool> inside_isr = ATOMIC_VAR_INIT(false);

static void handle_pit_irq(uint8_t irq_number) noexcept;

#ifndef NDEBUG
static void set_inside_isr(bool new_state)
{
    auto previous = inside_isr.exchange(new_state, std::memory_order_release);

    // FIXME This is too sensitive, for instance, it's ok to have a GPF happen
    // while inside a timer handler.
    assert(previous != new_state);
}
#endif

static void set_idt_descriptor(uint8_t number, idt_stub_t isr, uint8_t flags) noexcept {
    assert(number < IDT_SIZE);

    idt_entry_t* entry = &idt_entries[number];

    entry->isr_low = (uint32_t)isr & 0xFFFF;
    entry->kernel_cs = 0x08; // this value can be whatever offset your kernel code selector is in your GDT
    entry->attributes = flags;
    entry->isr_high = (uint32_t)isr >> 16;
    entry->reserved = 0;
}

static void set_irq_handler(uint8_t number, irq_handler_t handler) noexcept
{
    assert(number < NUM_IRQS);

    irq_handlers[number] = handler;
}

void init_idt() noexcept {
    // Ensure that indexes into the idt_handlers and irq_handlers arrays
    // will always fit into an uint8_t
    assert(IDT_SIZE <= 256);
    assert(NUM_IRQS <= 256);

    idt_table.base = &idt_entries[0];
    idt_table.limit = (uint16_t)sizeof(idt_entry_t) * IDT_SIZE - 1;

    for (size_t vector = 0; vector < IDT_SIZE; vector++) {
        set_idt_descriptor(vector, idt_stub_table[vector], 0x8E);
    }

    for(size_t irq = 0; irq < NUM_IRQS; irq++) {
        irq_handlers[irq] = nullptr;
        irq_did_warn[irq] = false;
    }

    set_irq_handler(PIT_IRQ, handle_pit_irq);

    // IRQ0 to IRQ15 map to IDT entries 32 to 47
    outb(PIC1_PORT, 0x11);
    x86::outb(PIC2_PORT, 0x11);
    x86::outb(0x21, 0x20);
    x86::outb(0xA1, 0x28);
    x86::outb(0x21, 0x04);
    x86::outb(0xA1, 0x02);
    x86::outb(0x21, 0x01);
    x86::outb(0xA1, 0x01);
    x86::outb(0x21, 0x0);
    x86::outb(0xA1, 0x0);

    // Tell the CPU to load the IDT table
    __asm__ volatile ("lidt %0" : : "m"(idt_table));
}

// Intentionally use uint32_t so if this function is called with a really
// bogus interrupt number it does not wrap the integer and return a valid
// description.
const char* idt_interrupt_description(uint32_t interrupt_number) noexcept
{
    static const char* descriptions[] = {
        "Division By Zero Exception",
        "Debug Exception",
        "NMI Exception",
        "Breakpoint Exception",
        "Into Detected Overflow Exception",
        "Out of Bounds Exception",
        "Invalid Opcode Exception",
        "No Coprocessor Exception",
        "Double Fault Exception",
        "Coprocessor Segment Overrun Exception",
        "Bad TSS Exception",
        "Segment Not Present Exception",
        "Stack Fault Exception",
        "General Protection Fault Exception",
        "Page Fault Exception",
        "Unknown Interrupt Exception",
        "Coprocessor Fault Exception",
        "Alignment Check Exception",
        "Machine Check Exception",
    };

    if (interrupt_number <= 18) {
       return descriptions[interrupt_number];
    } else if (interrupt_number <= EXCEPTION_MAX_INTERRUPT_NUM) {
        return "Reserved Exception";
    }

    return "!!invalid exception value!!";
}

// Interrupts are disabled when this function is called from idt.asm
void handle_interrupt(interrupt_info_t *info) noexcept
{
    assert(info->number < IDT_SIZE);

#ifndef NDEBUG
    set_inside_isr(true);
    libk::Finally inside_isr_guard([] { set_inside_isr(false); });
#endif

    uint8_t number = info->number;

    if (number <= EXCEPTION_MAX_INTERRUPT_NUM) {
        const char *description = idt_interrupt_description(info->number);

        if (info->number == EXCEPTION_TYPE_GPF) {
            libk::printf("Address that caused GPF: 0x%p\n", gpf_address());
        }

        libk::panic("Got an exception: #%u %s; eip=0x%p error=%u\n", info->number, description, info->eip, info->error);
    } else if (number <= IRQ_MAX_INTERRUPT_NUM) {
        unsigned int irq_number = info->number - (EXCEPTION_MAX_INTERRUPT_NUM + 1);

        assert(irq_number < NUM_IRQS);

        irq_handler_t handler = irq_handlers[irq_number];

        if (handler != nullptr) {
            handler(irq_number);
        } else if (! irq_did_warn[irq_number]) {
            libk::printf("Warning: unhandled IRQ #%u\n", irq_number);
            irq_did_warn[irq_number] = true;
        }

        if (irq_number > 8) {
            x86::outb(PIC2_PORT, PIC_CMD_EOI);
        }

        x86::outb(PIC1_PORT, PIC_CMD_EOI);
    } else if (number == UNWAIT_INTERRUPT) {
        // Nothing to do, the kernel will wakeup in it's mainloop
        // after the ISR completes.
    } else {
        libk::panic("Got an unknown interrupt: #%u\n", number);
    }
}

static void handle_pit_irq(uint8_t) noexcept
{
    if (tick_handler == nullptr) {
        libk::panic("Got PIT IRQ but no tick handler is installed\n");
    }

    tick_handler();
}

} // namespace x86

} // namespace cpu

namespace hal
{

using namespace cpu::x86;
using namespace driver::timer;

void set_tick_handler(tick_handler_t handler) noexcept
{
    assert(tick_handler == nullptr);

    tick_handler = handler;
}

void set_tick_frequency(size_t hz) noexcept
{
    assert(tick_handler != nullptr);

    uint16_t divider = pit_clock_freq / hz;

    outb(pit_port_command, pit_select_channel_0 | pit_access_lohi | pit_mode_rg | pit_divisor_16bit);
    // first write the low bits
    outb(pit_port_channel_0, divider & 0xFF);
    // then the high bits
    outb(pit_port_channel_0, divider >> 8);
}

#ifndef NDEBUG
bool inside_isr() noexcept
{
    return cpu::x86::inside_isr.load(std::memory_order_consume);
}
#endif

} // namespace hal
