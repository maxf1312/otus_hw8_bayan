#include <gtest/gtest.h>
#include <sstream>
#include <list>
#include <tuple>
#include <boost/regex.hpp>

//#include <fmt/format.h>
//#include <format>
#ifndef __PRETTY_FUNCTION__
#include "pretty.h"
#endif
#include "bayan_internal.h"

using namespace otus_hw8;

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

using namespace std::literals::string_literals;

void explore_boost_path(bfs::path p)
{
    using namespace std;
    using namespace bfs;

    auto say_what = [](bool b) -> const char * { return b ? "true" : "false"; };
    cout  <<  p << std::endl;
    cout  <<  "  make_preferred()-----: " << p.make_preferred() << "\n";

    cout << "\nelements:\n";
    for (auto element : p)
        cout << "  " << element << '\n';

    cout  <<  "\nobservers, native format:" << endl;
    # ifdef BOOST_POSIX_API
    cout  <<  "  native()-------------: " << p.native() << endl;
    cout  <<  "  c_str()--------------: " << p.c_str() << endl;
    # else  // BOOST_WINDOWS_API
    wcout << L"  native()-------------: " << p.native() << endl;
    wcout << L"  c_str()--------------: " << p.c_str() << endl;
    # endif
    cout  <<  "  string()-------------: " << p.string() << endl;
    wcout << L"  wstring()------------: " << p.wstring() << endl;

    cout  <<  "\nobservers, generic format:\n";
    cout  <<  "  generic_string()-----: " << p.generic_string() << endl;
    wcout << L"  generic_wstring()----: " << p.generic_wstring() << endl;

    cout  <<  "\ndecomposition:\n";
    cout  <<  "  root_name()----------: " << p.root_name() << '\n';
    cout  <<  "  root_directory()-----: " << p.root_directory() << '\n';
    cout  <<  "  root_path()----------: " << p.root_path() << '\n';
    cout  <<  "  relative_path()------: " << p.relative_path() << '\n';
    cout  <<  "  parent_path()--------: " << p.parent_path() << '\n';
    cout  <<  "  filename()-----------: " << p.filename() << '\n';
    cout  <<  "  stem()---------------: " << p.stem() << '\n';
    cout  <<  "  extension()----------: " << p.extension() << '\n';

    cout  <<  "\nquery:\n";
    cout  <<  "  empty()--------------: " << say_what(p.empty()) << '\n';
    cout  <<  "  is_absolute()--------: " << say_what(p.is_absolute()) << '\n';
    cout  <<  "  has_root_name()------: " << say_what(p.has_root_name()) << '\n';
    cout  <<  "  has_root_directory()-: " << say_what(p.has_root_directory()) << '\n';
    cout  <<  "  has_root_path()------: " << say_what(p.has_root_path()) << '\n';
    cout  <<  "  has_relative_path()--: " << say_what(p.has_relative_path()) << '\n';
    cout  <<  "  has_parent_path()----: " << say_what(p.has_parent_path()) << '\n';
    cout  <<  "  has_filename()-------: " << say_what(p.has_filename()) << '\n';
    cout  <<  "  has_stem()-----------: " << say_what(p.has_stem()) << '\n';
    cout  <<  "  has_extension()------: " << say_what(p.has_extension()) << '\n';
}

