#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <numeric>

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/regex.hpp>

#include "bayan_internal.h"

namespace otus_hw8{
    using namespace std;

    file_sz_t FileInfoSet_t::block_sz_ = 5;
    file_sz_t FileInfoSet_t::hashcode_sz_ = sizeof(uint32_t);
    
    file_sz_t FileInfo::block_sz()
    {
        return FileInfoSet_t::block_sz();
    }

    size_t FileInfo::hashcode_sz()
    {
        return FileInfoSet_t::hashcode_sz();
    }
        

    FileInfo::FileInfo(const std::string& file_path, FileInfoSet_t& owner) 
        : file_path_{ bfs::absolute(bfs::path(file_path)).string() }, 
          owner_(owner) 
    {
        ;
    }
    
    bool FileInfo::is_hashes_eq(const FileInfo& rhs, bool blk_cnt_must_be_max/*= false*/) const
    {
        size_t block_cnt = std::min(block_count(), rhs.block_count()); 
        if( !block_cnt )
            return false;
        if( blk_cnt_must_be_max && (block_count() < owner_.max_block_count() || block_count() != rhs.block_count()))
            return false;
        auto p_end = begin(hash_codes_); 
        advance(p_end, block_cnt * hashcode_sz()); 
        return equal(begin(hash_codes_), p_end, begin(rhs.hash_codes_));        
    } 

    void FileInfo::read_and_hash_blocks(size_t up_to_blocks_count)
    {
        using namespace boost::interprocess;
        size_t region_off = block_count() * block_sz();
        size_t region_size = (up_to_blocks_count - block_count()) * block_sz();
        
        file_mapping m_file(file_path_.c_str(), read_only);
        mapped_region region(m_file, read_only, region_off, region_size);

        void* region_addr = region.get_address();
        
        size_t real_region_size = region.get_size();
        std::ignore = region_addr;
        std::ignore = real_region_size;

        for(uint8_t const* p_begin = reinterpret_cast<uint8_t const*>(region_addr), 
            *p_end = p_begin + region_size; p_begin != p_end; p_begin += block_sz()
        )
        {
            HashCode h = hash_data(p_begin, block_sz());
            copy(h.begin(), h.end(), back_inserter(hash_codes_));
        }
    }

    FileInfo::HashCode FileInfo::hash_data(const uint8_t* data, size_t data_size)
    {
        return owner_.get_hash_func()(data, data_size);
    }

    bool   FileInfo::check_duplicate(FileInfo& rhs, DupFilePtrSet_t& dup_fileptr_set)
    {
        if( this == &rhs )
            return true;

        bool is_dup = is_hashes_eq(rhs, true);
        if( !is_dup )
            return is_dup;

        // файлы полностью равны - отмечаем это в наборе выбранных указателей на FileInfo 
        if( !duplicates_ )
        {
            duplicates_ = std::make_shared<DupFileSet_t>();
            dup_fileptr_set.insert(duplicates_.get());
        }

        if( !rhs.duplicates_ )
            rhs.duplicates_ = duplicates_;
        
        duplicates_->insert(this);        
        duplicates_->insert(&rhs);        

        return is_dup;
    }


    void  FileInfoSet_t::find_duplicates(DupFilePtrSet_t& dup_fileptr_set)
    {
        for(auto file_info = file_set_.begin(); file_info != file_set_.end(); ++file_info)
        {
            find_duplicates_for_file(file_info, file_set_.end(), dup_fileptr_set);
        }
    }

    void FileInfoSet_t::find_duplicates_for_file(FileInfos_t::iterator file0, FileInfos_t::iterator file_end, DupFilePtrSet_t& dup_fileptr_set)
    {
        auto file_nxt = file0;
        if( file_nxt++ == file_end ) return;
        
        for(; file_nxt != file_end ; ++file_nxt)
        {
            do
            {
                if( file0->block_count() <= file_nxt->block_count() ) 
                    file0->read_and_hash_blocks(file0->block_count() + 1);
                
                if( file0->check_duplicate(*file_nxt, dup_fileptr_set) )
                    break;
                
                if( file0->block_count() > file_nxt->block_count() ) 
                    file_nxt->read_and_hash_blocks(file_nxt->block_count() + 1);
                
                if( file0->check_duplicate(*file_nxt, dup_fileptr_set) )
                    break;
            } while( file0->is_hashes_eq(*file_nxt)
                     && 
                     ( file0->block_count() < max_block_count() || 
                       file_nxt->block_count() < max_block_count() 
                     ) 
                    );
        }
    }
    

    void FileInfoSet_t::set_block_sz(file_sz_t blk_sz)
    {
        block_sz_ = blk_sz;
    }

    void FileInfoSet_t::set_hash_sz(file_sz_t hash_sz)
    {
        hashcode_sz_ = hash_sz;
    }
    
