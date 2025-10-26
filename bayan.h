#pragma once

#include <ctime>
#include <iostream>
#include <string>
#include <memory>
#include <set>
#include <unordered_map>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include "hashalgo.hpp"

namespace otus_hw8
{
    using std::istream;
    using std::ostream;

    using string_arr_t = std::vector<std::string>;
    using string_set_t = std::set<std::string>;
    using regex_arr_t = std::vector<boost::regex>;
    struct Options
    {
        bool show_help;
        string_arr_t dirs2scan;
        string_arr_t dirs2excl;
        size_t block_sz;
        std::string hash_func;
        size_t depth;
        size_t min_file_size;
        string_arr_t file_mask;
    };
    bool parse_command_line(int argc, const char *argv[], Options &parsed_options);

    namespace bfs = boost::filesystem;
    using file_sz_t = boost::uintmax_t;

    struct FileInfo;
    class FileInfoSet_t;
    using DupFileSet_t = std::set<FileInfo *>;
    using DupFilePtrSet_t = std::set<DupFileSet_t *>;

    /// @brief Основная структура информации о файле. Содержит список хэш-блоков для сранвнения, путь и размер файла.
    struct FileInfo
    {
        /// @brief Тип хэш-кода пока что просто CRC32, потом будем делать разные функции хэширования
        using HashCode = std::vector<uint8_t>;

        /// @brief Тип списка хэшей - разделены по размеру блока
        using Hashes_t = std::vector<uint8_t>;

        /// @brief полный путь к файлу
        std::string file_path_;

        /// @brief Список хэш-кодов
        Hashes_t hash_codes_;

        /// @brief владелец - набор файлов одного размера
        FileInfoSet_t &owner_;

        using DupFileInfo_t = std::shared_ptr<DupFileSet_t>;
        /// @brief множество указателей на дубликаты
        DupFileInfo_t duplicates_;

        /// @brief Конструктор
        /// @param file_path путь к файлу
        /// @param owner владелец данной инфы о файле
        FileInfo(const std::string &file_path, FileInfoSet_t &owner);

        /// @brief размер блока
        static file_sz_t block_sz();

        /// @brief размер хэш-кода
        static size_t hashcode_sz();

        bool operator<(const FileInfo &rhs) const { return file_path_ < rhs.file_path_; }
        bool operator==(const FileInfo &rhs) const { return file_path_ == rhs.file_path_; }

        /// @brief актуальное число блоков
        size_t block_count() const { return hash_codes_.size() / hashcode_sz(); }

        /// @brief Проверить, является *this и rhs дубликатами, результат сохранить в dup_fileptr_set
        bool check_duplicate(FileInfo &rhs, DupFilePtrSet_t &dup_fileptr_set);

        /// @brief Сравнивает массив хэешей this и rhs. Сравнение происходит по размеру минимального из двух массивов
        /// @param rhs  - правосторонний аргумент сравнения
        /// @return true, если min(count_blocks, rhs.count_blocks) хэшкодов равны в обеих массиввх
        bool is_hashes_eq(const FileInfo &rhs, bool blk_cnt_must_be_max = false) const;

        /// @brief Читает и хэширует блоки вплоть до указанного количества блоков
        /// @param up_to_blocks_count
        void read_and_hash_blocks(size_t up_to_blocks_count);

        /// @brief Производит хэширование данных, вызывает установленную у владельца стартегию
        /// @param data  - адрес данных
        /// @param data_size  - размер данных
        /// @return - вектор байт с хэш-кодом данных
        HashCode hash_data(const uint8_t *data, size_t data_size);
    };

    /**
     * @brief Набор файлов одинакового размера
     *
     */
    class FileInfoSet_t
    {
    public:
        using FileInfos_t = std::vector<FileInfo>;

        FileInfoSet_t(file_sz_t file_size = std::numeric_limits<boost::uintmax_t>::max())
            : file_sz_(file_size)
        {
            ;
        }

        /// @brief добавляет файл в набор, инициализирует размер, если он был нетронутым
        /// @param file_path - путь к файлу
        /// @param file_sz  - размер файла
        void add_file(const std::string &file_path, file_sz_t file_sz = std::numeric_limits<boost::uintmax_t>::max())
        {
            FileInfo file_info{file_path, *this};
            if (std::numeric_limits<boost::uintmax_t>::max() == file_sz_)
                file_sz_ = file_sz != std::numeric_limits<boost::uintmax_t>::max() ? file_sz : bfs::file_size(file_info.file_path_);
            file_set_.emplace_back(file_info);
        }

        /// @brief максимальное число блоков для файлов в наборе
        /// @return
        size_t max_block_count() const { return (file_sz_ + 1) / block_sz_; }

        /// @brief размер блока
        static file_sz_t block_sz() { return block_sz_; }

        /// @brief размер хэш-кода
        static size_t hashcode_sz() { return hashcode_sz_; }

        /// @brief размер каждого файла в данном наборе
        file_sz_t file_sz() const { return file_sz_; }

        /// @brief актуальное число файлов в наборе
        size_t size() const { return file_set_.size(); }