TEST(test_bayan, test_file_collect)
{
    bfs::path p(__FILE__);
    auto parent_dir = p.parent_path();
    auto test_data_dir = parent_dir / "test_data/";
    EXPECT_EQ(test_data_dir.wstring(), parent_dir.wstring() + L"/test_data/" );
    FilesCollection_t files;
    constexpr const size_t Files_count = 10;
    for(size_t i = 1; i <= Files_count; ++i)
    {
        char file_nm[32];
        snprintf(file_nm, sizeof(file_nm)/sizeof(file_nm[0]), "test%02lu.txt", i);
        auto file_path = test_data_dir / file_nm;
        std::cout << file_path << std::endl;
        
        file_sz_t file_sz = bfs::file_size(file_path);
        auto& file_set = files[file_sz];
        file_set.add_file(file_path.string()); 
    }

    std::set<std::string>  etalon_set;
    for( const auto& [sz, file_set]: files )
    {
        std::cout << "sz: " << sz << std::endl;
        for( auto const& file_inf: file_set )
        {
            std::cout << "\tfile_info: " << file_inf.file_path_ << std::endl;
            etalon_set.insert(file_inf.file_path_);
        }
    }

    
    auto files2 = std::make_shared<FilesCollection_t>();
    FileFinder finder(test_data_dir.string(), 0, 1, files2);
    finder.find_files();

    std::set<std::string>  testing_set;
    for( const auto& [sz, file_set]: *files2 )
    {
        std::transform(file_set.begin(), file_set.end(), std::inserter(testing_set, testing_set.begin()), [](const auto& p_fi) -> std::string { return p_fi.file_path_; } );
    }
    EXPECT_EQ(etalon_set, testing_set) << "Filecollections are not equal!";

    for( const auto& [sz, file_set]: *files2 )
    {
        std::cout << "sz2: " << sz << std::endl;
        for( auto const& file_inf: file_set )
        std::cout << "\tfile_info2: " << file_inf.file_path_ << std::endl;
    
    }
}

TEST(test_bayan, test_file_collect_depth)
{
    bfs::path p(__FILE__);
    auto parent_dir = p.parent_path();
    auto test_data_dir = parent_dir / "test_data/";
    EXPECT_EQ(test_data_dir.wstring(), parent_dir.wstring() + L"/test_data/" );
    FilesCollection_t files;

    auto scan_dir = [](bfs::path const &dir_to_scan, FilesCollection_t& files)
    {
        for (bfs::recursive_directory_iterator cur_file_it(dir_to_scan); cur_file_it != end(cur_file_it) ; ++cur_file_it)
        {
            std::cout << cur_file_it->path() << std::endl;
            if( cur_file_it->is_regular_file() ){
                file_sz_t file_sz = bfs::file_size(cur_file_it->path());
                auto &file_set = files[file_sz];
                file_set.add_file(cur_file_it->path().string());
            }
        }
    };

    scan_dir(test_data_dir, files);

    std::set<std::string> etalon_set;
    for( const auto& [sz, file_set]: files )
    {
        std::cout << "sz: " << sz << std::endl;
        for( auto const& file_inf: file_set )
        {
            std::cout << "\tfile_info: " << file_inf.file_path_ << std::endl;
            etalon_set.insert(file_inf.file_path_);
        }
    }

    
    auto files2 = std::make_shared<FilesCollection_t>();
    FileFinder finder(test_data_dir.string(), 2, 1, files2);
    finder.find_files();

    std::set<std::string>  testing_set;
    for( const auto& [sz, file_set]: *files2 )
    {
        std::transform(file_set.begin(), file_set.end(), std::inserter(testing_set, testing_set.begin()), [](const auto& p_fi) -> std::string { return p_fi.file_path_; } );
    }
    EXPECT_EQ(etalon_set, testing_set) << "Filecollections are not equal!";

    for( const auto& [sz, file_set]: *files2 )
    {
        std::cout << "sz2: " << sz << std::endl;
        for( auto const& file_inf: file_set )
        std::cout << "\tfile_info2: " << file_inf.file_path_ << std::endl;
    
    }
}

