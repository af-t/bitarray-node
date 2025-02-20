// errors
const untitled_0 = new Error('The "size" parameter must be an integer and positive number');
const untitled_1 = new Error('Storage is disposed');
const untitled_2 = new Error('Size mismatch or invalid type');

const backend = require('./build/Release/bitarray.node');

class BitArray {
  #disposed = false;
  #key;

  constructor(size, proxy = false) {
    if (!Number.isInteger(size) || size < 0) throw untitled_0;
    this.#key = backend.create(size);

    if (proxy) return new Proxy([], {
      get: (_, p) => {
        let ret;
        try {
          const int = Number(p);
          if (Number.isInteger(int)) ret = backend.get(this.#key, int);
          else throw 1;
        } catch {
          ret = this[p]?.bind ? this[p].bind(this) : this[p];
        } finally {
          return ret;
        }
      },
      set: (_, p, v) => {
        try {
          const int = Number(p);
          if (Number.isInteger(int)) {
            backend.set(this.#key, int, v);
            return true;
          }
          throw 1;
        } catch {
          return false;
        }
      }
    });
  }

  #isDisposed() { if (this.#disposed) throw untitled_1; }

  set(pos, bool) {
    this.#isDisposed();
    backend.set(this.#key, pos, bool);
  }

  get(pos) {
    this.#isDisposed();
    return backend.get(this.#key, pos);
  }

  resize(size) {
    if (!Number.isInteger(size) || size < 0) throw untitled_0;
    this.#isDisposed();
    backend.resize(this.#key, size);
  }

  bitwiseOp(other, op) {
    if (!(other instanceof BitArray) || this.size !== other.size) throw untitled_2;
    this.#isDisposed();
    backend[op](this.#key, other.#key);
  }

  bitwiseAnd(other) {
    this.#isDisposed();
    this.bitwiseOp(other, 'bitwiseAnd');
  }
  bitwiseOr(other) {
    this.#isDisposed();
    this.bitwiseOp(other, 'bitwiseOr');
  }
  bitwiseXor(other) {
    this.#isDisposed();
    this.bitwiseOp(other, 'bitwiseXor');
  }

  bitwiseNot() {
    this.#isDisposed();
    backend.bitwiseNot(this.#key);
  }

  setBatch(positions, bool) {
    this.#isDisposed();
    backend.setBatch(this.#key, positions, bool);
  }

  getBatch(positions) {
    this.#isDisposed();
    return backend.getBatch(this.#key, positions);
  }

  setRange(min, max, bool) {
    this.#isDisposed();
    backend.setRange(this.#key, min, max, bool);
  }
  getRange(min, max) {
    this.#isDisposed();
    return backend.getRange(this.#key, min, max);
  }

  findFirst() {
    this.#isDisposed();
    return backend.findFirstSet(this.#key);
  }

  countSetBits() {
    this.#isDisposed();
    return backend.countSetBits(this.#key);
  }

  countBytesUsed() {
    this.#isDisposed();
    return backend.countBytesUsed(this.#key);
  }

  serialize() {
    this.#isDisposed();
    return backend.serialize(this.#key);
  }

  static deserialize(serialized) {
    const instance = new BitArray(1);
    const key = backend.deserialize(serialized);
    backend.dispose(instance.#key);

    instance.#key = key;
    return instance;
  }
  static from(serialized) { return BitArray.deserialize(serialized); }

  dispose() {
    this.#disposed = true;
    backend.dispose(this.#key);
  }

  get size() { return backend.getSize(this.#key); }
  get length() { return backend.getSize(this.#key); }

  *[Symbol.iterator]() {
    const length = this.size;
    for (let i = 0; i < length; i++) yield backend.get(this.#key, i);
  }
}

module.exports = BitArray;
