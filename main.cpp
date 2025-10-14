#include <iostream>

#include "vers.h"
#include "bayan.h"

using namespace std::literals::string_literals;


int main(int argc, char const* argv[]) 
{
	using namespace otus_hw8;
	try
	{
		Options options;
		if (!parse_command_line(argc, argv, options))
			return 1;
		
		bfs::path work_dir(".");
		std::cout << "work_dir: " << work_dir << std::endl;
		auto files = std::make_shared<FilesCollection_t>();
    	FileFinder finder(work_dir.string(), 0, 1, files);
    	finder.find_files();
		HashFunctionType hash_type = hash_function_type(options.hash_func);		
 		FileDupSearcher searcher(create_hash_function(hash_type));
		FileInfoSet_t::set_hash_sz(get_hash_bytes_len(hash_type));
    	searcher.add_dir(work_dir.string());
    	searcher.find_duplicates();
    	for( const auto file_ptr_set: searcher.dup_filepointers() )
    	{
     		for( const auto file_info_ptr : *file_ptr_set )
      	  	{
        	    std::cout << file_info_ptr->file_path_ << std::endl;
        	}
       	 	std::cout << std::endl;
    	}
	}	
	catch(const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
