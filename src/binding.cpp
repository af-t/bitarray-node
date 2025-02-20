#include "node_api.h"
#include "bitarray.h"
#include <unordered_map>
#include <string>
#include <vector>

// Global map to store BitArray instances with unique keys
std::unordered_map<std::string, BitArray> bitArrayInstances;
size_t instanceCounter = 0;

std::string generateKey() {
    return "bitarray_" + std::to_string(instanceCounter++);
}
void Throw(napi_env &env, const char* message) {
  napi_throw_error(env, nullptr, message);
}

// Function: create(size)
napi_value Create(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 1) {
        napi_throw_error(env, nullptr, "Invalid number of arguments. Expected 1 argument.");
        return nullptr;
    }

    size_t size;
    napi_get_value_uint32(env, args[0], &size);

    // Create a new BitArray instance
    std::string key = generateKey();
    bitArrayInstances.emplace(key, BitArray(size));

    // Return the key as a string
    napi_value result;
    napi_create_string_utf8(env, key.c_str(), key.size(), &result);
    return result;
}

// Function: get(key, pos)
napi_value Get(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 2) {
        napi_throw_error(env, nullptr, "Invalid number of arguments. Expected 2 arguments.");
        return nullptr;
    }

    // Extract key
    size_t keyLength;
    char keyBuffer[256];
    napi_get_value_string_utf8(env, args[0], keyBuffer, sizeof(keyBuffer), &keyLength);
    std::string key(keyBuffer, keyLength);

    // Extract position
    size_t pos;
    napi_get_value_uint32(env, args[1], &pos);

    // Access the BitArray instance
    auto it = bitArrayInstances.find(key);
    if (it == bitArrayInstances.end()) {
        Throw(env, "Invalid key. No BitArray instance found.");
        return nullptr;
    }

    // Call the get method and return the result as an integer
    int value;
    try {
        value = it->second.get(pos) ? 1 : 0;
    } catch (const std::exception& e) {
        Throw(env, e.what());
        return nullptr;
    }

    napi_value result;
    napi_create_int32(env, value, &result);
    return result;
}

// Function: set(key, pos, value)
napi_value Set(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value args[3];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 3) {
        napi_throw_error(env, nullptr, "Invalid number of arguments. Expected 3 arguments.");
        return nullptr;
    }

    // Extract key
    size_t keyLength;
    char keyBuffer[256];
    napi_get_value_string_utf8(env, args[0], keyBuffer, sizeof(keyBuffer), &keyLength);
    std::string key(keyBuffer, keyLength);

    // Extract position
    size_t pos;
    napi_get_value_uint32(env, args[1], &pos);

    // Extract value
    bool value;
    napi_get_value_bool(env, args[2], &value);

    // Access the BitArray instance
    auto it = bitArrayInstances.find(key);
    if (it == bitArrayInstances.end()) {
        Throw(env, "Invalid key. No BitArray instance found.");
        return nullptr;
    }

    // Call the set method
    try {
        it->second.set(pos, value);
    } catch (const std::exception& e) {
        Throw(env, e.what());
    }

    return nullptr;
}

// Function: resize(key, newSize)
napi_value Resize(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 2) {
        napi_throw_error(env, nullptr, "Invalid number of arguments. Expected 2 arguments.");
        return nullptr;
    }

    // Extract key
    size_t keyLength;
    char keyBuffer[256];
    napi_get_value_string_utf8(env, args[0], keyBuffer, sizeof(keyBuffer), &keyLength);
    std::string key(keyBuffer, keyLength);

    // Extract new size
    size_t newSize;
    napi_get_value_uint32(env, args[1], &newSize);

    // Access the BitArray instance
    auto it = bitArrayInstances.find(key);
    if (it == bitArrayInstances.end()) {
        Throw(env, "Invalid key. No BitArray instance found.");
        return nullptr;
    }

    // Call the resize method
    try {
        it->second.resize(newSize);
    } catch (const std::exception& e) {
        Throw(env, e.what());
    }

    return nullptr;
}

