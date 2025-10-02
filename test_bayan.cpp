#include <gtest/gtest.h>
#include <sstream>
#include <list>
#include <tuple>
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

    for( const auto& [sz, file_set]: files )
    {
        std::cout << "sz: " << sz << std::endl;
        for( auto const& file_inf: file_set )
        std::cout << "\tfile_info: " << file_inf.file_path_ << std::endl;
    }

    
    auto files2 = std::make_shared<FilesCollection_t>();
    FileFinder finder(test_data_dir.string(), 0, 1, files2);
    finder.find_files();
    EXPECT_EQ(files, *files2) << "Filecollections are not equal!";

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
   
    FileDupSearcher searcher;
    searcher.add_dir(test_data_dir.string());
    searcher.find_duplicates();

    for( const auto& [_, file_set]: *searcher.files() )
    {
        for( const auto file_ptr_set: file_set.dup_fileptr_set() )
        {
            for( const auto file_info_ptr : *file_ptr_set )
                std::cout << file_info_ptr->file_path_ << std::endl;
            std::cout << std::endl;
        }
    }


    //    EXPECT_EQ(files, *files2) << "Filecollections are not equal!";

}


