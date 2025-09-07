cmake_minimum_required(VERSION 3.12)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 0.0.${PATCH_VERSION})

project(bayan VERSION ${PROJECT_VERSION})

option(WITH_BOOST_TEST "Whether to build Boost test" ON)
option(WITH_GTEST "Whether to build Google test" ON)

configure_file(version.h.in version.h)

add_definitions(-D USE_PRETTY)

add_executable(bayan main.cpp bayan.cpp bayan_utils.cpp)
add_library(libbayan vers.cpp)

set_target_properties(bayan PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

set_target_properties(libbayan PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(bayan
    PRIVATE "${CMAKE_BINARY_DIR}"
)

target_include_directories(libbayan
    PRIVATE "${CMAKE_BINARY_DIR}"
)

find_package(Boost REQUIRED COMPONENTS program_options)
if( Boost_FOUND )
    message(status "** Boost Include: ${Boost_INCLUDE_DIR}")
    message(status "** Boost Libraries: ${Boost_LIBRARY_DIRS}")
    message(status "** Boost Libraries: ${Boost_LIBRARIES}")

    set_target_properties(bayan PROPERTIES
        COMPILE_DEFINITIONS BOOST_ALL_DYN_LINK
        INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR}
    )

    target_link_libraries(bayan PRIVATE
        ${Boost_LIBRARIES}
    )
endif()

target_link_libraries(bayan PRIVATE
    libbayan
)

if(WITH_BOOST_TEST)
    
    #if(WIN32)
        set (Boost_ROOT "C:/local/boost_1_87_0/") # Путь к библиотеке Boost
    #endif()

    find_package(Boost COMPONENTS unit_test_framework REQUIRED)
    add_executable(test_version test_version.cpp)

    set_target_properties(test_version PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )

    set_target_properties(test_version PROPERTIES
        COMPILE_DEFINITIONS BOOST_TEST_DYN_LINK
        INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR}
    )

    target_link_libraries(test_version
        ${Boost_LIBRARIES}
        libbayan
    )
endif()

if(WITH_GTEST)
    find_package(GTest  REQUIRED)
    add_executable(test_versiong test_versiong.cpp)
    add_executable(test_bayan test_bayan.cpp bayan.cpp)

    target_compile_definitions(test_bayan PUBLIC -DUSE_DBG_TRACE)

    set_target_properties(test_versiong PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )

    set_target_properties(test_bayan PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )

    target_link_libraries(test_versiong
        gtest
        libbayan
    )

    target_link_libraries(test_bayan
        gtest
        libbayan
    )
endif()

if (MSVC)
    target_compile_options(bayan PRIVATE
        /W4
    )
    target_compile_options(libbayan PRIVATE
        /W4
    )
    if(WITH_BOOST_TEST)
        target_compile_options(test_version PRIVATE
            /W4
        )
    endif()
    if(WITH_GTEST)
        target_compile_options(test_versiong PRIVATE
            /W4
        )
        target_compile_options(test_bayan PRIVATE
            /W4
        )
    endif()
else ()
    target_compile_options(bayan PRIVATE
        -Wall -Wextra -pedantic -Werror
    )
    target_compile_options(libbayan PRIVATE
        -Wall -Wextra -pedantic -Werror
    )
    if(WITH_BOOST_TEST)
        target_compile_options(test_version PRIVATE
            -Wall -Wextra -pedantic -Werror
        )
    endif()
    if(WITH_GTEST)
        target_compile_options(test_versiong PRIVATE
            -Wall -Wextra -pedantic -Werror
        )
        target_compile_options(test_bayan PRIVATE
            -Wall -Wextra -pedantic -Werror
        )
    endif()
endif()

install(TARGETS bayan RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT maxf1312@yandex.ru)
include(CPack)

if(WITH_BOOST_TEST)
    enable_testing()
    add_test(test_version test_version)
endif()

if(WITH_GTEST)
    #include(GoogleTest)
    enable_testing()
    add_test(test_versiong test_versiong)
    add_test(test_bayan test_bayan)
endif()
