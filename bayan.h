#pragma once

#include <ctime>
#include <iostream>
#include <string>
#include <memory>
#include <queue>
#include <boost/filesystem.hpp>
//#include <boost/algorithm>
#include <set>
#include <unordered_map>


namespace otus_hw8{
    using std::istream;
    using std::ostream;
    struct Options
    {
        bool   show_help;
        size_t cmd_chunk_sz;
    };
    bool parse_command_line(int argc, const char* argv[], Options& parsed_options);

    namespace bfs = boost::filesystem;
    using file_sz_t = boost::uintmax_t;
    

    struct FileInfo
    {
        /// @brief Тип хэш-кода пока что просто CRC32, потом будем делать разные функции хэширования
        using HashCode = uint32_t;

        /// @brief Тип списка хэшей
        using Hashes_t = std::vector<HashCode>;
        
        static file_sz_t block_sz_;
        std::string file_path_;
        file_sz_t file_sz_;
        Hashes_t hash_codes_;

        FileInfo(const std::string file_path);
        bool operator < (const FileInfo& rhs) const { return file_path_ < rhs.file_path_; }
        size_t block_count() const { return hash_codes_.size(); }
        bool is_hashes_eq(const FileInfo& rhs) const;  
    };

    /**
         * @brief Набор файлов одинакового размера, которые хранятся в двоичном дереве отсортированными по имени 
         *        (чтобы можно было сравнивать и читать с диска файлы, расположенные рядом в каталогах)   
         * 
         */
    using FileInfoSet_t = std::set<FileInfo>;
    
    /**
         * @brief Основная коллекция для хранения информации о файлах. 
         *        Рассматриваются файлы одинакового размера, которые хранятся в двоичном дереве отсортированными по имени 
         *        (чтобы можно было сравнивать и читать с диска файлы, расположенные рядом в каталогах)   
         * 
         */
    using FilesCollection_t = std::unordered_map<file_sz_t, FileInfoSet_t>;


} // otus_hw8

