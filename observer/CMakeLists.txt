set(observer_test_srcs
    "observer.h"
    "subject.h"
    "observer_test.cc")

include_directories(${CMAKE_SOURCE_DIR}/third_party/googletest/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/googlemock/include)

add_executable(observer_test ${observer_test_srcs})
target_link_libraries(observer_test gtest gmock)
set_target_properties(observer_test PROPERTIES FOLDER Observer)
