#include <cstdint>
#include <cstring>

#include <driver/video/vga/vga.hpp>
#include <libk/memory.hpp>
#include <libk/mutex.hpp>
#include <libk/string.hpp>
#include <libk/util.hpp>
#include <kernel/main.hpp>

#include "console.hpp"

#define CONSOLE_BG_COLOR vga::Color::black
#define CONSOLE_FG_COLOR vga::Color::light_grey
#define STATUS_BG_COLOR vga::Color::light_grey
#define STATUS_FG_COLOR vga::Color::black

#define STATUS_ROW 0
#define CONSOLE_FIRST_ROW 1
#define CONSOLE_HEIGHT VGA_HEIGHT - CONSOLE_FIRST_ROW

#define STATUS_CLOCK_WIDTH 8
#define STATUS_MEMORY_WIDTH VGA_WIDTH - STATUS_CLOCK_WIDTH - 1

namespace vga = driver::video::vga;

namespace platform
{

namespace ibmpc
{

libk::isr_spin_mutex_t vga_mutex;
bool console_initialized = false;
std::uint8_t console_line = 0;
std::uint8_t console_column = 0;
const vga::color_t console_color = vga::color(CONSOLE_FG_COLOR, CONSOLE_BG_COLOR);
const vga::color_t status_color = vga::color(STATUS_FG_COLOR, STATUS_BG_COLOR);

// RESTRICT No assert()
// RESTRICT No dynamic memory
// REQUIRE LOCKED vga_mutex
static void _clear_status() noexcept
{
    const auto status_blank = vga::character(' ', status_color);

    for (unsigned int column = 0; column < VGA_WIDTH; column++) {
        vga::place_character(status_blank, STATUS_ROW, column);
    }
}

// RESTRICT No assert()
// RESTRICT No dynamic memory
// REQUIRE LOCKED vga_mutex
static void _clear_console() noexcept
{
    const auto console_blank = vga::character(' ', console_color);

    for (unsigned int row = CONSOLE_FIRST_ROW; row < VGA_HEIGHT; row++) {
        for (unsigned int column = 0; column < VGA_WIDTH; column++) {
            vga::place_character(console_blank, row, column);
        }
    }

    vga::place_cursor(CONSOLE_FIRST_ROW, 0);
}

// RESTRICT No assert()
// RESTRICT No dynamic memory
// REQUIRE LOCKED vga_mutex
static void _clear_screen() noexcept
{
    _clear_status();
    _clear_console();
}

// RESTRICT No assert()
// RESTRICT No dynamic memory
void init_console() noexcept
{
        // Mutex is shared with ISRs
        libk::DisableInterrupts interrupt_guard;
        libk::SpinLock lock(vga_mutex);

        _clear_screen();
        console_initialized = true;
}

void update_status_clock() noexcept
{
    auto uptime = kernel::uptime();
    unsigned int minutes = uptime / 60;
    unsigned int seconds = uptime % 60;
    unsigned int hours = minutes / 60;

    minutes -= hours * 60;

    auto clock_face = libk::formatted_string("{:#02}:{:#02}:{:#02}", hours, minutes, seconds);
    auto column_start = VGA_WIDTH - clock_face.length();

    {
        // Mutex is shared with ISRs
        libk::DisableInterrupts interrupt_guard;
        libk::SpinLock lock(vga_mutex);

        vga::place_string(clock_face.c_str(), status_color, STATUS_ROW, column_start);
    }
}

void update_status_memory() noexcept
{
    auto memory_status = libk::memory_status();
    auto available_kb = memory_status.physical.available_pages * PAGE_SIZE / 1024;
    auto total_kb = memory_status.physical.total_pages * PAGE_SIZE / 1024;
    auto heap_kb = memory_status.paged.heap_size / 1024;
    auto content = libk::formatted_string("{} KiB / {} KiB / {} KiB", heap_kb, available_kb, total_kb);

    {
        // Mutex is shared with ISRs
        libk::DisableInterrupts interrupt_guard;
        libk::SpinLock lock(vga_mutex);

        vga::place_string(content.c_str(), status_color, STATUS_ROW, 0);

        for (uint8_t i = content.length(); i < STATUS_MEMORY_WIDTH; i++) {
            vga::place_character(vga::character(' ', status_color), STATUS_ROW, i);
        }
    }
}

// RESTRICT No assert()
// RESTRICT No dynamic memory
void console_put_char(char c) noexcept
{
    // Mutex is shared with ISRs
    libk::DisableInterrupts interrupt_guard;
    libk::SpinLock console_lock(platform::ibmpc::vga_mutex);

    if (! platform::ibmpc::console_initialized) return;

    if (c == '\n') {
        platform::ibmpc::console_line++;
        platform::ibmpc::console_column = 0;
    } else if (c == '\r') {
        platform::ibmpc::console_column = 0;
    } else if (c == '\t') {
        platform::ibmpc::console_column = (platform::ibmpc::console_column + 8) & ~(8 - 1);
    } else {
        vga::place_character(vga::character(c, platform::ibmpc::console_color), CONSOLE_FIRST_ROW + platform::ibmpc::console_line, platform::ibmpc::console_column);
        platform::ibmpc::console_column++;
    }

    if (platform::ibmpc::console_column > VGA_WIDTH) {
        platform::ibmpc::console_column = 0;
        platform::ibmpc::console_line++;
    }

    if (platform::ibmpc::console_line >= CONSOLE_HEIGHT) {
        vga::character_t blank = vga::character(' ', platform::ibmpc::console_color);

        vga::scroll(CONSOLE_FIRST_ROW, CONSOLE_HEIGHT);
        platform::ibmpc::console_line = CONSOLE_HEIGHT - 1;

        for (std::uint8_t column = 0; column < VGA_WIDTH; column++) {
            vga::place_character(blank, CONSOLE_FIRST_ROW + platform::ibmpc::console_line, column);
        }
    }

    vga::place_cursor(CONSOLE_FIRST_ROW + platform::ibmpc::console_line, platform::ibmpc::console_column);
}

} // namespace ibmpc

} // namespace platform
