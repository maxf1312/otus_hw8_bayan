#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <numeric>

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>


#include "bayan_internal.h"

namespace otus_hw8{
    using namespace std;

    file_sz_t FileInfo::block_sz_ = 5;

    FileInfo::FileInfo(const std::string file_path) 
        : file_path_{ bfs::absolute(bfs::path(file_path)).string() }, file_sz_{bfs::file_size(file_path_)} 
    {
        ;
    }
    
    bool FileInfo::is_hashes_eq(const FileInfo& rhs) const
    {
        return block_count() == rhs.block_count() && equal(begin(hash_codes_), end(hash_codes_), begin(rhs.hash_codes_));        
    } 

    void FileInfo::read_and_hash_blocks(size_t up_to_blocks_count)
    {
        using namespace boost::interprocess;
        size_t region_off = block_count() * block_sz_;
        size_t region_size = (up_to_blocks_count - block_count()) * block_sz_;
        
        file_mapping m_file(file_path_.c_str(), read_only);
        mapped_region region(m_file, read_only, region_off, region_size);

        //Get the address of the region
        void* region_addr = region.get_address();

        //Get the size of the region
        size_t real_region_size = region.get_size();
        
        std::ignore = region_addr;
        std::ignore = real_region_size;
    }

    FileInfo::HashCode FileInfo::hash_data(const uint8_t* data, size_t data_size)
    {
        return std::accumulate(data, data + data_size, FileInfo::HashCode{},
                [](const auto& new_v, const auto& sum_v) -> HashCode { return (sum_v + new_v) << 1; }
        );
    }


};