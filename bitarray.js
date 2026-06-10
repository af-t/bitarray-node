// errors
const untitled_0 = new Error('The "size" parameter must be an integer and positive number');
const untitled_1 = new Error('Storage is disposed');
const untitled_2 = new Error('Size mismatch or invalid type');

const backend = require('./build/Release/bitarray.node');

const registry = new FinalizationRegistry(key => {
  try {
    backend.dispose(key);
  } catch {}
});

class BitArray {
  #disposed = false;
  #key;

  constructor(size, proxy = false) {
    if (!Number.isInteger(size) || size < 0 || size > 0xFFFFFFFF) throw untitled_0;
    this.#key = backend.create(size);
    registry.register(this, this.#key, this);

    if (proxy) {
      const isIndex = (p) => {
        if (typeof p !== 'string') return false;
        const int = Number(p);
        return Number.isInteger(int) && String(int) === p && int >= 0;
      };

      return new Proxy(this, {
        get: (target, p) => {
          if (isIndex(p)) {
            return backend.get(this.#key, Number(p));
          }
          const val = Reflect.get(target, p);
          if (typeof val === 'function') {
            return val.bind(target);
          }
          return val;
        },
        set: (target, p, v) => {
          if (isIndex(p)) {
            backend.set(this.#key, Number(p), v);
            return true;
          }
          if (p === 'length') {
            target.resize(v);
            return true;
          }
          return Reflect.set(target, p, v);
        }
      });
    }
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
    if (!Number.isInteger(size) || size < 0 || size > 0xFFFFFFFF) throw untitled_0;
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
    try {
      const key = backend.deserialize(serialized);
      registry.unregister(instance);
      backend.dispose(instance.#key);
      instance.#key = key;
      registry.register(instance, key, instance);
      return instance;
    } catch (err) {
      instance.dispose();
      throw err;
    }
  }
  static from(serialized) { return BitArray.deserialize(serialized); }

  dispose() {
    if (this.#disposed) return;
    this.#disposed = true;
    registry.unregister(this);
    try {
      backend.dispose(this.#key);
    } catch {}
  }

  get size() { return backend.getSize(this.#key); }
  get length() { return backend.getSize(this.#key); }

  *[Symbol.iterator]() {
    const length = this.size;
    const chunkSize = 1024;
    for (let i = 0; i < length; i += chunkSize) {
      const end = Math.min(i + chunkSize - 1, length - 1);
      const chunk = backend.getRange(this.#key, i, end);
      for (let j = 0; j < chunk.length; j++) {
        yield chunk[j];
      }
    }
  }
}

module.exports = BitArray;
