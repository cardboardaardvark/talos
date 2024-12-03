#pragma once

#include <cstdint>

namespace cpu
{

namespace x86
{

#define UNWAIT_INTERRUPT 50

typedef struct
{
    // the order here is defined by the stubs in idt.asm
    std::uint32_t gs, fs, es, ds, cr3;
    std::uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    std::uint32_t number, error;
    std::uint32_t eip, cs, eflags, useresp, ss;
} interrupt_info_t;

typedef struct __attribute__((packed))
{
    std::uint16_t    isr_low;      // The lower 16 bits of the ISR's address
    std::uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
    std::uint8_t     reserved;     // Set to zero
    std::uint8_t     attributes;   // Type and attributes; see the IDT page
    std::uint16_t    isr_high;     // The higher 16 bits of the ISR's address
} idt_entry_t;

typedef struct __attribute__((packed)) {
    std::uint16_t limit;
    idt_entry_t *base;
} idt_table_t;

typedef void (*idt_stub_t)();
typedef void (*irq_handler_t)(uint8_t);

extern "C"
{
    extern idt_stub_t idt_stub_table[];

    void handle_interrupt(const interrupt_info_t *info) noexcept;
}

void init_idt() noexcept;

} // namespace x86

} // namespace cpu
