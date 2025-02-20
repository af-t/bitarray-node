BitArray Node is an implementation of bit storage optimized for native performance.

## Features
- Efficient bit value storage
- High performance with native binding
- Easy to use in Node.js projects
- Supports Proxy for more flexible access
- Built-in error handling for size validation and storage status

## Installation
Use npm to install:
```sh
npm install af-t/bitarray-node
```

## Usage
Import and use it in your Node.js code:
```javascript
const BitArray = require('bitarray-node');

// Create a BitArray with 8 bits
const bits = new BitArray(8);
bits.set(0, 1);
console.log(bits.get(0)); // Output: 1

// Using proxy for easier access
const proxiedBits = new BitArray(8, true);
proxiedBits[1] = 1;
console.log(proxiedBits[1]); // Output: 1
```

## Error Handling
BitArray has built-in error handling:

- Invalid Size: Throws an error if size is not a positive number.
- Storage Disposed: Throws an error if accessing a disposed storage.
- Size or Type Mismatch: Throws an error if there is a size or type inconsistency.

## Contribution
Open a pull request or report an issue if you find a bug or want to add new features.

## License
This project is licensed under the MIT License.
