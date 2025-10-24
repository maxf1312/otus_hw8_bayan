#include <ctime>
#include <iostream>
#include <string>
#include <memory>
#include <algorithm>
#include <numeric>
#include <map>
#include <boost/functional/factory.hpp>
#include <boost/crc.hpp>
#include <boost/uuid/detail/md5.hpp>

#include "hashalgo.hpp"

namespace otus_hw8
{
    using boost::uuids::detail::md5;

    /// @brief Стратегия вычисления хэш-кода для последовательности байт
    struct IHashFunction
    {
        virtual ~IHashFunction() = default;
        virtual size_t hash_len() const = 0; 
        virtual HashCode make_hash(const uint8_t *data, size_t data_size) const = 0; 
        HashCode operator()(const uint8_t *data, size_t data_size) const { return make_hash(data, data_size); }
    };

    using IHashFunctionPtr = std::shared_ptr<IHashFunction>;

    class HashDumbImpl : public IHashFunction
    {
    public:
        virtual size_t hash_len() const override { return 4; } 
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

    class HashMD5Impl : public IHashFunction
    {
    public:
        virtual size_t hash_len() const override 
        {
            return sizeof(md5::digest_type);
        } 
        virtual HashCode make_hash(const uint8_t *data, size_t data_size) const override
        {
            md5 hash;
            hash.process_bytes(data, data_size);

            md5::digest_type md5result;
            hash.get_digest(md5result);

            HashCode::value_type const *beg_md5 = reinterpret_cast<HashCode::value_type*>(&md5result[0]), 
                                       *end_md5 = beg_md5 + sizeof(md5result);
            return HashCode(beg_md5, end_md5);
        }
    };

    template<typename crc_algo_t>
    class HashCRCxxImpl : public IHashFunction
    {
    public:
        virtual size_t hash_len() const override { return crc_algo_t::bit_count / 8; } 
        virtual HashCode make_hash(const uint8_t *data, size_t data_size) const override
        {
            crc_algo_t crc_algo;
            crc_algo.process_bytes(data, data_size);
            typename crc_algo_t::value_type hc = crc_algo.checksum();

            HashCode rv{};
            for (int n = crc_algo_t::bit_count / 8 ; n > 0; --n, hc >>= 8)
                rv.push_back(hc & 0xFF);
            return rv;
        }
    };

    using HashCRC32Impl = HashCRCxxImpl<boost::crc_32_type>;
    using HashCRC16Impl = HashCRCxxImpl<boost::crc_16_type>;

    IHashFunctionPtr create_hash_function_impl(const HashFunctionType t)
    {
        using HashFactory_t = std::function<IHashFunction* ()>;
        static std::map<HashFunctionType, HashFactory_t> registry = { 
            {HashFunctionType::HashDumb,  boost::factory<HashDumbImpl*>()}, 
            {HashFunctionType::HashMD5,   boost::factory<HashMD5Impl*>()}, 
            {HashFunctionType::HashCRC16, boost::factory<HashCRC16Impl*>()},
            {HashFunctionType::HashCRC32, boost::factory<HashCRC32Impl*>()}
        };
        auto p = registry.find(t);
        if (p == registry.end()) 
            return nullptr;
        return IHashFunctionPtr(p->second()); 
    }

    HashFunction create_hash_function(const HashFunctionType t)
    {
        auto spHF = create_hash_function_impl( t );
        return [spHF](const uint8_t *data, size_t data_size) -> HashCode { return (*spHF)(data, data_size);  };
    }

    HashFunctionType hash_function_type( const std::string& t )
    {
        static std::unordered_map<std::string, HashFunctionType> types = { 
            {"dumb", HashFunctionType::HashDumb}, 
            {"md5", HashFunctionType::HashMD5}, 
            {"crc16", HashFunctionType::HashCRC16},
            {"crc32", HashFunctionType::HashCRC32}
        };
        auto p = types.find(t);
        if (p == types.end()) 
            return HashFunctionType::HashUnknown;
        return p->second;
    }
    
    HashFunction    create_hash_function( const std::string& t )
    {
        return create_hash_function( hash_function_type(t) );
    }

    size_t           get_hash_bytes_len( HashFunctionType t )
    {
        auto spF = create_hash_function_impl(t);
        return spF ? spF->hash_len() : 4;
    }

    
} // otus_hw8
