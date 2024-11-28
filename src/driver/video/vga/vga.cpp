#include <cstddef>
#include <cstring>

#include <cpu/x86/io.hpp>

#include "vga.hpp"

#define VGA_BUFFER_ADDRESS 0xB8000
#define VGA_COMMAND_PORT 0x3D4
#define VGA_DATA_PORT 0x3D5

#define VGA_CURSOR_POSITION_HIGH 14
#define VGA_CUSOR_POSITION_LOW 15

namespace driver
{

namespace video
{

namespace vga
{

namespace x86 = cpu::x86;

character_t* buffer = reinterpret_cast<character_t*>(VGA_BUFFER_ADDRESS);

// RESTRICT no assert()
// RESTRICT no dynamic memory
color_t color(Color fg, Color bg) noexcept
{
    return static_cast<color_t>(fg) | static_cast<color_t>(bg) << 4;
}

// RESTRICT No assert()
// RESTRICT No dynamic memory
character_t character(char c, color_t colors) noexcept
{
    return static_cast<character_t>(c) | static_cast<character_t>(colors) << 8;
}

// RESTRICT No assert()
// RESTRICT No dynamic memory
character_t character(char c, Color fg, Color bg) noexcept
{
    return character(c, color(fg, bg));
}

// RESTRICT no assert()
// RESTRICT no dynamic memory
void place_character(character_t character, std::uint8_t row, std::uint8_t column) noexcept
{
    buffer[row * screen_width + column] = character;
}

// RESTRICT no assert()
// RESTRICT no dynamic memory
void place_string(const char* string, color_t color, std::uint8_t row, std::uint8_t column) noexcept
{
    for (std::uint32_t i = 0; string[i] != '\0'; i++) {
        place_character(character(string[i], color), row, column + i);
    }
}

// RESTRICT No assert()
// RESTRICT No dynamic memory
void place_cursor(std::uint8_t row, std::uint8_t column) noexcept
{
    std::uint16_t position = row * screen_width + column;

    x86::outb(VGA_COMMAND_PORT, VGA_CURSOR_POSITION_HIGH);
    x86::outb(VGA_DATA_PORT, position >> 8);
    x86::outb(VGA_COMMAND_PORT, VGA_CUSOR_POSITION_LOW);
    x86::outb(VGA_DATA_PORT, position);
}

// Move the contents of the VGA buffer from start_row to start_row + num_rows (inclusive)
// up a single row. Does not clear the last line.
// RESTRICT No assert()
// RESTRICT No dynamic memory
void scroll(std::uint8_t start_row, std::uint8_t num_rows) noexcept
{
    character_t* start_row_addr = buffer + start_row * screen_width;
    character_t* next_row_addr = start_row_addr + screen_width;
    std::size_t copy_size = num_rows * screen_width * sizeof(character_t);

    // TODO Is there a way to pause updates to the display while large operations
    // are happening on the VGA buffer? Is this needed?
    //
    // There does not appear to be a way to double buffer the VGA hardware so
    // pausing updates would be the only way to avoid a possible update to the
    // screen during such operations?
    std::memcpy(start_row_addr, next_row_addr, copy_size);
}

} // namespace vga

} // namespace video

} // namespace driver
