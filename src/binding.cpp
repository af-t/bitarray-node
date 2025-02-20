#include "node_api.h"
#include "bitarray.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>
#include <limits>

// Global map to store BitArray instances with unique keys, guarded by a mutex for thread-safety in worker threads
std::unordered_map<std::string, BitArray> bitArrayInstances;
std::mutex registryMutex;
size_t instanceCounter = 0;

std::string generateKey() {
    return "bitarray_" + std::to_string(instanceCounter++);
}

void Throw(napi_env &env, const char* message) {
    napi_throw_error(env, nullptr, message);
}

// Safe argument extraction helper functions
napi_status get_string_arg(napi_env env, napi_value value, std::string& out_str) {
    napi_valuetype valuetype;
    napi_status status = napi_typeof(env, value, &valuetype);
    if (status != napi_ok || valuetype != napi_string) {
        return napi_string_expected;
    }
    size_t length = 0;
    status = napi_get_value_string_utf8(env, value, nullptr, 0, &length);
    if (status != napi_ok) return status;
    
    out_str.resize(length);
    status = napi_get_value_string_utf8(env, value, &out_str[0], length + 1, &length);
    return status;
}

napi_status get_uint32_arg(napi_env env, napi_value value, uint32_t& out_val) {
    napi_valuetype valuetype;
    napi_status status = napi_typeof(env, value, &valuetype);
    if (status != napi_ok || valuetype != napi_number) {
        return napi_number_expected;
    }
    return napi_get_value_uint32(env, value, &out_val);
}

napi_status get_uint32_array_arg(napi_env env, napi_value value, std::vector<size_t>& out_vec) {
    bool is_array = false;
    napi_status status = napi_is_array(env, value, &is_array);
    if (status != napi_ok || !is_array) {
        return napi_array_expected;
    }
    uint32_t length = 0;
    status = napi_get_array_length(env, value, &length);
    if (status != napi_ok) return status;
    
    out_vec.resize(length);
    for (uint32_t i = 0; i < length; ++i) {
        napi_value element;
        status = napi_get_element(env, value, i, &element);
        if (status != napi_ok) return status;
        
        uint32_t val = 0;
        status = napi_get_value_uint32(env, element, &val);
        if (status != napi_ok) return status;
        
        out_vec[i] = val;
    }
    return napi_ok;
}

// Function: create(size)
napi_value Create(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    if (napi_get_cb_info(env, info, &argc, args, nullptr, nullptr) != napi_ok || argc < 1) {
        Throw(env, "Invalid number of arguments. Expected 1 argument.");
        return nullptr;
    }

    uint32_t sizeVal;
    if (get_uint32_arg(env, args[0], sizeVal) != napi_ok) {
        Throw(env, "Argument 1 (size) must be a positive number.");
        return nullptr;
    }

    try {
        std::lock_guard<std::mutex> lock(registryMutex);
        std::string key = generateKey();
        bitArrayInstances.emplace(key, BitArray(sizeVal));

        napi_value result;
        napi_create_string_utf8(env, key.c_str(), key.size(), &result);
        return result;
    } catch (const std::exception& e) {
        Throw(env, e.what());
        return nullptr;
    }
}

