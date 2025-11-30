if(MRN_BUILD_TESTS)
    # 查找 Google Test
    find_package(GTest QUIET)
    
    if(GTest_FOUND)
        # 添加测试可执行文件
        add_executable(mrn_tests
            test_main.cpp
            test_compression.cpp
            test_algorithms.cpp
            test_file_io.cpp
        )
        
        target_link_libraries(mrn_tests PRIVATE GTest::gtest GTest::gtest_main mrn)
        target_include_directories(mrn_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
        
        # 添加测试
        add_test(NAME CompressionTests COMMAND mrn_tests)
        add_test(NAME AlgorithmTests COMMAND mrn_tests --gtest_filter="Algorithm*")
        add_test(NAME IOTests COMMAND mrn_tests --gtest_filter="IO*")
    else()
        message(WARNING "GTest not found, tests will not be built")
    endif()
endif()
