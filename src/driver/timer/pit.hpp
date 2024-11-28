#include <cstddef>

namespace driver
{

namespace timer
{

const uint32_t pit_clock_freq = 1193182; // in hz

const uint8_t pit_port_command = 0x43;
const uint8_t pit_port_channel_0 = 0x40;
const uint8_t pit_port_channel_1 = 0x41;
const uint8_t pit_port_channel_2 = 0x42;

const uint8_t pit_select_channel_0 = 0 << 6;
const uint8_t pit_select_channel_1 = 1 << 6;
const uint8_t pit_select_channel_2 = 2 << 6;

const uint8_t pit_access_latch = 0 << 4; // latch count value
const uint8_t pit_access_lobyte = 1 << 5;
const uint8_t pit_access_hibyte = 2 << 4;
const uint8_t pit_access_lohi = 3 << 4;

const uint8_t pit_mode_iotc = 0 << 1; // interrupt on terminal count
const uint8_t pit_mode_hro = 1 << 1; // hardware re-triggerable one-shot
const uint8_t pit_mode_rg = 2 << 1; // rate generator
const uint8_t pit_mode_swg = 3 << 1; // square wave generator
const uint8_t pit_mode_sts = 4 << 1; // software triggered strobe
const uint8_t pit_mode_hts = 5 << 1; // hardware triggered strobe

const uint8_t pit_divisor_16bit = 0; // 16 bit number
const uint8_t pit_divisor_bcd = 1; // binary coded decimal

} // namespace timer

} // namespace driver
