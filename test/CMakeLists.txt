add_executable(test01
        ../src/math/Vector.cpp
        ../src/2dfm/2dfmFile.hpp
        test01.cpp
)
find_package(Iconv REQUIRED)
target_link_libraries(test01 Iconv)