TEST(test_bayan, test_find_dup)
{
    bfs::path p(__FILE__);
    auto parent_dir = p.parent_path();
    auto test_data_dir = parent_dir / "test_data/";
    EXPECT_EQ(test_data_dir.wstring(), parent_dir.wstring() + L"/test_data/" );

    std::set<std::string>  etalon_set;
    for(size_t i : {1, 2, 3, 5, 6, 8, 9, 10} )
    {
        char file_nm[32];
        snprintf(file_nm, sizeof(file_nm)/sizeof(file_nm[0]), "test%02lu.txt", i);
        auto file_path = test_data_dir / file_nm;
        etalon_set.insert(file_path.string());
    }
    
    std::set<std::string>  result_set;
    FileDupSearcher searcher;
    searcher.add_dir(test_data_dir.string());
    searcher.find_duplicates();
    for( const auto file_ptr_set: searcher.dup_filepointers() )
    {
        for( const auto file_info_ptr : *file_ptr_set )
        {
            result_set.insert(file_info_ptr->file_path_);
            std::cout << file_info_ptr->file_path_ << std::endl;
        }
        std::cout << std::endl;
    }

    EXPECT_EQ(etalon_set, result_set) << "Result and etalon sets are not equal!";
}

TEST(test_bayan, test_find_dup_crc16)
{
    bfs::path p(__FILE__);
    auto parent_dir = p.parent_path();
    auto test_data_dir = parent_dir / "test_data/";
    EXPECT_EQ(test_data_dir.wstring(), parent_dir.wstring() + L"/test_data/" );

    std::set<std::string>  etalon_set;
    for(size_t i : {1, 2, 3, 5, 6, 8, 9, 10} )
    {
        char file_nm[32];
        snprintf(file_nm, sizeof(file_nm)/sizeof(file_nm[0]), "test%02lu.txt", i);
        auto file_path = test_data_dir / file_nm;
        etalon_set.insert(file_path.string());
    }
    
    std::set<std::string>  result_set;
    FileDupSearcher searcher(create_hash_function("crc16"));
    FileInfoSet_t::set_hash_sz(get_hash_bytes_len(HashFunctionType::HashCRC16));
    searcher.add_dir(test_data_dir.string());
    searcher.find_duplicates();
    for( const auto file_ptr_set: searcher.dup_filepointers() )
    {
        for( const auto file_info_ptr : *file_ptr_set )
        {
            result_set.insert(file_info_ptr->file_path_);
            std::cout << file_info_ptr->file_path_ << std::endl;
        }
        std::cout << std::endl;
    }

    EXPECT_EQ(etalon_set, result_set) << "Result and etalon sets are not equal!";
}

TEST(test_bayan, test_find_dup_md5)
{
    bfs::path p(__FILE__);
    auto parent_dir = p.parent_path();
    auto test_data_dir = parent_dir / "test_data/";
    EXPECT_EQ(test_data_dir.wstring(), parent_dir.wstring() + L"/test_data/" );

    std::set<std::string>  etalon_set;
    for(size_t i : {1, 2, 3, 5, 6, 8, 9, 10} )
    {
        char file_nm[32];
        snprintf(file_nm, sizeof(file_nm)/sizeof(file_nm[0]), "test%02lu.txt", i);
        auto file_path = test_data_dir / file_nm;
        etalon_set.insert(file_path.string());
    }
    
    std::set<std::string>  result_set;
    FileDupSearcher searcher(create_hash_function("md5"));
    FileInfoSet_t::set_hash_sz(get_hash_bytes_len(HashFunctionType::HashMD5));
    searcher.add_dir(test_data_dir.string());
    searcher.find_duplicates();
    for( const auto file_ptr_set: searcher.dup_filepointers() )
    {
        for( const auto file_info_ptr : *file_ptr_set )
        {
            result_set.insert(file_info_ptr->file_path_);
            std::cout << file_info_ptr->file_path_ << std::endl;
        }
        std::cout << std::endl;
    }

    EXPECT_EQ(etalon_set, result_set) << "Result and etalon sets are not equal!";
}

