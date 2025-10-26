#pragma once

#include <ctime>
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <functional>

namespace otus_hw8
{
    /// @brief Тип хэш-кода - просто массив байт
    using HashCode = std::vector<uint8_t>;

    /// @brief Тип стратегии хэширования  
    using HashFunction = std::function<HashCode(const uint8_t *data, size_t data_size)>;

    /// @brief Тип воможных хэш кодов и стратегий для их вычисления
    enum class HashFunctionType : uint8_t
    {
        HashUnknown = 0,
        HashDumb,
        HashMD5,
        HashCRC16,
        HashCRC32
    };

    /// @brief получить тип хэш-функции по названию
    /// @param t - текстовое название (dumb, md5, crc16, crc32)
    /// @return - код типа хэш-функции
    HashFunctionType hash_function_type(const std::string &t);

    /// @brief Получить длину хэш-кода по его коду. Необходимо для отделения одного хэш-кода от другого в общем массиве байт 
    /// @param t код типа хэш-кода
    /// @return размер хэш-кода, байты
    size_t get_hash_bytes_len(HashFunctionType t);

    /// @brief Получить длину хэш-кода по его текстовому имени типа. Необходимо для отделения одного хэш-кода от другого в общем массиве байт 
    /// @param t код типа хэш-кода
    /// @return размер хэш-кода, байты
    size_t get_hash_bytes_len(const std::string &t);

    /// @brief создать хэш-функцию по коду типа 
    /// @param t код типа хэш-кода 
    /// @return функция хэширования
    HashFunction create_hash_function(HashFunctionType t);

    /// @brief создать хэш-функцию по текстовому имени типа 
    /// @param t код типа хэш-кода 
    /// @return функция хэширования
    HashFunction create_hash_function(const std::string &t);
} // otus_hw8