    bool FileFinder::is_file_mask_match(const std::string& file_path) const
    {
        return !file_mask_re_ || file_mask_re_->empty() ||
                file_mask_re_->end() != std::find_if(file_mask_re_->begin(), file_mask_re_->end(),
                                                     [&file_path](const auto& re)
                                                     {
                                                        std::cout << "re:" << re.str() << std::endl;
                                                        std::cout << "file_path:" << file_path << std::endl;
                                                        
                                                        return boost::regex_search(file_path, re);
                                                     }
                                                    );
    }

    void FileFinder::find_files() const
    {
        for(bfs::directory_iterator cur_file(dir_path_), end_file; cur_file != end_file; ++cur_file)
        {
            if( cur_file->is_directory() && depth_ > 0 && 
                (!dirs_to_excl_ || dirs_to_excl_->end() == dirs_to_excl_->find(cur_file->path().string()) ) 
            )
            {
                FileFinder sub_finder(cur_file->path().string(), depth_ - 1, min_file_sz_, dest_files_, dirs_to_excl_, file_mask_re_);
                sub_finder.find_files();
                
            }
            else if( cur_file->is_regular_file() && is_file_mask_match(cur_file->path().string()) )
            {
                file_sz_t file_sz = bfs::file_size(cur_file->path());
                auto& file_set = (*dest_files_)[file_sz];
                file_set.add_file(cur_file->path().string());
            }
        }            
    }

     
    FileDupSearcher::FileDupSearcher(HashFunction const& hash_func) 
        : files_(std::make_shared<FilesCollection_t>()), hash_func_(hash_func)
    {}
    
    void FileDupSearcher::add_dir(const std::string& dir_path, size_t depth, file_sz_t min_size, string_arr_t const& dirs_to_excl, const string_arr_t& file_mask)
    {
        auto sp_dirs_to_excl = std::make_shared<string_set_t>();
        std::transform(dirs_to_excl.begin(), dirs_to_excl.end(), std::inserter(*sp_dirs_to_excl, sp_dirs_to_excl->end()), 
            [](const auto& s_dir){ return bfs::canonical(bfs::path{s_dir}.lexically_normal()).string(); }
        );
        for(const auto& dir: *sp_dirs_to_excl)
            std::cout << "excl dir: " << dir << std::endl;

        auto sp_file_mask_re = std::make_shared<regex_arr_t>();
        std::transform(file_mask.begin(), file_mask.end(), std::inserter(*sp_file_mask_re, sp_file_mask_re->end()), 
            [](const auto& s_mask) { return boost::regex( reg_ex_from_file_mask(s_mask), boost::regex::basic|boost::regex::icase); }
        );
        for(const auto& re: *sp_file_mask_re)
            std::cout << "regex: " << re.str() << std::endl;

        FileFinder finder(dir_path, depth, min_size, files_, sp_dirs_to_excl, sp_file_mask_re);
        finder.find_files();
    }

    void FileDupSearcher::remove_single_file_sets()
    {
        vector<file_sz_t> keys4del; 
        keys4del.reserve((files_->size() + 1) / 2);
        transform(files_->begin(), files_->end(), back_inserter(keys4del), [](const auto& v){ return v.second.size() <= 1 ? v.first : 0; });
        for_each(keys4del.begin(), keys4del.end(), [&](const auto k){ if( k ) files_->erase(k); });
    }

    std::string FileDupSearcher::reg_ex_from_file_mask(const std::string& file_mask_in)
    {
        boost::regex re("\\.", boost::regex::basic|boost::regex::icase);
        auto file_mask = boost::regex_replace(file_mask_in, re, "\\\\.", boost::match_default | boost::format_sed );
        re = boost::regex("?", boost::regex::basic|boost::regex::icase);
        file_mask = boost::regex_replace(file_mask, re, ".", boost::match_default | boost::format_sed );
        re = boost::regex("\\*", boost::regex::basic|boost::regex::icase);
        file_mask = boost::regex_replace(file_mask, re, ".*", boost::match_default | boost::format_sed );
        return file_mask;        
    };

    void FileDupSearcher::find_duplicates()
    {
        remove_single_file_sets();
        for( auto& file_set_by_sz : *files_ )
        {
            auto& [_, file_set] = file_set_by_sz; 
            if(file_set.size() <= 1)
                continue;
            file_set.set_hash_func(hash_func_);
            find_duplicates_for_same_file_sizes(file_set, dup_filepointers_);
        }
    }
        
    void FileDupSearcher::find_duplicates_for_same_file_sizes(FileInfoSet_t& file_set, DupFilePtrSet_t& dup_fileptr_set)
    {
        assert(file_set.size() > 1);
        if(file_set.size() <= 1)
            return;
        
        file_set.find_duplicates(dup_fileptr_set);
    }
}