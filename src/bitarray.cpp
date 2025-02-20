#include "bitarray.h"
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <functional>

// Constructor
BitArray::BitArray(size_t size) : bits(size), disposed(false) {
    data.resize((size + 7) / 8, 0); // Allocate bytes for the given number of bits
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

// Helper function to calculate byte index and bit offset
std::pair<size_t, size_t> BitArray::getPos(size_t pos) const {
    return {pos / 8, pos % 8};
}

// Set a bit at a specific position
void BitArray::set(size_t pos, bool value) {
    validate(pos);
    auto [index, offset] = getPos(pos);
    if (value) {
        data[index] |= (1 << offset); // Set the bit
    } else {
        data[index] &= ~(1 << offset); // Clear the bit
    }
}

// Get a bit at a specific position
bool BitArray::get(size_t pos) const {
    validate(pos);
    auto [index, offset] = getPos(pos);
    return (data[index] & (1 << offset)) != 0; // Check if the bit is set
}

// Resize the BitArray
void BitArray::resize(size_t newSize) {
    validate(0); // Ensure not disposed
    std::vector<uint8_t> newData((newSize + 7) / 8, 0);
    std::copy(data.begin(), data.begin() + std::min(data.size(), newData.size()), newData.begin());
    data = std::move(newData);
    bits = newSize;
}

// Perform bitwise operations
void BitArray::bitwiseOp(const BitArray& other, std::function<uint8_t(uint8_t, uint8_t)> op) {
    if (bits != other.bits) throw std::invalid_argument("Size mismatch");
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = op(data[i], other.data[i]);
    }
}

// Specific bitwise operations
void BitArray::bitwiseAnd(const BitArray& other) {
    bitwiseOp(other, [](uint8_t a, uint8_t b) { return a & b; });
}

void BitArray::bitwiseOr(const BitArray& other) {
    bitwiseOp(other, [](uint8_t a, uint8_t b) { return a | b; });
}

void BitArray::bitwiseXor(const BitArray& other) {
    bitwiseOp(other, [](uint8_t a, uint8_t b) { return a ^ b; });
}

void BitArray::bitwiseNot() {
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = ~data[i];
    }
    size_t lastByteBits = bits % 8;
    if (lastByteBits > 0) {
        data[data.size() - 1] &= ((1 << lastByteBits) - 1); // Mask out unused bits in the last byte
    }
}

// Set multiple bits at once
void BitArray::setBatch(const std::vector<size_t>& positions, bool value) {
    for (auto pos : positions) {
        set(pos, value);
    }
}

// Get multiple bits at once
std::vector<bool> BitArray::getBatch(const std::vector<size_t>& positions) const {
    std::vector<bool> result;
    result.reserve(positions.size());
    for (auto pos : positions) {
        result.push_back(get(pos));
    }
    return result;
}

// Set a range of bits
void BitArray::setRange(size_t min, size_t max, bool value) {
    validate(min);
    validate(max);
    for (size_t i = min; i <= max; ++i) {
        set(i, value);
    }
}

// Get a range of bits
std::vector<bool> BitArray::getRange(size_t min, size_t max) const {
    validate(min);
    validate(max);
    std::vector<bool> result;
    result.reserve(max - min + 1);
    for (size_t i = min; i <= max; ++i) {
        result.push_back(get(i));
    }
    return result;
}

// Find the first set bit (1)
size_t BitArray::findFirst() const {
    for (size_t i = 0; i < bits; ++i) {
        if (get(i)) return i;
    }
    return static_cast<size_t>(-1); // Return -1 if no bit is set
}

// Count the number of set bits (1s)
size_t BitArray::countSetBits() const {
    size_t count = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        uint8_t byte = data[i];
        while (byte) {
            count += byte & 1;
            byte >>= 1;
        }
    }
    return count;
}

// Count the number of bytes used (non-zero bytes)
size_t BitArray::countBytesUsed() const {
    return std::count_if(data.begin(), data.end(), [](uint8_t byte) { return byte != 0; });
}

// Serialize the BitArray to a JSON-like string
std::string BitArray::serialize() const {
    std::ostringstream oss;
    oss << "{\"bits\":" << bits << ",\"data\":[";
    for (size_t i = 0; i < data.size(); ++i) {
        oss << static_cast<int>(data[i]);
        if (i < data.size() - 1) oss << ",";
    }
    oss << "]}";
    return oss.str();
}

// Deserialize a BitArray from a JSON-like string
BitArray BitArray::deserialize(const std::string& serialized) {
    size_t bitsStart = serialized.find("\"bits\":") + 7;
    size_t bitsEnd = serialized.find(",", bitsStart);
    size_t bits = std::stoull(serialized.substr(bitsStart, bitsEnd - bitsStart));

    size_t dataStart = serialized.find("\"data\":[") + 8;
    size_t dataEnd = serialized.find("]", dataStart);
    std::string dataStr = serialized.substr(dataStart, dataEnd - dataStart);

    std::vector<uint8_t> dataVec;
    size_t pos = 0;
    while (pos < dataStr.size()) {
        size_t commaPos = dataStr.find(",", pos);
        if (commaPos == std::string::npos) commaPos = dataStr.size();
        dataVec.push_back(static_cast<uint8_t>(std::stoul(dataStr.substr(pos, commaPos - pos))));
        pos = commaPos + 1;
    }

    BitArray instance(bits);
    instance.data = std::move(dataVec);
    return instance;
}

// Get the total number of bits
size_t BitArray::getSize() const {
    return bits;
}
