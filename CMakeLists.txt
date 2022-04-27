cmake_minimum_required(VERSION 3.16)
project(cadmium)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_library(cadmium INTERFACE)
target_include_directories(cadmium INTERFACE include/)

FILE(GLOB Examples RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} example/*/main_*.cpp)
foreach(exampleSrc ${Examples})
    get_filename_component(exampleName ${exampleSrc} NAME_WE)
    add_executable(${exampleName} ${exampleSrc})
    target_link_libraries(${exampleName} cadmium)
endforeach(exampleSrc)

# Option to compile examples with flat coordinator
# It won't execute unless compiled with FLAT flag
# example:
# cmake -DFLAT=ON .
option(FLAT "Compile with flat coordinator" OFF) #OFF by default

if(FLAT)
  #add some compilation flags
  FILE(GLOB Examples RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} example/*/main_*.cpp)
  foreach(exampleSrc ${Examples})
      get_filename_component(exampleName ${exampleSrc} NAME_WE)
      add_executable("${exampleName}_flat" ${exampleSrc})
      target_link_libraries("${exampleName}_flat" cadmium)
      target_compile_definitions("${exampleName}_flat" PUBLIC FLAT)
  endforeach(exampleSrc)
else()

endif(FLAT)
unset(FLAT CACHE) # <---- this is the important!!


find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)
if(Boost_FOUND)
    add_definitions(-DBOOST_TEST_DYN_LINK)
    enable_testing()
    FILE(GLOB Tests RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} test/*/test_*.cpp)
    foreach(testSrc ${Tests})
        get_filename_component(testName ${testSrc} NAME_WE)
        add_executable(${testName} ${testSrc})
        target_link_libraries(${testName} cadmium ${Boost_FILESYSTEM_LIBRARY}
                ${Boost_SYSTEM_LIBRARY} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
        add_test(NAME ${testName} COMMAND ${testName})
    endforeach(testSrc)
endif()
