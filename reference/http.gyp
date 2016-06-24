{
  'target_defaults': {
    'default_configuration': 'debug',
    # apparently they call it xcode even with make as generator
    'xcode_settings': {
      'OTHER_CFLAGS': [
        '-std=c++11',
        '-stdlib=libc++'
       ]
    },
    'configurations': {
      'debug': {
        'defines': [
          'LOG_LEVEL=DEBUG_LEVEL'
        ],
        'xcode_settings': {
          # make implicit variables: http://goo.gl/YN7wjT
          # make -p shows 
          #   CXX = c++
          #   CC = cc
          # lrwxr-xr-x  1 root  wheel  7 Sep 23  2013 /usr/bin/c++ -> clang++
          # lrwxr-xr-x  1 root  wheel  5 Sep 23  2013 /usr/bin/cc -> clang
          # xcode build settings: http://goo.gl/uYnGs6
          # apparently hard-coded to gcc (??) its using clang
          # who translates these to actual compiler flags?
          'GCC_GENERATE_DEBUGGING_SYMBOLS': 'YES',
          # this is the default http://goo.gl/WG6YSs
          'GCC_OPTIMIZATION_LEVEL': '0'
        }
      },
      'release': {
        'xcode_settings': {
          'GCC_OPTIMIZATION_LEVEL': '3'
        }
      }
    }
  },
  'targets':[{
    'target_name': 'tcp',
    'type': 'executable',
    'sources': [
      'tcp.cpp',
      'parser.cpp',
      'main.cpp'
    ]
  },
  {
    'target_name': 'parser_tests',
    'type': 'executable',
    'dependencies': [
      'gtest'
    ],
    'include_dirs': [
      'vendor/gtest/include'
    ],
    'sources': [
      'parser.cpp',
      'test/test_parser.cpp'
    ]
  },
  {
    'target_name': 'gtest',
    'type': 'static_library',
    'include_dirs': [
      'vendor/gtest',
      'vendor/gtest/include',
    ],
    'link_settings': {
      'ldflags': ['-lpthread'],
    },
    'sources': [
      'vendor/gtest/include/gtest/gtest-death-test.h',
      'vendor/gtest/include/gtest/gtest-message.h',
      'vendor/gtest/include/gtest/gtest-param-test.h',
      'vendor/gtest/include/gtest/gtest-printers.h',
      'vendor/gtest/include/gtest/gtest-spi.h',
      'vendor/gtest/include/gtest/gtest-test-part.h',
      'vendor/gtest/include/gtest/gtest-typed-test.h',
      'vendor/gtest/include/gtest/gtest.h',
      'vendor/gtest/include/gtest/gtest_pred_impl.h',
      'vendor/gtest/include/gtest/internal/gtest-death-test-internal.h',
      'vendor/gtest/include/gtest/internal/gtest-filepath.h',
      'vendor/gtest/include/gtest/internal/gtest-internal.h',
      'vendor/gtest/include/gtest/internal/gtest-linked_ptr.h',
      'vendor/gtest/include/gtest/internal/gtest-param-util-generated.h',
      'vendor/gtest/include/gtest/internal/gtest-param-util.h',
      'vendor/gtest/include/gtest/internal/gtest-port.h',
      'vendor/gtest/include/gtest/internal/gtest-string.h',
      'vendor/gtest/include/gtest/internal/gtest-tuple.h',
      'vendor/gtest/include/gtest/internal/gtest-type-util.h',
      'vendor/gtest/src/gtest-all.cc',
      'vendor/gtest/src/gtest-death-test.cc',
      'vendor/gtest/src/gtest-filepath.cc',
      'vendor/gtest/src/gtest-internal-inl.h',
      'vendor/gtest/src/gtest-port.cc',
      'vendor/gtest/src/gtest-printers.cc',
      'vendor/gtest/src/gtest-test-part.cc',
      'vendor/gtest/src/gtest-typed-test.cc',
      'vendor/gtest/src/gtest.cc',
    ]
  }]
}
