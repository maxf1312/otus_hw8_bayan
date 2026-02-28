#include <iostream>
#include <boost/program_options.hpp>
#include "bayan.h"

using namespace std::literals::string_literals;

namespace otus_hw8
{
    namespace po = boost::program_options;

    void show_help(po::options_description const &desc)
    {
        std::cout << desc << std::endl;
    }

    bool parse_command_line(int argc, const char *argv[], Options &parsed_options)
    {
        constexpr const char *const OPTION_NAME_HELP = "help";
        constexpr const char *const OPTION_NAME_BLOCK_SIZE = "block-size";
        constexpr const char *const OPTION_NAME_HASH_FUNC = "hash-func";
        constexpr const char *const OPTION_NAME_DEPTH = "depth";
        constexpr const char *const OPTION_NAME_MIN_FILE_SZ = "min-file-size";
        constexpr const char *const OPTION_NAME_DIR_TO_SCAN = "dir-to-scan";
        constexpr const char *const OPTION_NAME_DIR_TO_EXCL = "dir-to-excl";
        constexpr const char *const OPTION_NAME_FILE_MASK = "file-mask";

        parsed_options = {false, {}, {}, 0, {}, 0, 1, {}};

        auto check_size = [](const size_t &sz, const size_t min_sz, const char *opt_nm)
        {
            if (sz < min_sz)
                throw po::invalid_option_value(opt_nm);
        };
        auto check_hash_func = [](const std::string &hf_nm)
        {
            if (hash_function_type(hf_nm) == HashFunctionType::HashUnknown)
                throw po::invalid_option_value(OPTION_NAME_HASH_FUNC);
        };

        auto check_dir = [](const string_arr_t &dirs)
        {
            std::ignore = dirs;
        };

        po::options_description desc("Аргументы командной строки");
        desc.add_options()((OPTION_NAME_HELP + ",h"s).c_str(), po::bool_switch(&parsed_options.show_help), "Отображение справки")((OPTION_NAME_DIR_TO_SCAN + ",d"s).c_str(), po::value<string_arr_t>(&parsed_options.dirs2scan)->multitoken()->zero_tokens()->composing()->default_value({"."s}, ".")->notifier(check_dir),
                                                                                                                                  "Одна или несколько директорий для сканирования")((OPTION_NAME_DIR_TO_EXCL + ",e"s).c_str(), po::value<string_arr_t>(&parsed_options.dirs2excl)->multitoken()->zero_tokens()->composing()->notifier(check_dir),
                                                                                                                                                                                    "Исключаемые из сканирования директории")((OPTION_NAME_FILE_MASK + ",m"s).c_str(), po::value<string_arr_t>(&parsed_options.file_mask)->multitoken()->zero_tokens()->composing()->notifier(check_dir),
                                                                                                                                                                                                                              "Маски имен файлов для поиска и сравнения")((OPTION_NAME_BLOCK_SIZE + ",b"s).c_str(), po::value<size_t>(&parsed_options.block_sz)->default_value(1024, "1024")->notifier(std::bind(check_size, std::placeholders::_1, 1, OPTION_NAME_BLOCK_SIZE)), "Размер блока сравнения, байты")((OPTION_NAME_HASH_FUNC + ",f"s).c_str(), po::value<std::string>(&parsed_options.hash_func)->default_value("crc32"s, "crc32")->notifier(check_hash_func), "Функция хэширования")((OPTION_NAME_DEPTH + ",n"s).c_str(), po::value<size_t>(&parsed_options.depth)->default_value(0, "0")->notifier(std::bind(check_size, std::placeholders::_1, 0, OPTION_NAME_DEPTH)), "Глубина сканирования вложенных директорий")((OPTION_NAME_MIN_FILE_SZ + ",s"s).c_str(), po::value<size_t>(&parsed_options.min_file_size)->default_value(1, "1")->notifier(std::bind(check_size, std::placeholders::_1, 1, OPTION_NAME_MIN_FILE_SZ)), "Минимальный размер файла, байты");

        po::positional_options_description pos_desc;
        pos_desc.add(OPTION_NAME_DIR_TO_SCAN, -1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(pos_desc).run(), vm);
        po::notify(vm);

        size_t sz = vm.size();
        bool not_need_exit = true;
        if (sz < 2 || !vm.count(OPTION_NAME_DIR_TO_SCAN))
            parsed_options.show_help = true;

        if (parsed_options.show_help)
            show_help(desc),
                not_need_exit = false;

        return not_need_exit;
    }

}
