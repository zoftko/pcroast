include(CTest)

add_executable(test_uid test_uid.cpp)
target_link_libraries(test_uid uid gtest_main)
add_test(uid test_uid)
