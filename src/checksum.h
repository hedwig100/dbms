#ifndef _CHECKSUM_H
#define _CHECKSUM_H

#include <cstdint>
#include <vector>

namespace dblog {

// Computes checksum of the `bytes`.
uint32_t ComputeChecksum(const std::vector<uint8_t> &bytes);

} // namespace dblog

#endif // _CHECKSUM_H