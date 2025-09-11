#include <iostream>
#include <fstream>
#include <sstream>

#include <map>

#include "bayan_internal.h"

namespace otus_hw8{
    file_sz_t FileInfo::block_sz_ = 5;

    FileInfo::FileInfo(const std::string file_path) 
        : file_path_{ bfs::absolute(bfs::path(file_path)).string() }, file_sz_{bfs::file_size(file_path_)} 
    {
        ;
    }
    
};