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

        for(uint8_t const* p_begin = reinterpret_cast<uint8_t const*>(region_addr), 
            *p_end = p_begin + region_size; p_begin != p_end; p_begin += block_sz_
        )
        {
            HashCode h = hash_data(p_begin, block_sz_);
            hash_codes_.emplace_back( h );
        }
    }

    FileInfo::HashCode FileInfo::hash_data(const uint8_t* data, size_t data_size)
    {
        return std::accumulate(data, data + data_size, FileInfo::HashCode{},
                [](const auto& new_v, const auto& sum_v) -> HashCode { return (sum_v + new_v) << 1; }
        );
    }


    void FileFinder::find_files()
    {
        for(bfs::directory_iterator cur_file(dir_path_), end_file; cur_file != end_file; ++cur_file)
        {
            
            FileInfo file_inf{cur_file->path().string()};
            (*dest_files_)[file_inf.file_sz_].insert(file_inf); 
        }            
    }
    
    FileDupSearcher::FileDupSearcher() : files_(std::make_shared<FilesCollection_t>()) {}
    
    void FileDupSearcher::add_dir(const std::string& dir_path){
        auto files2 = std::make_shared<FilesCollection_t>();
        FileFinder finder(dir_path, 0, 1, files_);
        finder.find_files();
    }

    void FileDupSearcher::find_duplicates()
    {

    }
        
    void FileDupSearcher::find_duplicates_for_same_file_sizes(FileInfoSet_t& file_set)
    {
        assert(file_set.size() > 1);
        if(file_set.size() <= 1)
            return;
        
        for(auto file_info = file_set.begin(); file_info != file_set.end(); ++file_info)
        {
            find_duplicates_for_file(file_info, file_set.end());
        }
    }


    void FileDupSearcher::find_duplicates_for_file(FileInfoSet_t::iterator file0, FileInfoSet_t::iterator file_end)
    {
        auto file_nxt = file0;
        if( file_nxt++ == file_end ) return;
        if( !file0->block_count() )
            (*file0).read_and_hash_blocks( file0->block_count() + 1);
        for(; file_nxt != file_end ; ++file_nxt)
        {
            ;
        }
    }
}