cmake_minimum_required(VERSION 3.14)
project(gLang)

set(CMAKE_CXX_COMPILER "/usr/bin/g++" CACHE string "g++ compiler" FORCE)

set(CMAKE_CXX_STANDART 20)
set(CMAKE_CXX_STANDART_REQUIRED ON)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2 -w " CACHE STRING "Comment" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} " CACHE STRING "Comment" FORCE)
set(CMAKE_CXX_FLAGS_SANITIZER "${CMAKE_CXX_FLAGS} -msse4.2 -Wpedantic -Wall -Wextra -Wformat=2 -fsanitize=address,undefined -g -D __SANITIZE_ADDRESS__ -D EXTRA_VERBOSE" CACHE STRING "Comment" FORCE)
set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS} -msse4.2 -D NDEBUG -fprofile-instr-generate -fcoverage-mapping" CACHE STRING "Comment" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -msse4.2 -D FULL_DEBUG -g -Werror -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++0x-compat -Wc++11-compat -Wc++14-compat -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlarger-than=8192 -Wlogical-op -Wmissing-declarations -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstack-usage=8192 -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -fcheck-new -fsized-deallocation -fstack-check -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -fPIE -fsanitize=address -fsanitize=alignment -fsanitize=bool -fsanitize=bounds -fsanitize=enum -fsanitize=float-cast-overflow -fsanitize=float-divide-by-zero -fsanitize=integer-divide-by-zero -fsanitize=leak -fsanitize=nonnull-attribute -fsanitize=null -fsanitize=object-size -fsanitize=return -fsanitize=returns-nonnull-attribute -fsanitize=shift -fsanitize=signed-integer-overflow -fsanitize=undefined -fsanitize=unreachable -fsanitize=vla-bound -fsanitize=vptr -lm -pie")

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.8.0
)

FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()

FetchContent_Declare(
  gutils
  GIT_REPOSITORY https://github.com/lord-ka/gutils.git
  GIT_TAG        release-1.0.4
)

FetchContent_GetProperties(gutils)
if(NOT gutils_POPULATED)
  FetchContent_Populate(gutils)
  include_directories(${gutils_SOURCE_DIR})
endif()

FetchContent_Declare(
  gobjpool
  GIT_REPOSITORY https://github.com/Lord-KA/gObjPool.git
  GIT_TAG        release-1.0.4
)
if(NOT gobjpool_POPULATED)
  FetchContent_Populate(gobjpool)
  include_directories(${gobjpool_SOURCE_DIR})
endif()

FetchContent_Declare(
  gtree
  GIT_REPOSITORY https://github.com/Lord-KA/gTree.git
  GIT_TAG        release-1.1.0.1
)
if(NOT gtree_POPULATED)
  FetchContent_Populate(gtree)
  include_directories(${gtree_SOURCE_DIR})
endif()

FetchContent_Declare(
  gstack
  GIT_REPOSITORY https://github.com/lord-KA/hopefullyunkillablestack.git
  GIT_TAG        release-1.1.1
)
if(NOT gstack_POPULATED)
  FetchContent_Populate(gstack)
  include_directories(${gstack_SOURCE_DIR})
endif()



set_target_properties(gtest PROPERTIES FOLDER extern)
set_target_properties(gtest_main PROPERTIES FOLDER extern)
set_target_properties(gmock PROPERTIES FOLDER extern)
set_target_properties(gmock_main PROPERTIES FOLDER extern)

enable_testing()

add_executable(glang glang-main.cpp glang.h)

target_link_libraries(
    #    gderivator-test?
    gtest_main
)

include(GoogleTest)
