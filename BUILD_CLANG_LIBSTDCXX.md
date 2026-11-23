# macOS: LLVM clang with GCC libstdc++

These commands build wxWidgets 3.3.4 and Croplines for arm64 macOS using
Homebrew LLVM 23.1.6 and GCC 16's libstdc++.

```sh
cmake -S /Users/mac/Downloads/wxWidgets-3.3.4 \
  -B /Users/mac/Downloads/wxWidgets-3.3.4/build-clang-libstdcxx-make \
  -G 'Unix Makefiles' \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/Users/mac/Downloads/wxWidgets-3.3.4/install-clang-libstdcxx \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=26.0 \
  -DCMAKE_C_COMPILER=/opt/homebrew/Cellar/llvm/23.1.6/bin/clang \
  -DCMAKE_CXX_COMPILER=/opt/homebrew/Cellar/llvm/23.1.6/bin/clang++ \
  -DCMAKE_OBJC_COMPILER=/opt/homebrew/Cellar/llvm/23.1.6/bin/clang \
  -DCMAKE_OBJCXX_COMPILER=/opt/homebrew/Cellar/llvm/23.1.6/bin/clang++ \
  -DCMAKE_CXX_FLAGS='-nostdinc++ -isystem /opt/homebrew/Cellar/gcc/16.2.0/include/c++/16 -isystem /opt/homebrew/Cellar/gcc/16.2.0/include/c++/16/aarch64-apple-darwin25' \
  -DCMAKE_OBJCXX_FLAGS='-nostdinc++ -isystem /opt/homebrew/Cellar/gcc/16.2.0/include/c++/16 -isystem /opt/homebrew/Cellar/gcc/16.2.0/include/c++/16/aarch64-apple-darwin25 -fno-objc-msgsend-selector-stubs -fno-objc-msgsend-class-selector-stubs' \
  -DCMAKE_EXE_LINKER_FLAGS='-nostdlib++ /opt/homebrew/Cellar/gcc/16.2.0/lib/gcc/current/libstdc++.a' \
  -DCMAKE_SHARED_LINKER_FLAGS='-nostdlib++ /opt/homebrew/Cellar/gcc/16.2.0/lib/gcc/current/libstdc++.a' \
  -DCMAKE_MODULE_LINKER_FLAGS='-nostdlib++ /opt/homebrew/Cellar/gcc/16.2.0/lib/gcc/current/libstdc++.a' \
  -DwxBUILD_SHARED=OFF \
  -DwxBUILD_SAMPLES=OFF \
  -DwxBUILD_TESTS=OFF \
  -DwxBUILD_BENCHMARKS=OFF \
  -DwxBUILD_INSTALL=ON

cmake --build /Users/mac/Downloads/wxWidgets-3.3.4/build-clang-libstdcxx-make --parallel 8
cmake --install /Users/mac/Downloads/wxWidgets-3.3.4/build-clang-libstdcxx-make
```

```sh
cmake -S /Users/mac/code/Croplines \
  -B /Users/mac/code/Croplines/build-clang-libstdcxx \
  -G 'Unix Makefiles' \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=26.0 \
  -DCMAKE_C_COMPILER=/opt/homebrew/Cellar/llvm/23.1.6/bin/clang \
  -DCMAKE_CXX_COMPILER=/opt/homebrew/Cellar/llvm/23.1.6/bin/clang++ \
  -DwxWidgets_DIR=/Users/mac/Downloads/wxWidgets-3.3.4/install-clang-libstdcxx/lib/cmake/wxWidgets-3.3 \
  -DCMAKE_CXX_FLAGS='-nostdinc++ -isystem /opt/homebrew/Cellar/gcc/16.2.0/include/c++/16 -isystem /opt/homebrew/Cellar/gcc/16.2.0/include/c++/16/aarch64-apple-darwin25' \
  -DCMAKE_EXE_LINKER_FLAGS='-nostdlib++ -L/opt/homebrew/Cellar/gcc/16.2.0/lib/gcc/current /opt/homebrew/Cellar/gcc/16.2.0/lib/gcc/current/libstdc++.a'

cmake --build /Users/mac/code/Croplines/build-clang-libstdcxx --parallel 8
/Users/mac/code/Croplines/build-clang-libstdcxx/Croplines
```

`CMAKE_CXX_STANDARD` is already set to 23 by `CMakeLists.txt`. GCC 16's
`libstdc++` provides `std::generator`; `std::execution::par` uses TBB, which
the project links through `TBB::tbb`.
