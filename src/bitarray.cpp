#include "bitarray.h"
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <functional>
#include <limits>

#ifdef _MSC_VER
#include <intrin.h>
inline int ctzll(unsigned __int64 value) {
    unsigned long index;
    return _BitScanForward64(&index, value) ? index : 64;
}
inline int popcntll(unsigned __int64 value) {
    return __popcnt64(value);
}
#else
#define ctzll(x) __builtin_ctzll(x)
#define popcntll(x) __builtin_popcountll(x)
#endif

// Constructor
BitArray::BitArray(size_t size) : bits(size), disposed(false) {
    if (size > std::numeric_limits<size_t>::max() - 63) {
        throw std::out_of_range("Requested size overflows capacity");
    }
    size_t words = size / 64 + (size % 64 != 0 ? 1 : 0);
    data.resize(words, 0); // Allocate 64-bit words for the given number of bits
}

// Destructor
BitArray::~BitArray() {
    dispose();
}

// Dispose the instance
void BitArray::dispose() {
    data.clear();
    bits = 0;
    disposed = true;
}

// Helper function to validate a position
void BitArray::validate(size_t pos) const {
    if (disposed) throw std::runtime_error("Storage is disposed");
    if (pos >= bits) throw std::out_of_range("Invalid position: " + std::to_string(pos));
}

// Helper function to calculate word index and bit offset
std::pair<size_t, size_t> BitArray::getPos(size_t pos) const {
    return {pos / 64, pos % 64};
}

// Helper function to clear unused padding bits in the last word
void BitArray::clearUnusedBits() {
    if (disposed || data.empty()) return;
    size_t lastWordBits = bits % 64;
    if (lastWordBits > 0) {
        data.back() &= ((static_cast<uint64_t>(1) << lastWordBits) - 1);
    }
}

// Set a bit at a specific position
void BitArray::set(size_t pos, bool value) {
    validate(pos);
    auto [index, offset] = getPos(pos);
    if (value) {
        data[index] |= (static_cast<uint64_t>(1) << offset); // Set the bit
    } else {
        data[index] &= ~(static_cast<uint64_t>(1) << offset); // Clear the bit
    }
}

// Get a bit at a specific position
bool BitArray::get(size_t pos) const {
    validate(pos);
    auto [index, offset] = getPos(pos);
    return (data[index] & (static_cast<uint64_t>(1) << offset)) != 0; // Check if the bit is set
}

// Resize the BitArray
void BitArray::resize(size_t newSize) {
    if (disposed) throw std::runtime_error("Storage is disposed");
    if (newSize > std::numeric_limits<size_t>::max() - 63) {
        throw std::out_of_range("Requested size overflows capacity");
    }
    size_t words = newSize / 64 + (newSize % 64 != 0 ? 1 : 0);
    std::vector<uint64_t> newData(words, 0);
    std::copy(data.begin(), data.begin() + std::min(data.size(), newData.size()), newData.begin());
    data = std::move(newData);
    bits = newSize;
    clearUnusedBits();
}

// Perform bitwise operations
void BitArray::bitwiseOp(const BitArray& other, std::function<uint64_t(uint64_t, uint64_t)> op) {
    if (bits != other.bits) throw std::invalid_argument("Size mismatch");
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = op(data[i], other.data[i]);
    }
}

// Specific bitwise operations
void BitArray::bitwiseAnd(const BitArray& other) {
    if (bits != other.bits) throw std::invalid_argument("Size mismatch");
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] &= other.data[i];
    }
}

void BitArray::bitwiseOr(const BitArray& other) {
    if (bits != other.bits) throw std::invalid_argument("Size mismatch");
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] |= other.data[i];
    }
}

void BitArray::bitwiseXor(const BitArray& other) {
    if (bits != other.bits) throw std::invalid_argument("Size mismatch");
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] ^= other.data[i];
    }
}

void BitArray::bitwiseNot() {
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = ~data[i];
    }
    clearUnusedBits();
}

// Set multiple bits at once
void BitArray::setBatch(const std::vector<size_t>& positions, bool value) {
    if (disposed) throw std::runtime_error("Storage is disposed");
    for (auto pos : positions) {
        if (pos >= bits) throw std::out_of_range("Invalid position: " + std::to_string(pos));
    }
    if (value) {
        for (auto pos : positions) {
            data[pos / 64] |= (static_cast<uint64_t>(1) << (pos % 64));
        }
    } else {
        for (auto pos : positions) {
            data[pos / 64] &= ~(static_cast<uint64_t>(1) << (pos % 64));
        }
    }
}

// Get multiple bits at once
std::vector<bool> BitArray::getBatch(const std::vector<size_t>& positions) const {
    if (disposed) throw std::runtime_error("Storage is disposed");
    std::vector<bool> result;
    result.reserve(positions.size());
    for (auto pos : positions) {
        if (pos >= bits) throw std::out_of_range("Invalid position: " + std::to_string(pos));
        result.push_back((data[pos / 64] & (static_cast<uint64_t>(1) << (pos % 64))) != 0);
    }
    return result;
}