// Function: bitwiseAnd(key1, key2)
napi_value BitwiseAnd(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 2) {
        Throw(env, "Invalid number of arguments. Expected 2 arguments.");
        return nullptr;
    }

    // Extract keys
    size_t key1Length, key2Length;
    char key1Buffer[256], key2Buffer[256];
    napi_get_value_string_utf8(env, args[0], key1Buffer, sizeof(key1Buffer), &key1Length);
    napi_get_value_string_utf8(env, args[1], key2Buffer, sizeof(key2Buffer), &key2Length);
    std::string key1(key1Buffer, key1Length);
    std::string key2(key2Buffer, key2Length);

    // Access the BitArray instances
    auto it1 = bitArrayInstances.find(key1);
    auto it2 = bitArrayInstances.find(key2);
    if (it1 == bitArrayInstances.end() || it2 == bitArrayInstances.end()) {
        Throw(env, "Invalid key(s). No BitArray instance found.");
        return nullptr;
    }

    // Call the bitwiseAnd method
    try {
        it1->second.bitwiseAnd(it2->second);
    } catch (const std::exception& e) {
        Throw(env, e.what());
    }

    return nullptr;
}

// Function: dispose(key)
napi_value Dispose(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 1) {
        Throw(env, "Invalid number of arguments. Expected 1 argument.");
        return nullptr;
    }

    // Extract key
    size_t keyLength;
    char keyBuffer[256];
    napi_get_value_string_utf8(env, args[0], keyBuffer, sizeof(keyBuffer), &keyLength);
    std::string key(keyBuffer, keyLength);

    // Remove the BitArray instance
    auto it = bitArrayInstances.find(key);
    if (it == bitArrayInstances.end()) {
        Throw(env, "Invalid key. No BitArray instance found.");
        return nullptr;
    }

    bitArrayInstances.erase(it);

    return nullptr;
}

// Function: bitwiseOr(key1, key2)
napi_value BitwiseOr(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 2) {
        Throw(env, "Invalid number of arguments. Expected 2 arguments.");
        return nullptr;
    }

    // Extract keys
    size_t key1Length, key2Length;
    char key1Buffer[256], key2Buffer[256];
    napi_get_value_string_utf8(env, args[0], key1Buffer, sizeof(key1Buffer), &key1Length);
    napi_get_value_string_utf8(env, args[1], key2Buffer, sizeof(key2Buffer), &key2Length);
    std::string key1(key1Buffer, key1Length);
    std::string key2(key2Buffer, key2Length);

    // Access the BitArray instances
    auto it1 = bitArrayInstances.find(key1);
    auto it2 = bitArrayInstances.find(key2);
    if (it1 == bitArrayInstances.end() || it2 == bitArrayInstances.end()) {
        Throw(env, "Invalid key(s). No BitArray instance found.");
        return nullptr;
    }

    // Call the bitwiseOr method
    try {
        it1->second.bitwiseOr(it2->second);
    } catch (const std::exception& e) {
        Throw(env, e.what());
    }

    return nullptr;
}

// Function: bitwiseXor(key1, key2)
napi_value BitwiseXor(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 2) {
        Throw(env, "Invalid number of arguments. Expected 2 arguments.");
        return nullptr;
    }

    // Extract keys
    size_t key1Length, key2Length;
    char key1Buffer[256], key2Buffer[256];
    napi_get_value_string_utf8(env, args[0], key1Buffer, sizeof(key1Buffer), &key1Length);
    napi_get_value_string_utf8(env, args[1], key2Buffer, sizeof(key2Buffer), &key2Length);
    std::string key1(key1Buffer, key1Length);
    std::string key2(key2Buffer, key2Length);

    // Access the BitArray instances
    auto it1 = bitArrayInstances.find(key1);
    auto it2 = bitArrayInstances.find(key2);
    if (it1 == bitArrayInstances.end() || it2 == bitArrayInstances.end()) {
        Throw(env, "Invalid key(s). No BitArray instance found.");
        return nullptr;
    }

    // Call the bitwiseXor method
    try {
        it1->second.bitwiseXor(it2->second);
    } catch (const std::exception& e) {
        Throw(env, e.what());
    }

    return nullptr;
}

