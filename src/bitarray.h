#ifndef BITARRAY_H
#define BITARRAY_H

#include <vector>
#include <string>
#include <stdexcept>
#include <cstdint>
#include <functional>

class BitArray {
private:
    std::vector<uint8_t> data; // Internal storage for bits
    size_t bits;               // Total number of bits
    bool disposed;             // Whether the instance is disposed

    // Helper function to validate a position
    void validate(size_t pos) const;

    // Helper function to calculate byte index and bit offset
    std::pair<size_t, size_t> getPos(size_t pos) const;

public:
    // Constructor
    explicit BitArray(size_t size);

    // Destructor
    ~BitArray();

    // Dispose the instance
    void dispose();

    // Set a bit at a specific position
    void set(size_t pos, bool value);

    // Get a bit at a specific position
    bool get(size_t pos) const;

    // Resize the BitArray
    void resize(size_t newSize);

    // Perform bitwise operations
    void bitwiseOp(const BitArray& other, std::function<uint8_t(uint8_t, uint8_t)> op);

    // Specific bitwise operations
    void bitwiseAnd(const BitArray& other);
    void bitwiseOr(const BitArray& other);
    void bitwiseXor(const BitArray& other);
    void bitwiseNot();

    // Set multiple bits at once
    void setBatch(const std::vector<size_t>& positions, bool value);

    // Get multiple bits at once
    std::vector<bool> getBatch(const std::vector<size_t>& positions) const;

    // Set a range of bits
    void setRange(size_t min, size_t max, bool value);

    // Get a range of bits
    std::vector<bool> getRange(size_t min, size_t max) const;

    // Find the first set bit (1)
    size_t findFirst() const;

    // Count the number of set bits (1s)
    size_t countSetBits() const;

    // Count the number of bytes used (non-zero bytes)
    size_t countBytesUsed() const;

    // Serialize the BitArray to a JSON-like string
    std::string serialize() const;

    // Deserialize a BitArray from a JSON-like string
    static BitArray deserialize(const std::string& serialized);

    // Get the total number of bits
    size_t getSize() const;
};

#endif // BITARRAY_H
