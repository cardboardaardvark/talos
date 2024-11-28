#pragma once

namespace platform
{

namespace ibmpc
{

void console_put_char(char c) noexcept;
void init_console() noexcept;
void update_status_memory() noexcept;
void update_status_clock() noexcept;

} // namespace ibmpc

} // namespace platform