// Function: bitwiseNot(key)
napi_value BitwiseNot(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 1) {
        Throw(env, "Invalid number of arguments. Expected 1 argument.");
        return nullptr;
    }

    // Extract key
    size_t keyLength;
    char keyBuffer[256];
    napi_get_value_string_utf8(env, args[0], keyBuffer, sizeof(keyBuffer), &keyLength);
    std::string key(keyBuffer, keyLength);

    // Access the BitArray instance
    auto it = bitArrayInstances.find(key);
    if (it == bitArrayInstances.end()) {
        Throw(env, "Invalid key. No BitArray instance found.");
        return nullptr;
    }

    // Call the bitwiseNot method
    try {
        it->second.bitwiseNot();
    } catch (const std::exception& e) {
        Throw(env, e.what());
    }

    return nullptr;
}

// Function: setBatch(key, positions, value)
napi_value SetBatch(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value args[3];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 3) {
        Throw(env, "Invalid number of arguments. Expected 3 arguments.");
        return nullptr;
    }

    // Extract key
    size_t keyLength;
    char keyBuffer[256];
    napi_get_value_string_utf8(env, args[0], keyBuffer, sizeof(keyBuffer), &keyLength);
    std::string key(keyBuffer, keyLength);

    // Extract positions array
    uint32_t arrayLength;
    napi_get_array_length(env, args[1], &arrayLength);
    std::vector<size_t> positions(arrayLength);
    for (uint32_t i = 0; i < arrayLength; ++i) {
        napi_value element;
        napi_get_element(env, args[1], i, &element);
        napi_get_value_uint32(env, element, &positions[i]);
    }

    // Extract value
    bool value;
    napi_get_value_bool(env, args[2], &value);

    // Access the BitArray instance
    auto it = bitArrayInstances.find(key);
    if (it == bitArrayInstances.end()) {
        Throw(env, "Invalid key. No BitArray instance found.");
        return nullptr;
    }

    // Call the setBatch method
    try {
        it->second.setBatch(positions, value);
    } catch (const std::exception& e) {
        Throw(env, e.what());
    }

    return nullptr;
}

// Function: getBatch(key, positions)
napi_value GetBatch(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 2) {
        Throw(env, "Invalid number of arguments. Expected 2 arguments.");
        return nullptr;
    }

    // Extract key
    size_t keyLength;
    char keyBuffer[256];
    napi_get_value_string_utf8(env, args[0], keyBuffer, sizeof(keyBuffer), &keyLength);
    std::string key(keyBuffer, keyLength);

    // Extract positions array
    uint32_t arrayLength;
    napi_get_array_length(env, args[1], &arrayLength);
    std::vector<size_t> positions(arrayLength);
    for (uint32_t i = 0; i < arrayLength; ++i) {
        napi_value element;
        napi_get_element(env, args[1], i, &element);
        napi_get_value_uint32(env, element, &positions[i]);
    }

    // Access the BitArray instance
    auto it = bitArrayInstances.find(key);
    if (it == bitArrayInstances.end()) {
        Throw(env, "Invalid key. No BitArray instance found.");
        return nullptr;
    }

    // Call the getBatch method
    std::vector<bool> results;
    try {
        results = it->second.getBatch(positions);
    } catch (const std::exception& e) {
        Throw(env, e.what());
        return nullptr;
    }

    // Convert results to an array of integers
    napi_value resultArray;
    napi_create_array_with_length(env, results.size(), &resultArray);
    for (size_t i = 0; i < results.size(); ++i) {
        napi_value value;
        napi_create_int32(env, results[i] ? 1 : 0, &value);
        napi_set_element(env, resultArray, i, value);
    }

    return resultArray;
}

// Function: setRange(key, min, max, value)
napi_value SetRange(napi_env env, napi_callback_info info) {
    size_t argc = 4;
    napi_value args[4];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 4) {
        Throw(env, "Invalid number of arguments. Expected 4 arguments.");
        return nullptr;
    }

    // Extract key
    size_t keyLength;
    char keyBuffer[256];
    napi_get_value_string_utf8(env, args[0], keyBuffer, sizeof(keyBuffer), &keyLength);
    std::string key(keyBuffer, keyLength);

    // Extract min, max, and value
    size_t min, max;
    bool value;
    napi_get_value_uint32(env, args[1], &min);
    napi_get_value_uint32(env, args[2], &max);
    napi_get_value_bool(env, args[3], &value);

    // Access the BitArray instance
    auto it = bitArrayInstances.find(key);
    if (it == bitArrayInstances.end()) {
        Throw(env, "Invalid key. No BitArray instance found.");
        return nullptr;
    }

    // Call the setRange method
    try {
        it->second.setRange(min, max, value);
    } catch (const std::exception& e) {
        Throw(env, e.what());
    }

    return nullptr;
}