TEST(test_bayan, test_find_dup_sz)
{
    bfs::path p(__FILE__);
    auto parent_dir = p.parent_path();
    auto test_data_dir = parent_dir / "test_data/";
    EXPECT_EQ(test_data_dir.wstring(), parent_dir.wstring() + L"/test_data/" );

    std::set<std::string>  etalon_set;
    for(size_t i : {3, 5} )
    {
        char file_nm[32];
        snprintf(file_nm, sizeof(file_nm)/sizeof(file_nm[0]), "test%02lu.txt", i);
        auto file_path = test_data_dir / file_nm;
        etalon_set.insert(file_path.string());
    }
    
    constexpr const file_sz_t min_size = 15; 
    std::set<std::string>  result_set;
    FileDupSearcher searcher(create_hash_function("md5"));
    FileInfoSet_t::set_hash_sz(get_hash_bytes_len(HashFunctionType::HashMD5));
    searcher.add_dir(test_data_dir.string(), 0, min_size);
    searcher.find_duplicates();
    for( const auto file_ptr_set: searcher.dup_filepointers() )
    {
        for( const auto file_info_ptr : *file_ptr_set )
        {
            result_set.insert(file_info_ptr->file_path_);
            std::cout << file_info_ptr->file_path_ << std::endl;
        }
        std::cout << std::endl;
    }

    EXPECT_EQ(etalon_set, result_set) << "Result and etalon sets are not equal!";
}


TEST(test_bayan, test_find_files_mask)
{
    bfs::path p(__FILE__);
    auto parent_dir = p.parent_path();
    auto test_data_dir = parent_dir / "test_data/";
    EXPECT_EQ(test_data_dir.wstring(), parent_dir.wstring() + L"/test_data/" );

    auto file_mask = "/test_data/t*??.*t"s;

    auto file_mask_re = FileDupSearcher::reg_ex_from_file_mask(file_mask);
    std::cout << "file_mask: " << file_mask << ", file_mask_re: " << file_mask_re << std::endl;
        
    auto reg_ex_s = file_mask_re;
    boost::regex re(reg_ex_s, boost::regex::basic|boost::regex::icase);

    std::cout << reg_ex_s << std::endl;

    auto scaned = "/mnt/test_data/test78.text"s;
    
    boost::smatch match_res;
    bool re_res = boost::regex_search(scaned, match_res, re);
    if(re_res)
        std::cout << "p: " << match_res.position() << ", l: "<< match_res.length() << std::endl;
    EXPECT_TRUE(re_res) << "Regexp " << reg_ex_s << " not found in " << scaned;

    std::set<std::string>  etalon_set;
    for(size_t i : {1, 2, 3, 5, 6, 8, 9, 10} )
    {
        char file_nm[32];
        snprintf(file_nm, sizeof(file_nm)/sizeof(file_nm[0]), "test%02lu.txt", i);
        auto file_path = test_data_dir / file_nm;
        etalon_set.insert(file_path.string());
    }
    std::set<std::string>  result_set;

    //EXPECT_EQ(etalon_set, result_set) << "Result and etalon sets are not equal!";
}

TEST(test_bayan, test_find_dup_mask)
{
    bfs::path p(__FILE__);
    auto parent_dir = p.parent_path();
    auto test_data_dir = parent_dir / "test_data/";
    EXPECT_EQ(test_data_dir.wstring(), parent_dir.wstring() + L"/test_data/" );

    std::set<std::string>  etalon_set;
    for(size_t i : {2, 6, 8} )
    {
        char file_nm[32];
        snprintf(file_nm, sizeof(file_nm)/sizeof(file_nm[0]), "test%02lu.txt", i);
        auto file_path = test_data_dir / file_nm;
        etalon_set.insert(file_path.string());
    }
    
    std::set<std::string>  result_set;
    FileDupSearcher searcher;
    FileInfoSet_t::set_hash_sz(get_hash_bytes_len(HashFunctionType::HashDumb));
    searcher.add_dir(test_data_dir.string(), 0, 1, {}, {"tes*2.*"s, "tes*6.*"s, "tes*8.*"s});
    searcher.find_duplicates();
    for( const auto file_ptr_set: searcher.dup_filepointers() )
    {
        for( const auto file_info_ptr : *file_ptr_set )
        {
            result_set.insert(file_info_ptr->file_path_);
            std::cout << file_info_ptr->file_path_ << std::endl;
        }
        std::cout << std::endl;
    }

    EXPECT_EQ(etalon_set, result_set) << "Result and etalon sets are not equal!";
}
