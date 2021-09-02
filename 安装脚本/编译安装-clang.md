#### build_clang.sh:

可能要配一下 `.bashrc` 里的 gcc 环境变量。
```cpp
git clone git@gitee.com:mirrors/LLVM.git
cd LLVM
git branch 7.x origin/release/7.x
git checkout 7.x

mkdir build && cd build
cmake -G "Unix Makefiles" \
-DLLVM_ENABLE_PROJECTS='clang;clang-tools-extra;debuginfo-tests;libcxx;libcxxabi;libunwind;lldb;compiler-rt' \
-DCMAKE_INSTALL_PREFIX=/home/zhangrongrong/.local/llvm-9.0.1 \
-DCMAKE_BUILD_TYPE=Release \
-DLLVM_ENABLE_ASSERTIONS=On \
../llvm

#### 其他可选：
-DCMAKE_C_COMPILER=/home/zhangrongrong/.local/gcc-7.4.0/bin/gcc \
-DCMAKE_CXX_COMPILER=/home/zhangrongrong/.local/gcc-7.4.0/bin/g++ \

#### 上面只是选了 `clang;clang-tools-extra;libcxx;libcxxabi;libunwind;lldb;compiler-rt` 几种包，cmake 开始会打印更多信息，还有其他包可选:
-- clang project is enabled
-- clang-tools-extra project is enabled
-- compiler-rt project is enabled
-- debuginfo-tests project is disabled
-- libc project is disabled
-- libclc project is disabled
-- libcxx project is enabled
-- libcxxabi project is enabled
-- libunwind project is enabled
-- lld project is disabled
-- lldb project is enabled
-- mlir project is disabled
-- openmp project is disabled
-- parallel-libs project is disabled
-- polly project is disabled
-- pstl project is disabled


make -j32
make install 
make docs-llvm-html
make clean
```