clang++ -Ivendor/gtest/include -o test_parser -Lvendor/gtest/make_cmake -lgtest_main -lgtest -stdlib=libc++ -std=c++11 test/test_parser.cpp && ./test_parser
