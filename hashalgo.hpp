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
        HashUnknown = 0,
        HashDumb,
        HashMD5,
        HashCRC16,
        HashCRC32
    };
    HashFunctionType hash_function_type( const std::string& t );
    size_t           get_hash_bytes_len( HashFunctionType t );
    HashFunction    create_hash_function( HashFunctionType t );
    HashFunction    create_hash_function( const std::string& t );
} // otus_hw8
