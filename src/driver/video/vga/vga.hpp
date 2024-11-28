#pragma once

#include <cstdint>

namespace driver

{

namespace video
{

namespace vga
{

#define VGA_BUFFER_ADDRESS 0xB8000
#define VGA_COMMAND_PORT 0x3D4
#define VGA_DATA_PORT 0x3D5

#define VGA_CURSOR_POSITION_HIGH 14
#define VGA_CUSOR_POSITION_LOW 15

#define VGA_HEIGHT 25
#define VGA_WIDTH 80

using character_t = std::uint16_t;
using color_t = std::uint8_t;

enum class Color : color_t
{
    black = 0,
    blue = 1,
    green = 2,
    cyan = 3,
    red = 4,
    magenta = 5,
    brown = 6,
    light_grey = 7,
    dark_grey = 8,
    light_blue = 9,
    light_green = 10,
    light_cyan = 11,
    light_red = 12,
    light_magenta = 13,
    light_brown = 14,
    white = 15,
};

extern character_t* buffer;

color_t color(Color fg, Color bg) noexcept;
character_t character(char character, color_t colors) noexcept;
character_t character(char character, Color fg, Color bg) noexcept;
void place_character(character_t character, std::uint8_t row, std::uint8_t column) noexcept;
void place_string(const char* string, color_t color, std::uint8_t row, std::uint8_t column) noexcept;
void place_cursor(std::uint8_t row, std::uint8_t column) noexcept;
void scroll(std::uint8_t start_row, std::uint8_t num_rows) noexcept;

} // namespace vga

} // namespace video

} // namespace driver