        /// @brief найти дубликаты среди файлов в наборе.
        /// @param dup_fileptr_set - множество указателей на метаданные дубликатов
        void find_duplicates(DupFilePtrSet_t &dup_fileptr_set);

        FileInfos_t::const_iterator cbegin() const { return file_set_.cbegin(); }
        FileInfos_t::const_iterator cend() const { return file_set_.cend(); }

        FileInfos_t::const_iterator begin() const { return file_set_.begin(); }
        FileInfos_t::const_iterator end() const { return file_set_.end(); }

        bool operator==(const FileInfoSet_t &rhs) const { return file_sz_ == rhs.file_sz_ && file_set_ == rhs.file_set_; }

        /// @brief установить стратегию хэширования
        /// @param hash_func - функция хэширования
        void set_hash_func(HashFunction const &hash_func) { hash_func_ = hash_func; }
        HashFunction get_hash_func() const { return hash_func_; }

        /// @brief установить размер блока
        /// @param blk_sz
        static void set_block_sz(file_sz_t blk_sz);

        /// @brief установить размер хэша
        /// @param hash_sz
        static void set_hash_sz(file_sz_t hash_sz);

    private:
        /// @brief поиск дубликатов для файла *file0, внутри интервала (file0, file_end), результаты сохранить в dup_fileptr_set
        /// @param file0 - файл, который сравнивается с остальными в интервале (file0, file_end)
        /// @param file_end - конец интервала
        /// @param dup_fileptr_set  -  набор результатов поиска
        void find_duplicates_for_file(FileInfos_t::iterator file0, FileInfos_t::iterator file_end, DupFilePtrSet_t &dup_fileptr_set);

        /// @brief размер блока
        static file_sz_t block_sz_;

        /// @brief размер хэша
        static size_t hashcode_sz_;

        /// @brief размер каждого файла в данном наборе
        file_sz_t file_sz_;

        /// @brief Набор информации о файлах в данном наборе
        FileInfos_t file_set_;

        /// @brief функция хэширования
        HashFunction hash_func_;
    };

    /**
     * @brief Основная коллекция для хранения информации о файлах.
     *        Рассматриваются файлы одинакового размера, которые хранятся в двоичном дереве отсортированными по имени
     *        (чтобы можно было сравнивать и читать с диска файлы, расположенные рядом в каталогах)
     *
     */
    using FilesCollection_t = std::unordered_map<file_sz_t, FileInfoSet_t>;
    using FilesCollectionPtr = std::shared_ptr<FilesCollection_t>;

    /// @brief Класс для поиска файлов в указанной директории. Помещает найденные файлы в dest_files_, сразу раделяя их по размеру.
    class FileFinder
    {
    public:
        FileFinder(std::string const &dir_path, size_t depth, size_t min_file_sz, FilesCollectionPtr dest_files,
                   std::shared_ptr<string_set_t> const &excl_dirs = {},
                   std::shared_ptr<regex_arr_t> const &file_mask_re = {})
            : dir_path_(dir_path),
              dirs_to_excl_(excl_dirs),
              file_mask_re_(file_mask_re),
              depth_(depth),
              min_file_sz_(min_file_sz),
              dest_files_(dest_files)
        {
        }

        /// @brief основной поиск файлов
        void find_files() const;

    private:
        /// @brief true, если файл file_path удовлетворяет маске или маска пустая
        bool is_file_mask_match(const std::string &file_path) const;

        /// @brief путь к сканируемой директории
        std::string dir_path_;

        /// @brief набор директорий, исключаемых из сканирования
        std::shared_ptr<string_set_t> dirs_to_excl_;

        /// @brief набор регулярок для маски файлов
        std::shared_ptr<regex_arr_t> file_mask_re_;

        /// @brief глубина сканирования
        size_t depth_;

        /// @brief минимальный размер файла
        size_t min_file_sz_;

        /// @brief результирующий набор из множеств одного размера
        FilesCollectionPtr dest_files_;
    };

    /// @brief Основной класс для обработки файлов, использует FileFinder для заполнения списка файлов по директории
    class FileDupSearcher
    {
    public:
        FileDupSearcher(HashFunction const &hash_func = create_hash_function(HashFunctionType::HashDumb));

        /// @brief добавить файлы из каталога в список для сканирования
        void add_dir(const std::string &dir_path, size_t depth = 0, file_sz_t min_size = 1, const string_arr_t &dirs_to_excl = {}, const string_arr_t &file_mask = {});

        /// @brief найти дубликаты в добалвенных ранее через add_dir() ктаалогах
        void find_duplicates();

        /// @brief найденые для сканирования файлы
        FilesCollectionPtr const &files() const { return files_; }

        /// @brief найденные дубликаты
        DupFilePtrSet_t const &dup_filepointers() const { return dup_filepointers_; }

        /// @brief утилита для получения строки регулярного выражения из маски файла
        static std::string reg_ex_from_file_mask(const std::string &file_mask);

    private:
        void remove_single_file_sets();
        void find_duplicates_for_same_file_sizes(FileInfoSet_t &file_set, DupFilePtrSet_t &dup_fileptr_set);
        FilesCollectionPtr files_;
        DupFilePtrSet_t dup_filepointers_;
        HashFunction hash_func_;
    };

} // otus_hw8
