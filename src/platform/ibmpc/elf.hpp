#include <cstdint>

namespace platform
{

namespace ibmpc
{

typedef struct  __attribute__((packed))
{
    uint32_t name;
    uint32_t type;
    uint32_t flags;
    uint32_t addr;
    uint32_t offset;

} elf32_section_header_t;

} // ibmpc

} // namespace platform
