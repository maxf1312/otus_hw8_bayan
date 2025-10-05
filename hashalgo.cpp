#include <ctime>
#include <iostream>
#include <string>
#include <memory>
#include <algorithm>
#include <numeric>
#include <boost/functional/factory.hpp>

#include "hashalgo.hpp"

namespace otus_hw8
{
    /// @brief Стратегия вычисления хэш-кода для последовательности байт
    struct IHashFunction
    {
        virtual ~IHashFunction() = default;
        virtual HashCode make_hash(const uint8_t *data, size_t data_size) const = 0;
        HashCode operator()(const uint8_t *data, size_t data_size) const { return make_hash(data, data_size); }
    };

    using IHashFunctionPtr = std::shared_ptr<IHashFunction>;

    class HashDumbImpl : public IHashFunction
    {
    public:
        virtual HashCode make_hash(const uint8_t *data, size_t data_size) const override
        {
            uint32_t hc = std::accumulate(data, data + data_size, uint32_t{},
                                          [](const auto &new_v, const auto &sum_v) -> uint32_t
                                          { return (sum_v + new_v) << 1; });
            HashCode rv{};
            for (int n = 4; n > 0; --n, hc >>= 8)
                rv.push_back(hc & 0xFF);
            return rv;
        }
    };

    HashFunction create_hash_function(HashFunctionType t)
    {
        static std::unordered_map<HashFunctionType, boost::factory<IHashFunctionPtr>> registry = { {HashFunctionType::HashDumb, boost::factory<HashDumbImpl*>}
        //, {HashFunctionType::HashMD5, }, {HashFunctionType::HashCRC32} 
        };
        auto p = registry.find(t);
        return p != registry.end() ? p->second() : nullptr;
    }   

} // otus_hw8
