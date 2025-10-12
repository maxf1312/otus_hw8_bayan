#pragma once

#include <ctime>
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <functional>


namespace otus_hw8
{
    /// @brief Тип хэш-кода просто массив байт
    using HashCode = std::vector<uint8_t>;

    using HashFunction = std::function<HashCode (const uint8_t *data, size_t data_size)>;
    
    enum class HashFunctionType : uint8_t
    {
        HashDumb = 0,
        HashMD5,
        HashCRC32
    };
    HashFunction    create_hash_function( HashFunctionType t );
} // otus_hw8
