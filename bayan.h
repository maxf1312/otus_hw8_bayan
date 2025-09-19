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
    
    /// @brief Основная структура информации о файле. Содержит список хэш-блоков для сранвнения, путь и размер файла. 
    struct FileInfo
    {
        /// @brief Тип хэш-кода пока что просто CRC32, потом будем делать разные функции хэширования
        using HashCode = uint32_t;

        /// @brief Тип списка хэшей (TODO сделать классом с методами сравнения и чтения )
        using Hashes_t = std::vector<HashCode>;
        
        /// @brief размер блока
        static file_sz_t block_sz_;
        
        /// @brief полный путь к файлу
        std::string file_path_;

        /// @brief размер файла
        file_sz_t file_sz_;

        /// @brief Список хэш-кодов
        Hashes_t hash_codes_;

        FileInfo(const std::string file_path);
        bool operator < (const FileInfo& rhs) const { return file_path_ < rhs.file_path_; }
        bool operator == (const FileInfo& rhs) const { return file_path_ == rhs.file_path_; }
        size_t block_count() const { return hash_codes_.size(); }
        bool is_hashes_eq(const FileInfo& rhs) const;
        void read_and_hash_blocks(size_t up_to_blocks_count);
        HashCode hash_data(const uint8_t* data, size_t data_size);
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
    using FilesCollectionPtr = std::shared_ptr<FilesCollection_t>;

    class FileFinder
    {
    public:
        FileFinder(std::string const& dir_path, size_t depth, size_t min_file_sz, FilesCollectionPtr dest_files)
        :   dir_path_(dir_path),
            depth_(depth),
            min_file_sz_(min_file_sz),
            dest_files_(dest_files) 
        {
            std::ignore = depth_;
            std::ignore = min_file_sz_;
        }
        void find_files();
    private:
        std::string dir_path_;
        size_t depth_;
        size_t min_file_sz_;
        FilesCollectionPtr dest_files_;
    };

    /// @brief Основной класс для обработки файлов, использует FileFinder для заполнения списка файлов по директории
    class FileDupSearcher
    {
    public:
        FileDupSearcher();
        void add_dir(const std::string& dir_path);
        void find_duplicates();            
    private:
        void find_duplicates_for_same_file_sizes(FileInfoSet_t& file_set);
        void find_duplicates_for_file(FileInfoSet_t::iterator file0, FileInfoSet_t::iterator file_end);
        FilesCollectionPtr files_;    
    };


} // otus_hw8