// Set a range of bits
void BitArray::setRange(size_t min, size_t max, bool value) {
    validate(min);
    validate(max);
    if (min > max) throw std::invalid_argument("min cannot be greater than max");

    auto [startWord, startOffset] = getPos(min);
    auto [endWord, endOffset] = getPos(max);

    if (startWord == endWord) {
        size_t k = endOffset - startOffset + 1;
        uint64_t mask = (k == 64) ? ~0ULL : ((1ULL << k) - 1);
        mask <<= startOffset;
        if (value) {
            data[startWord] |= mask;
        } else {
            data[startWord] &= ~mask;
        }
    } else {
        // 1. Modify partial start word
        uint64_t startMask = ~0ULL << startOffset;
        if (value) {
            data[startWord] |= startMask;
        } else {
            data[startWord] &= ~startMask;
        }

        // 2. Modify fully spanned intermediate words
        uint64_t fillVal = value ? ~0ULL : 0ULL;
        for (size_t w = startWord + 1; w < endWord; ++w) {
            data[w] = fillVal;
        }

        // 3. Modify partial end word
        uint64_t endMask = (endOffset == 63) ? ~0ULL : ((1ULL << (endOffset + 1)) - 1);
        if (value) {
            data[endWord] |= endMask;
        } else {
            data[endWord] &= ~endMask;
        }
    }
}

// Get a range of bits
std::vector<bool> BitArray::getRange(size_t min, size_t max) const {
    validate(min);
    validate(max);
    if (min > max) throw std::invalid_argument("min cannot be greater than max");
    
    std::vector<bool> result;
    result.reserve(max - min + 1);
    for (size_t i = min; i <= max; ++i) {
        result.push_back((data[i / 64] & (static_cast<uint64_t>(1) << (i % 64))) != 0);
    }
    return result;
}

// Find the first set bit (1)
size_t BitArray::findFirst() const {
    for (size_t i = 0; i < data.size(); ++i) {
        if (data[i] != 0) {
            return i * 64 + ctzll(data[i]);
        }
    }
    return static_cast<size_t>(-1); // Return -1 if no bit is set
}

// Count the number of set bits (1s)
size_t BitArray::countSetBits() const {
    size_t count = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        count += popcntll(data[i]);
    }
    return count;
}

// Count the number of bytes used (non-zero bytes)
size_t BitArray::countBytesUsed() const {
    if (disposed) throw std::runtime_error("Storage is disposed");
    size_t count = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        uint64_t word = data[i];
        // Branchless bitwise parallel check for non-zero bytes in a 64-bit word
        uint64_t temp = word | ((word & 0x7f7f7f7f7f7f7f7fULL) + 0x7f7f7f7f7f7f7f7fULL);
        count += popcntll(temp & 0x8080808080808080ULL);
    }
    return count;
}

// Serialize the BitArray to a JSON-like string
std::string BitArray::serialize() const {
    std::ostringstream oss;
    oss << "{\"bits\":" << bits << ",\"data\":[";
    for (size_t i = 0; i < data.size(); ++i) {
        oss << data[i];
        if (i < data.size() - 1) oss << ",";
    }
    oss << "]}";
    return oss.str();
}

// Deserialize a BitArray from a JSON-like string
BitArray BitArray::deserialize(const std::string& serialized) {
    size_t bitsPos = serialized.find("\"bits\":");
    if (bitsPos == std::string::npos) {
        throw std::invalid_argument("Malformed serialized data: missing \"bits\"");
    }
    size_t bitsStart = bitsPos + 7;
    size_t bitsEnd = serialized.find(",", bitsStart);
    if (bitsEnd == std::string::npos) {
        throw std::invalid_argument("Malformed serialized data: missing comma after bits");
    }
    size_t bits = std::stoull(serialized.substr(bitsStart, bitsEnd - bitsStart));

    size_t dataPos = serialized.find("\"data\":[");
    if (dataPos == std::string::npos) {
        throw std::invalid_argument("Malformed serialized data: missing \"data\"");
    }
    size_t dataStart = dataPos + 8;
    size_t dataEnd = serialized.find("]", dataStart);
    if (dataEnd == std::string::npos) {
        throw std::invalid_argument("Malformed serialized data: missing closing bracket for data");
    }
    std::string dataStr = serialized.substr(dataStart, dataEnd - dataStart);

    std::vector<uint64_t> dataVec;
    size_t pos = 0;
    if (!dataStr.empty()) {
        while (pos < dataStr.size()) {
            size_t commaPos = dataStr.find(",", pos);
            if (commaPos == std::string::npos) commaPos = dataStr.size();
            dataVec.push_back(static_cast<uint64_t>(std::stoull(dataStr.substr(pos, commaPos - pos))));
            pos = commaPos + 1;
        }
    }

    if (bits > std::numeric_limits<size_t>::max() - 63) {
        throw std::out_of_range("Deserialized bits size overflows capacity");
    }
    size_t expectedSize = bits / 64 + (bits % 64 != 0 ? 1 : 0);
    if (dataVec.size() != expectedSize) {
        throw std::invalid_argument("Malformed serialized data: data array size mismatch");
    }

    BitArray instance(bits);
    instance.data = std::move(dataVec);
    instance.clearUnusedBits();
    return instance;
}

// Get the total number of bits
size_t BitArray::getSize() const {
    return bits;
}
