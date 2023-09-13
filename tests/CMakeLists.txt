include(CTest)

add_executable(test_max6675 test_max6675.cpp)
target_link_libraries(test_max6675 max6675 gtest_main)
add_test(max6675 test_max6675)

add_executable(test_beeper test_beeper.cpp)
target_link_libraries(test_beeper beeper gtest_main)
add_test(beeper test_beeper)