// Function: getRange(key, min, max)
napi_value GetRange(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value args[3];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 3) {
        Throw(env, "Invalid number of arguments. Expected 3 arguments.");
        return nullptr;
    }

    // Extract key
    size_t keyLength;
    char keyBuffer[256];
    napi_get_value_string_utf8(env, args[0], keyBuffer, sizeof(keyBuffer), &keyLength);
    std::string key(keyBuffer, keyLength);

    // Extract min and max
    size_t min, max;
    napi_get_value_uint32(env, args[1], &min);
    napi_get_value_uint32(env, args[2], &max);

    // Access the BitArray instance
    auto it = bitArrayInstances.find(key);
    if (it == bitArrayInstances.end()) {
        Throw(env, "Invalid key. No BitArray instance found.");
        return nullptr;
    }

    // Call the getRange method
    std::vector<bool> results;
    try {
        results = it->second.getRange(min, max);
    } catch (const std::exception& e) {
        Throw(env, e.what());
        return nullptr;
    }

    // Convert results to an array of integers
    napi_value resultArray;
    napi_create_array_with_length(env, results.size(), &resultArray);
    for (size_t i = 0; i < results.size(); ++i) {
        napi_value value;
        napi_create_int32(env, results[i] ? 1 : 0, &value);
        napi_set_element(env, resultArray, i, value);
    }

    return resultArray;
}

// Function: findFirst(key)
napi_value FindFirstSet(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 1) {
        Throw(env, "Invalid number of arguments. Expected 1 argument.");
        return nullptr;
    }

    // Extract key
    size_t keyLength;
    char keyBuffer[256];
    napi_get_value_string_utf8(env, args[0], keyBuffer, sizeof(keyBuffer), &keyLength);
    std::string key(keyBuffer, keyLength);

    // Access the BitArray instance
    auto it = bitArrayInstances.find(key);
    if (it == bitArrayInstances.end()) {
        Throw(env, "Invalid key. No BitArray instance found.");
        return nullptr;
    }

    // Call the findFirstSet method
    int result;
    try {
        result = it->second.findFirst();
    } catch (const std::exception& e) {
        Throw(env, e.what());
        return nullptr;
    }

    // Return the result as an integer
    napi_value returnValue;
    napi_create_int32(env, result, &returnValue);
    return returnValue;
}

// Function: countSetBits(key)
napi_value CountSetBits(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 1) {
        Throw(env, "Invalid number of arguments. Expected 1 argument.");
        return nullptr;
    }

    // Extract key
    size_t keyLength;
    char keyBuffer[256];
    napi_get_value_string_utf8(env, args[0], keyBuffer, sizeof(keyBuffer), &keyLength);
    std::string key(keyBuffer, keyLength);

    // Access the BitArray instance
    auto it = bitArrayInstances.find(key);
    if (it == bitArrayInstances.end()) {
        Throw(env, "Invalid key. No BitArray instance found.");
        return nullptr;
    }

    // Call the countSetBits method
    size_t result;
    try {
        result = it->second.countSetBits();
    } catch (const std::exception& e) {
        Throw(env, e.what());
        return nullptr;
    }

    // Return the result as an integer
    napi_value returnValue;
    napi_create_uint32(env, result, &returnValue);
    return returnValue;
}

// Function: countBytesUsed(key)
napi_value CountBytesUsed(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 1) {
        Throw(env, "Invalid number of arguments. Expected 1 argument.");
        return nullptr;
    }

    // Extract key
    size_t keyLength;
    char keyBuffer[256];
    napi_get_value_string_utf8(env, args[0], keyBuffer, sizeof(keyBuffer), &keyLength);
    std::string key(keyBuffer, keyLength);

    // Access the BitArray instance
    auto it = bitArrayInstances.find(key);
    if (it == bitArrayInstances.end()) {
        Throw(env, "Invalid key. No BitArray instance found.");
        return nullptr;
    }

    // Call the countBytesUsed method
    size_t result;
    try {
        result = it->second.countBytesUsed();
    } catch (const std::exception& e) {
        Throw(env, e.what());
        return nullptr;
    }

    // Return the result as an integer
    napi_value returnValue;
    napi_create_uint32(env, result, &returnValue);
    return returnValue;
}

