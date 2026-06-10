# BitArray Node

BitArray Node is an implementation of bit storage optimized specifically for **64-bit native performance**. It bridges Node.js and C++ to handle bitwise storage and operations efficiently.

## Features

- **64-bit Architecture Optimization**: Uses `uint64_t` words internally. Bitwise operations process 64 bits at a time, making them up to 8x faster than traditional 8-bit byte-level or bit-by-bit operations.
- **Hardware-Accelerated Instructions**:
  - Trailing Zero Count (`TZCNT` via `__builtin_ctzll` or `_BitScanForward64`) for near-instant search of the first set bit.
  - Population Count (`POPCNT` via `__builtin_popcountll` or `__popcnt64`) for O(1) clock-cycle set bit counting.
- **Automatic Memory Cleanup**: Leverages JavaScript's `FinalizationRegistry` to automatically release native C++ memory resources when the JavaScript `BitArray` object is garbage collected.
- **Truthy/Falsy Coercion**: Support standard JS coercion rules when writing bit values (e.g., passing `1` or `0` will automatically convert to `true`/`false`).
- **Flexible Interface**: Support standard accessor methods as well as a JavaScript `Proxy` mode for array-like index access (fully supporting symbols like iteration, prototype inspection, and `instanceof`).
- **Bitwise Logic**: Fast native implementation for `AND`, `OR`, `XOR`, and `NOT` operations directly inside C++.

## Installation

Install using npm:
```bash
npm install af-t/bitarray-node
```

## Usage

Import and use it in your Node.js code:

```javascript
const BitArray = require('bitarray-node');

// 1. Standard Usage
const bits = new BitArray(8);
bits.set(0, true);
bits.set(2, 1); // Auto coerced to true
console.log(bits.get(0)); // Output: 1
console.log(bits.get(1)); // Output: 0

// 2. Proxy Access (Array-like)
const proxiedBits = new BitArray(8, true);
proxiedBits[1] = 1; // Set index 1 to true
console.log(proxiedBits[1]); // Output: 1
console.log(proxiedBits instanceof BitArray); // Output: true

// 3. Batch and Range Operations
const batchBits = new BitArray(10);
batchBits.setBatch([2, 4, 6], true);
console.log(batchBits.getBatch([2, 3, 4])); // Output: [1, 0, 1]

const rangeBits = new BitArray(10);
rangeBits.setRange(2, 5, true);
console.log(rangeBits.getRange(1, 6)); // Output: [0, 1, 1, 1, 1, 0]

// 4. Bitwise Logic
const a = new BitArray(8);
const b = new BitArray(8);
a.set(2, true);
b.set(2, true);
b.set(5, true);

a.bitwiseAnd(b);
console.log(a.get(2)); // Output: 1
console.log(a.get(5)); // Output: 0

const c = new BitArray(8);
c.set(0, true);
c.bitwiseNot();
console.log(c.get(0)); // Output: 0
console.log(c.get(1)); // Output: 1

// 5. Fast Set Bit Counting & Search
const countBits = new BitArray(100);
countBits.set(42, true);
countBits.set(77, true);
console.log(countBits.countSetBits()); // Output: 2 (uses native popcnt)
console.log(countBits.findFirst());     // Output: 42 (uses native tzcnt)
console.log(countBits.countBytesUsed()); // Output: 2 (counts exact non-zero bytes branchlessly)

// 6. Dynamic Resizing
const resizable = new BitArray(4);
resizable.set(3, true);
resizable.resize(8); // Dynamically changes capacity to 8 bits
console.log(resizable.size); // Output: 8
console.log(resizable.get(3)); // Output: 1

// 7. Iteration (ES6 Iterator)
// Highly optimized chunked iteration retrieves 1024-bit blocks at a time,
// avoiding expensive C++ boundary-crossing overhead.
for (const bit of countBits) {
  // Loops through all 100 bits
}

// 8. Manual Disposal (Optional)
// Native resources are cleaned up by Garbage Collection, 
// but you can release them immediately (idempotent):
bits.dispose();
```

## API Reference

### Getters
* **`size`**: Returns the capacity (in bits) of the BitArray.
* **`length`**: Alias for `size`.

### Methods
* **`set(pos, value)`**: Sets a bit at `pos` to `value` (coerced to boolean).
* **`get(pos)`**: Returns the bit value (`1` or `0`) at `pos`.
* **`resize(newSize)`**: Resizes the BitArray. Clears padding bits on downsizing.
* **`bitwiseAnd(other)`** / **`bitwiseOr(other)`** / **`bitwiseXor(other)`**: Bitwise ops against `other` BitArray (must be same size).
* **`bitwiseNot()`**: Bitwise NOT (negate) all bits.
* **`setBatch(positions, value)`**: Transactional batch set.
* **`getBatch(positions)`**: Batch get.
* **`setRange(min, max, value)`**: Word-level optimized range set.
* **`getRange(min, max)`**: Range get.
* **`findFirst()`**: Returns index of first set bit, or `-1` if none.
* **`countSetBits()`**: Returns count of `1` bits.
* **`countBytesUsed()`**: Returns count of non-zero bytes.
* **`serialize()`**: Returns a serialized JSON string representation.
* **`dispose()`**: Idempotently releases native resources early.

### Static Methods
* **`BitArray.deserialize(serializedString)`**: Creates a BitArray from a serialized string.
* **`BitArray.from(serializedString)`**: Alias for `deserialize()`.

## Serialization

You can serialize a `BitArray` to a string and recreate it later:

```javascript
const original = new BitArray(16);
original.set(5, true);

const serialized = original.serialize();
const restored = BitArray.from(serialized);

console.log(restored.get(5)); // Output: 1
```

## Error Handling

BitArray has built-in error handling:

- **Invalid Size**: Throws an error if size is not a non-negative integer or exceeds `0xFFFFFFFF` (4,294,967,295 bits / 512MB capacity limit).
- **Storage Disposed**: Throws an error if attempting to access a manually disposed storage.
- **Size or Type Mismatch**: Throws an error if attempting bitwise operations between BitArrays of different sizes.

## Contribution

Open a pull request or report an issue if you find a bug or want to add new features.

## License

This project is licensed under the MIT License.
