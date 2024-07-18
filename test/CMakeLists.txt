add_executable(test01
        ../src/math/Vector.cpp
        ../src/2dfm/2dfmFile.hpp
        test01.cpp
)
#find_package(Iconv REQUIRED)
target_link_libraries(test01 PUBLIC iconv)
# target_include_directories(test01 PUBLIC ../third_party/libiconv/include)