// Function: get(key, pos)
napi_value Get(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    if (napi_get_cb_info(env, info, &argc, args, nullptr, nullptr) != napi_ok || argc < 2) {
        Throw(env, "Invalid number of arguments. Expected 2 arguments.");
        return nullptr;
    }

    std::string key;
    if (get_string_arg(env, args[0], key) != napi_ok) {
        Throw(env, "Argument 1 (key) must be a string.");
        return nullptr;
    }

    uint32_t posVal;
    if (get_uint32_arg(env, args[1], posVal) != napi_ok) {
        Throw(env, "Argument 2 (position) must be a number.");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(registryMutex);
    auto it = bitArrayInstances.find(key);
    if (it == bitArrayInstances.end()) {
        Throw(env, "Invalid key. No BitArray instance found.");
        return nullptr;
    }

    int value;
    try {
        value = it->second.get(posVal) ? 1 : 0;
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
    if (napi_get_cb_info(env, info, &argc, args, nullptr, nullptr) != napi_ok || argc < 3) {
        Throw(env, "Invalid number of arguments. Expected 3 arguments.");
        return nullptr;
    }

    std::string key;
    if (get_string_arg(env, args[0], key) != napi_ok) {
        Throw(env, "Argument 1 (key) must be a string.");
        return nullptr;
    }

    uint32_t posVal;
    if (get_uint32_arg(env, args[1], posVal) != napi_ok) {
        Throw(env, "Argument 2 (position) must be a number.");
        return nullptr;
    }

    napi_value coercedValue;
    if (napi_coerce_to_bool(env, args[2], &coercedValue) != napi_ok) {
        Throw(env, "Failed to coerce argument 3 to boolean.");
        return nullptr;
    }
    bool value;
    if (napi_get_value_bool(env, coercedValue, &value) != napi_ok) {
        Throw(env, "Argument 3 must be a boolean.");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(registryMutex);
    auto it = bitArrayInstances.find(key);
    if (it == bitArrayInstances.end()) {
        Throw(env, "Invalid key. No BitArray instance found.");
        return nullptr;
    }

    try {
        it->second.set(posVal, value);
    } catch (const std::exception& e) {
        Throw(env, e.what());
    }

    return nullptr;
}

// Function: resize(key, newSize)
napi_value Resize(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    if (napi_get_cb_info(env, info, &argc, args, nullptr, nullptr) != napi_ok || argc < 2) {
        Throw(env, "Invalid number of arguments. Expected 2 arguments.");
        return nullptr;
    }

    std::string key;
    if (get_string_arg(env, args[0], key) != napi_ok) {
        Throw(env, "Argument 1 (key) must be a string.");
        return nullptr;
    }

    uint32_t newSizeVal;
    if (get_uint32_arg(env, args[1], newSizeVal) != napi_ok) {
        Throw(env, "Argument 2 (size) must be a number.");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(registryMutex);
    auto it = bitArrayInstances.find(key);
    if (it == bitArrayInstances.end()) {
        Throw(env, "Invalid key. No BitArray instance found.");
        return nullptr;
    }

    try {
        it->second.resize(newSizeVal);
    } catch (const std::exception& e) {
        Throw(env, e.what());
    }

    return nullptr;
}

// Function: bitwiseAnd(key1, key2)
napi_value BitwiseAnd(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    if (napi_get_cb_info(env, info, &argc, args, nullptr, nullptr) != napi_ok || argc < 2) {
        Throw(env, "Invalid number of arguments. Expected 2 arguments.");
        return nullptr;
    }

    std::string key1, key2;
    if (get_string_arg(env, args[0], key1) != napi_ok || get_string_arg(env, args[1], key2) != napi_ok) {
        Throw(env, "Arguments must be strings.");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(registryMutex);
    auto it1 = bitArrayInstances.find(key1);
    auto it2 = bitArrayInstances.find(key2);
    if (it1 == bitArrayInstances.end() || it2 == bitArrayInstances.end()) {
        Throw(env, "Invalid key(s). No BitArray instance found.");
        return nullptr;
    }

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
    if (napi_get_cb_info(env, info, &argc, args, nullptr, nullptr) != napi_ok || argc < 1) {
        Throw(env, "Invalid number of arguments. Expected 1 argument.");
        return nullptr;
    }

    std::string key;
    if (get_string_arg(env, args[0], key) != napi_ok) {
        Throw(env, "Argument 1 (key) must be a string.");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(registryMutex);
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
    if (napi_get_cb_info(env, info, &argc, args, nullptr, nullptr) != napi_ok || argc < 2) {
        Throw(env, "Invalid number of arguments. Expected 2 arguments.");
        return nullptr;
    }

    std::string key1, key2;
    if (get_string_arg(env, args[0], key1) != napi_ok || get_string_arg(env, args[1], key2) != napi_ok) {
        Throw(env, "Arguments must be strings.");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(registryMutex);
    auto it1 = bitArrayInstances.find(key1);
    auto it2 = bitArrayInstances.find(key2);
    if (it1 == bitArrayInstances.end() || it2 == bitArrayInstances.end()) {
        Throw(env, "Invalid key(s). No BitArray instance found.");
        return nullptr;
    }

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
    if (napi_get_cb_info(env, info, &argc, args, nullptr, nullptr) != napi_ok || argc < 2) {
        Throw(env, "Invalid number of arguments. Expected 2 arguments.");
        return nullptr;
    }

    std::string key1, key2;
    if (get_string_arg(env, args[0], key1) != napi_ok || get_string_arg(env, args[1], key2) != napi_ok) {
        Throw(env, "Arguments must be strings.");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(registryMutex);
    auto it1 = bitArrayInstances.find(key1);
    auto it2 = bitArrayInstances.find(key2);
    if (it1 == bitArrayInstances.end() || it2 == bitArrayInstances.end()) {
        Throw(env, "Invalid key(s). No BitArray instance found.");
        return nullptr;
    }

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
    if (napi_get_cb_info(env, info, &argc, args, nullptr, nullptr) != napi_ok || argc < 1) {
        Throw(env, "Invalid number of arguments. Expected 1 argument.");
        return nullptr;
    }

    std::string key;
    if (get_string_arg(env, args[0], key) != napi_ok) {
        Throw(env, "Argument 1 (key) must be a string.");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(registryMutex);
    auto it = bitArrayInstances.find(key);
    if (it == bitArrayInstances.end()) {
        Throw(env, "Invalid key. No BitArray instance found.");
        return nullptr;
    }

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
    if (napi_get_cb_info(env, info, &argc, args, nullptr, nullptr) != napi_ok || argc < 3) {
        Throw(env, "Invalid number of arguments. Expected 3 arguments.");
        return nullptr;
    }

    std::string key;
    if (get_string_arg(env, args[0], key) != napi_ok) {
        Throw(env, "Argument 1 (key) must be a string.");
        return nullptr;
    }

    std::vector<size_t> positions;
    if (get_uint32_array_arg(env, args[1], positions) != napi_ok) {
        Throw(env, "Argument 2 (positions) must be an array of numbers.");
        return nullptr;
    }

    napi_value coercedValue;
    if (napi_coerce_to_bool(env, args[2], &coercedValue) != napi_ok) {
        Throw(env, "Failed to coerce argument 3 to boolean.");
        return nullptr;
    }
    bool value;
    if (napi_get_value_bool(env, coercedValue, &value) != napi_ok) {
        Throw(env, "Argument 3 must be a boolean.");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(registryMutex);
    auto it = bitArrayInstances.find(key);
    if (it == bitArrayInstances.end()) {
        Throw(env, "Invalid key. No BitArray instance found.");
        return nullptr;
    }

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
    if (napi_get_cb_info(env, info, &argc, args, nullptr, nullptr) != napi_ok || argc < 2) {
        Throw(env, "Invalid number of arguments. Expected 2 arguments.");
        return nullptr;
    }

    std::string key;
    if (get_string_arg(env, args[0], key) != napi_ok) {
        Throw(env, "Argument 1 (key) must be a string.");
        return nullptr;
    }

    std::vector<size_t> positions;
    if (get_uint32_array_arg(env, args[1], positions) != napi_ok) {
        Throw(env, "Argument 2 (positions) must be an array of numbers.");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(registryMutex);
    auto it = bitArrayInstances.find(key);
    if (it == bitArrayInstances.end()) {
        Throw(env, "Invalid key. No BitArray instance found.");
        return nullptr;
    }

    std::vector<bool> results;
    try {
        results = it->second.getBatch(positions);
    } catch (const std::exception& e) {
        Throw(env, e.what());
        return nullptr;
    }

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
    if (napi_get_cb_info(env, info, &argc, args, nullptr, nullptr) != napi_ok || argc < 4) {
        Throw(env, "Invalid number of arguments. Expected 4 arguments.");
        return nullptr;
    }

    std::string key;
    if (get_string_arg(env, args[0], key) != napi_ok) {
        Throw(env, "Argument 1 (key) must be a string.");
        return nullptr;
    }

    uint32_t minVal, maxVal;
    if (get_uint32_arg(env, args[1], minVal) != napi_ok || get_uint32_arg(env, args[2], maxVal) != napi_ok) {
        Throw(env, "Arguments 2 and 3 must be numbers.");
        return nullptr;
    }

    napi_value coercedValue;
    if (napi_coerce_to_bool(env, args[3], &coercedValue) != napi_ok) {
        Throw(env, "Failed to coerce argument 4 to boolean.");
        return nullptr;
    }
    bool value;
    if (napi_get_value_bool(env, coercedValue, &value) != napi_ok) {
        Throw(env, "Argument 4 must be a boolean.");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(registryMutex);
    auto it = bitArrayInstances.find(key);
    if (it == bitArrayInstances.end()) {
        Throw(env, "Invalid key. No BitArray instance found.");
        return nullptr;
    }

    try {
        it->second.setRange(minVal, maxVal, value);
    } catch (const std::exception& e) {
        Throw(env, e.what());
    }

    return nullptr;
}

// Function: getRange(key, min, max)
napi_value GetRange(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value args[3];
    if (napi_get_cb_info(env, info, &argc, args, nullptr, nullptr) != napi_ok || argc < 3) {
        Throw(env, "Invalid number of arguments. Expected 3 arguments.");
        return nullptr;
    }

    std::string key;
    if (get_string_arg(env, args[0], key) != napi_ok) {
        Throw(env, "Argument 1 (key) must be a string.");
        return nullptr;
    }

    uint32_t minVal, maxVal;
    if (get_uint32_arg(env, args[1], minVal) != napi_ok || get_uint32_arg(env, args[2], maxVal) != napi_ok) {
        Throw(env, "Arguments 2 and 3 must be numbers.");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(registryMutex);
    auto it = bitArrayInstances.find(key);
    if (it == bitArrayInstances.end()) {
        Throw(env, "Invalid key. No BitArray instance found.");
        return nullptr;
    }

    std::vector<bool> results;
    try {
        results = it->second.getRange(minVal, maxVal);
    } catch (const std::exception& e) {
        Throw(env, e.what());
        return nullptr;
    }

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
    if (napi_get_cb_info(env, info, &argc, args, nullptr, nullptr) != napi_ok || argc < 1) {
        Throw(env, "Invalid number of arguments. Expected 1 argument.");
        return nullptr;
    }

    std::string key;
    if (get_string_arg(env, args[0], key) != napi_ok) {
        Throw(env, "Argument 1 (key) must be a string.");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(registryMutex);
    auto it = bitArrayInstances.find(key);
    if (it == bitArrayInstances.end()) {
        Throw(env, "Invalid key. No BitArray instance found.");
        return nullptr;
    }

    size_t result;
    try {
        result = it->second.findFirst();
    } catch (const std::exception& e) {
        Throw(env, e.what());
        return nullptr;
    }

    napi_value returnValue;
    if (result == static_cast<size_t>(-1)) {
        napi_create_int64(env, -1, &returnValue);
    } else {
        napi_create_int64(env, static_cast<int64_t>(result), &returnValue);
    }
    return returnValue;
}

// Function: countSetBits(key)
napi_value CountSetBits(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    if (napi_get_cb_info(env, info, &argc, args, nullptr, nullptr) != napi_ok || argc < 1) {
        Throw(env, "Invalid number of arguments. Expected 1 argument.");
        return nullptr;
    }

    std::string key;
    if (get_string_arg(env, args[0], key) != napi_ok) {
        Throw(env, "Argument 1 (key) must be a string.");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(registryMutex);
    auto it = bitArrayInstances.find(key);
    if (it == bitArrayInstances.end()) {
        Throw(env, "Invalid key. No BitArray instance found.");
        return nullptr;
    }

    size_t result;
    try {
        result = it->second.countSetBits();
    } catch (const std::exception& e) {
        Throw(env, e.what());
        return nullptr;
    }

    napi_value returnValue;
    napi_create_int64(env, static_cast<int64_t>(result), &returnValue);
    return returnValue;
}

// Function: countBytesUsed(key)
napi_value CountBytesUsed(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    if (napi_get_cb_info(env, info, &argc, args, nullptr, nullptr) != napi_ok || argc < 1) {
        Throw(env, "Invalid number of arguments. Expected 1 argument.");
        return nullptr;
    }

    std::string key;
    if (get_string_arg(env, args[0], key) != napi_ok) {
        Throw(env, "Argument 1 (key) must be a string.");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(registryMutex);
    auto it = bitArrayInstances.find(key);
    if (it == bitArrayInstances.end()) {
        Throw(env, "Invalid key. No BitArray instance found.");
        return nullptr;
    }

    size_t result;
    try {
        result = it->second.countBytesUsed();
    } catch (const std::exception& e) {
        Throw(env, e.what());
        return nullptr;
    }

    napi_value returnValue;
    napi_create_int64(env, static_cast<int64_t>(result), &returnValue);
    return returnValue;
}

// Function: serialize(key)
napi_value Serialize(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    if (napi_get_cb_info(env, info, &argc, args, nullptr, nullptr) != napi_ok || argc < 1) {
        Throw(env, "Invalid number of arguments. Expected 1 argument.");
        return nullptr;
    }

    std::string key;
    if (get_string_arg(env, args[0], key) != napi_ok) {
        Throw(env, "Argument 1 (key) must be a string.");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(registryMutex);
    auto it = bitArrayInstances.find(key);
    if (it == bitArrayInstances.end()) {
        Throw(env, "Invalid key. No BitArray instance found.");
        return nullptr;
    }

    std::string result;
    try {
        result = it->second.serialize();
    } catch (const std::exception& e) {
        Throw(env, e.what());
        return nullptr;
    }

    napi_value returnValue;
    napi_create_string_utf8(env, result.c_str(), result.size(), &returnValue);
    return returnValue;
}

// Function: deserialize(serialized)
napi_value Deserialize(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    if (napi_get_cb_info(env, info, &argc, args, nullptr, nullptr) != napi_ok || argc < 1) {
        Throw(env, "Invalid number of arguments. Expected 1 argument.");
        return nullptr;
    }

    std::string serialized;
    if (get_string_arg(env, args[0], serialized) != napi_ok) {
        Throw(env, "Argument 1 (serialized) must be a string.");
        return nullptr;
    }

    try {
        BitArray instance = BitArray::deserialize(serialized);
        std::lock_guard<std::mutex> lock(registryMutex);
        std::string key = generateKey();
        bitArrayInstances.emplace(key, std::move(instance));

        napi_value returnValue;
        napi_create_string_utf8(env, key.c_str(), key.size(), &returnValue);
        return returnValue;
    } catch (const std::exception& e) {
        Throw(env, e.what());
        return nullptr;
    }
}

// Function: getSize(key)
napi_value GetSize(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    if (napi_get_cb_info(env, info, &argc, args, nullptr, nullptr) != napi_ok || argc < 1) {
        Throw(env, "Invalid number of arguments. Expected 1 argument.");
        return nullptr;
    }

    std::string key;
    if (get_string_arg(env, args[0], key) != napi_ok) {
        Throw(env, "Argument 1 (key) must be a string.");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(registryMutex);
    auto it = bitArrayInstances.find(key);
    if (it == bitArrayInstances.end()) {
        Throw(env, "Invalid key. No BitArray instance found.");
        return nullptr;
    }

    size_t result;
    try {
        result = it->second.getSize();
    } catch (const std::exception& e) {
        Throw(env, e.what());
        return nullptr;
    }

    napi_value returnValue;
    napi_create_int64(env, static_cast<int64_t>(result), &returnValue);
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
