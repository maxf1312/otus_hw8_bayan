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
    
    struct FileInfo;
    class FileInfoSet_t;
    using DupFileSet_t = std::set<FileInfo*>;
    using DupFilePtrSet_t = std::set<DupFileSet_t*>;

    /// @brief Основная структура информации о файле. Содержит список хэш-блоков для сранвнения, путь и размер файла. 
    struct FileInfo
    {
        /// @brief Тип хэш-кода пока что просто CRC32, потом будем делать разные функции хэширования
        using HashCode = std::vector<uint8_t>;

        /// @brief Тип списка хэшей - разделены по размеру блока
        using Hashes_t = std::vector<uint8_t>;
        
        /// @brief размер блока
        static file_sz_t block_sz_;

        /// @brief размер хэша
        static size_t hashcode_sz_;
        
        /// @brief полный путь к файлу
        std::string file_path_;

        /// @brief Список хэш-кодов
        Hashes_t hash_codes_;

        /// @brief владелец - набор файлов одного размера 
        FileInfoSet_t& owner_;

        using DupFileInfo_t = std::shared_ptr<DupFileSet_t>;
        DupFileInfo_t duplicates_; 

        /// @brief Конструктор
        /// @param file_path путь к файлу 
        /// @param owner владелец данной инфы о файле
        FileInfo(const std::string& file_path, FileInfoSet_t& owner);

        bool operator < (const FileInfo& rhs) const { return file_path_ < rhs.file_path_; }
        bool operator == (const FileInfo& rhs) const { return file_path_ == rhs.file_path_; }
        size_t block_count() const { return hash_codes_.size() / hashcode_sz_; }
        bool   check_duplicate(FileInfo& rhs, DupFilePtrSet_t& dup_fileptr_set);

        /// @brief Сравнивает массив хэешей this и rhs. Сравнение происходит по размеру минимального из двух массивов
        /// @param rhs  - правосторонний аргумент сравнения
        /// @return true, если min(count_blocks, rhs.count_blocks) хэшкодов равны в обеих массиввх
        bool is_hashes_eq(const FileInfo& rhs, bool blk_cnt_must_be_max = false) const;
        void read_and_hash_blocks(size_t up_to_blocks_count);
        HashCode hash_data(const uint8_t* data, size_t data_size);
    };

    /**
     * @brief Набор файлов одинакового размера, которые хранятся в двоичном дереве отсортированными по имени 
     *        (чтобы можно было сравнивать и читать с диска файлы, расположенные рядом в каталогах)   
     * 
     */
    class FileInfoSet_t 
    {
    public:
        using FileInfos_t = std::vector<FileInfo>;

        FileInfoSet_t(file_sz_t file_size = std::numeric_limits<boost::uintmax_t>::max()) 
        : file_sz_(file_size)
        { ; }
        
        void add_file(const std::string& file_path, file_sz_t file_sz = std::numeric_limits<boost::uintmax_t>::max())
        {
            FileInfo file_info{file_path, *this};
            if( std::numeric_limits<boost::uintmax_t>::max() == file_sz_ )
                file_sz_ = file_sz != std::numeric_limits<boost::uintmax_t>::max() ? file_sz : bfs::file_size(file_info.file_path_);
            file_set_.emplace_back(file_info);            
        }

        size_t max_block_count() const { return (file_sz_ + 1) / block_sz_; }

        /// @brief размер блока
        static file_sz_t block_sz() { return block_sz_; }

        /// @brief размер каждого файла в данном наборе        
        file_sz_t file_sz() const { return file_sz_; }

        size_t size() const { return file_set_.size(); }

        void  find_duplicates(DupFilePtrSet_t& dup_fileptr_set);

        FileInfos_t::const_iterator cbegin() const { return file_set_.cbegin(); }
        FileInfos_t::const_iterator cend() const  { return file_set_.cend(); }

        FileInfos_t::const_iterator begin() const { return file_set_.begin(); }
        FileInfos_t::const_iterator end() const  { return file_set_.end(); }

        bool operator == (const FileInfoSet_t& rhs) const { return  file_sz_ == rhs.file_sz_ && file_set_ == rhs.file_set_; }

    private:
        void find_duplicates_for_file(FileInfos_t::iterator file0, FileInfos_t::iterator file_end, DupFilePtrSet_t& dup_fileptr_set);

        /// @brief размер блока
        static file_sz_t block_sz_;

        /// @brief размер каждого файла в данном наборе        
        file_sz_t file_sz_;

        /// @brief Набор информации о файлах в данном наборе
        FileInfos_t file_set_;
    };
    
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
        FilesCollectionPtr const& files() const { return files_; }
        DupFilePtrSet_t const&    dup_filepointers() const { return dup_filepointers_; }
    private:
        void remove_single_file_sets();
        void find_duplicates_for_same_file_sizes(FileInfoSet_t& file_set, DupFilePtrSet_t& dup_fileptr_set);
        FilesCollectionPtr files_;
        DupFilePtrSet_t    dup_filepointers_;    
    };


} // otus_hw8