// Function: serialize(key)
napi_value Serialize(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 1) {
        Throw(env, "Invalid number of arguments. Expected 1 argument.");
        return nullptr;
    }

    // Extract key
    size_t keyLength;
    char keyBuffer[256];
    napi_get_value_string_utf8(env, args[0], keyBuffer, sizeof(keyBuffer), &keyLength);
    std::string key(keyBuffer, keyLength);

    // Access the BitArray instance
    auto it = bitArrayInstances.find(key);
    if (it == bitArrayInstances.end()) {
        Throw(env, "Invalid key. No BitArray instance found.");
        return nullptr;
    }

    // Call the serialize method
    std::string result;
    try {
        result = it->second.serialize();
    } catch (const std::exception& e) {
        Throw(env, e.what());
        return nullptr;
    }

    // Return the result as a string
    napi_value returnValue;
    napi_create_string_utf8(env, result.c_str(), result.size(), &returnValue);
    return returnValue;
}

// Function: deserialize(serialized)
napi_value Deserialize(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 1) {
        Throw(env, "Invalid number of arguments. Expected 1 argument.");
        return nullptr;
    }

    // Extract serialized string
    size_t serializedLength;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &serializedLength);
    std::string serialized(serializedLength, '\0');
    napi_get_value_string_utf8(env, args[0], &serialized[0], serializedLength + 1, &serializedLength);

    // Call the deserialize method
    BitArray instance = BitArray::deserialize(serialized);

    // Generate a new key and store the instance
    std::string key = generateKey();
    bitArrayInstances.emplace(key, std::move(instance));

    // Return the key as a string
    napi_value returnValue;
    napi_create_string_utf8(env, key.c_str(), key.size(), &returnValue);
    return returnValue;
}

// Function: getSize(key)
napi_value GetSize(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 1) {
        Throw(env, "Invalid number of arguments. Expected 1 argument.");
        return nullptr;
    }

    // Extract key
    size_t keyLength;
    char keyBuffer[256];
    napi_get_value_string_utf8(env, args[0], keyBuffer, sizeof(keyBuffer), &keyLength);
    std::string key(keyBuffer, keyLength);

    // Access the BitArray instance
    auto it = bitArrayInstances.find(key);
    if (it == bitArrayInstances.end()) {
        Throw(env, "Invalid key. No BitArray instance found.");
        return nullptr;
    }

    // Call the getSize method
    size_t result;
    try {
        result = it->second.getSize();
    } catch (const std::exception& e) {
        Throw(env, e.what());
        return nullptr;
    }

    // Return the result as an integer
    napi_value returnValue;
    napi_create_uint32(env, result, &returnValue);
    return returnValue;
}

// Initialize the module
napi_value Init(napi_env env, napi_value exports) {
    napi_property_descriptor desc[] = {
        {"create", nullptr, Create, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"get", nullptr, Get, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"set", nullptr, Set, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"resize", nullptr, Resize, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"bitwiseAnd", nullptr, BitwiseAnd, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"bitwiseOr", nullptr, BitwiseOr, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"bitwiseXor", nullptr, BitwiseXor, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"bitwiseNot", nullptr, BitwiseNot, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setBatch", nullptr, SetBatch, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getBatch", nullptr, GetBatch, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setRange", nullptr, SetRange, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getRange", nullptr, GetRange, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"findFirstSet", nullptr, FindFirstSet, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"countSetBits", nullptr, CountSetBits, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"countBytesUsed", nullptr, CountBytesUsed, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"serialize", nullptr, Serialize, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"deserialize", nullptr, Deserialize, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getSize", nullptr, GetSize, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"dispose", nullptr, Dispose, nullptr, nullptr, nullptr, napi_default, nullptr},
    };

    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
