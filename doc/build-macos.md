# Building librime-qjs on macOS

## Prerequisites

### System Requirements
- Tested macOS versions: 15.3 (Apple Silicon) and 12.7.6 (Intel) with Squirrel.
- The other versions should also work.

### Required Tools
- Xcode Command Line Tools
- cmake 3.12 or newer
- clang/llvm 22 or newer
- Node.js 22 or newer

## Environment Setup

### Installing Build Tools
- Xcode Command Line Tools:
  - Install Xcode Command Line Tools: `xcode-select --install`
  - Check Xcode version: `xcodebuild -version`

- cmake:
  - Install cmake: `sudo port install cmake`
  - Check cmake version: `cmake --version`

- clang/llvm:
  - Install clang/llvm: `sudo port install clang-22`
  - Add llvm to PATH: `export PATH="/opt/local/libexec/llvm-22/bin:$PATH"`
  - config llvm: `sudo port select --set llvm mp-llvm-22`
  - config clang: `sudo port select --set clang mp-clang-22`
  - Update the shell configuration file (.zshrc or .bash_profile):
    ```shell
    # Add MacPorts LLVM to your PATH
    export PATH="/opt/local/libexec/llvm-22/bin:$PATH"

    # llvm/clang installed by MacPorts
    alias clang="$(llvm-config --bindir)/clang"
    alias clang++="$(llvm-config --bindir)/clang++"

    # Set the compiler environment variables before running cmake
    export CC=$(llvm-config --bindir)/clang
    export CXX=$(llvm-config --bindir)/clang++
    # Set up compiler flags if needed
    export LDFLAGS="-L/opt/local/libexec/llvm-22/lib"
    export CPPFLAGS="-I/opt/local/libexec/llvm-22/include"
    ```
  - Check LLVM version: `llvm-config --version` and `clang --version`

- Node.js:
  - Download and install Node.js: https://nodejs.org/en/download/
  - Verify node version in command line: `node --version`
  - Verify npm version in command line: `npm --version`

## Build Steps

### Getting the Source Code

- Clone the [librime](https://github.com/rime/librime) repository:
  ```shell
  cd <your/folder/to/develop/librime>
  git clone --recursive https://github.com/rime/librime.git
  ```

- Clone the [librime-qjs](https://github.com/HuangJian/librime-qjs) repository:
  ```shell
  cd plugins
  git clone --recursive https://github.com/HuangJian/librime-qjs.git qjs
  ```

### Building the Project
- Install Boost C++ libraries: `bash ./install-boost.sh`
- Build 3rd-party libraries: `make deps`
- Build librime and librime-qjs: `make` or `make debug`

### Running Tests
- Install the node modules: `(cd plugins/qjs/tests/js; npm install)`
- Run all unit tests: `make test` or `make test-debug`
- Run only the librime-qjs tests: `(cd plugins/qjs; ctest)`

## Installation

### Installing the Plugin
```shell
export qjs_dylib="<your/librime/folder>/build/lib/rime-plugins/librime-qjs.dylib"
export rime_plugin_folder="/Library/Input Methods/Squirrel.app/Contents/Frameworks/rime-plugins"

sudo cp ${qjs_dylib} ${rime_plugin_folder} && \
    /Library/Input\ Methods/Squirrel.app/Contents/MacOS/Squirrel --quit
```

### Verifying Installation
- Enable the Squirrel input method with <kbd>⌘</kbd> + <kbd>Space</kbd>
- `cat $TMPDIR/rime.squirrel.INFO | grep qjs` there should be something like:
    > loading plugin 'qjs' from /Library/Input Methods/Squirrel.app/Contents/Frameworks/rime-plugins/librime-qjs.dylib

### Testing JavaScript Plugins

1. Set up the test environment:
   - A quickjs executable is patched to load node modules. It's built at `librime/plugins/qjs/build/qjs`.
   - Copy it to the Squirrel user folder `~/Library/Rime`
   - Ensure your folder structure looks like this:
     ```
     <Rime-user-folder>
     ├─── build
     ├─── js
     │    ├─── qjs
     │    ├─── package.json
     │    ├─── node_modules
     │    ├─── <your-plugin>.js
     │    └─── tests
     │          └─── <your-plugin>.test.js
     ├─── ...
     ├─── squirrel.custom.yaml
     └─── default.yaml
     ```

2. Run your plugin tests:
   ```shell
   qjs tests/<your-plugin>.test.js
   ```

## Troubleshooting

### Get comprehensive logs
- Merge all the logs into one file:
    ```shell
    export qjs_dylib="<your/librime/folder>/build/lib/rime-plugins/librime-qjs.dylib"
    export rime_plugin_folder="/Library/Input Methods/Squirrel.app/Contents/Frameworks/rime-plugins"

    sudo cp ${qjs_dylib} ${rime_plugin_folder} && \
        /Library/Input\ Methods/Squirrel.app/Contents/MacOS/Squirrel --quit && \
        /Library/Input\ Methods/Squirrel.app/Contents/MacOS/Squirrel \
        > $TMPDIR/rime.debug-$(date +%Y%m%d).log 2>&1
    ```
- check the logs in another terminal: `tail -f $TMPDIR/rime.debug-$(date +%Y%m%d).log`
  - The librime-qjs engine's logs will be printed with the prefix `[qjs]`:
    - `module.cc:16] [qjs] registering components from module 'qjs'`
    - `qjs_component.h:111] [qjs] creating component 'select_character'.`
  - The javascript plugins' `console.log` will be printed with the prefix `$qjs$`:
    - `qjs_helper.cc:100] $qjs$ select_character.js init`
    - `qjs_helper.cc:100] $qjs$ charset_filter finit`
  - libRime and other plugins' logs will be printed as well

## Development

### VSCode C/C++ Intellisense with clangd

- Generate the compilation database: `(cd ./plugins/qjs; cmake .)`
- Move it to the build folder: `mv ./plugins/qjs/compile_commands.json ./plugins/qjs/build/`
- Install VSCode's [clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd) extension

### Lint with clang-tidy

- clang-tidy is a part of the clang/llvm toolchain, check it's version: `clang-tidy --version`
- lint all the files: `(cd ./plugins/qjs; bash ./tools/clang-tidy.sh)`
- lint certain file: `(cd ./plugins/qjs; bash ./tools/clang-tidy.sh --worker-file=<file>)`
- The issues should be present in the VSCode Editor window after clangd setup.
- Fix the issues as best as you can.

### Format with clang-format

- clang-format is a part of the clang/llvm toolchain, check it's version: `clang-format --version`
- format all the code: `./tools/format-code.sh`
- add the pre-commit hook to format the changed files automatically: `git config core.hooksPath .githooks`

### Verify the javascript code with qjs in command line

- A quickjs executable is patched to load node modules. It's built at `librime/plugins/qjs/build/qjs`.
- Copy it to your folder: `cp ./plugins/qjs/build/qjs ./plugins/qjs/tests/js`
- run the javascript code: `./plugins/qjs/tests/js/qjs ./plugins/qjs/tests/js/main.js`

### (inaccurate) Benchmark

- uncomment the benchmark building section in CMakeLists.txt:
  ```cmake
    set(benchmark_files "tests/benchmark/dict/load_trie_benchmark.cc")
    add_executable(load-dict-benchmark ${benchmark_files})
    target_link_libraries(load-dict-benchmark
      ${rime_library}
      ${rime_dict_library}
      ${rime_gears_library}
      ${GTEST_LIBRARIES}
    )
  ```
- build it: `make`
- run it: `./plugins/qjs/build/load-dict-benchmark`
- feel free to add more benchmarks

### Debugging memory issues

1. remove the previous built files: `make clean`
2. Rebuild the project with ASan: `make test-debug CFLAGS="-g -fsanitize=address" LDFLAGS="-fsanitize=address"`
3. run the tests: `ctest`
4. fix the issues shown in the output.
    1. > `Assertion failed: (list_empty(&rt->gc_obj_list)), function JS_FreeRuntime, file quickjs.c, line 2145.`
        - This is a memory leak issue, which usually happens when a JSValue is returned from a function but is not used in the caller.
        - For example: `JS_EvalFunction` and `QjsHelper::loadJsModuleToGlobalThis`
        - To fix it, the JSValue should be freed by `JS_FreeValue`,
            or be wrapped into a `JSValueRAII` to free automatically.
    2. > ` ==14658==ERROR: AddressSanitizer: heap-use-after-free on address 0x6060000453e0 at pc 0x00010037f462 bp 0x7ff7bfc07f90 sp 0x7ff7bfc07f88`
        - This is a heap-use-after-free issue, which usually happens when a
            JSValue is wrapped into `JSValueRAII`, and then set as a property of another JSValue.
        - For example: `JSValueRAII xxx(); JS_SetPropertyStr(ctx, var, "xxx", xxx);`
        - To fix it, just define it as a `JSValue` rather than `JSValueRAII`.
5. a step further: `ASAN_OPTIONS=detect_leaks=1 ctest`. It would print all the detected leaks:
     - <details>
         <summary>leak at librime-qjs</summary>
         <blockquote>
         Direct leak of 48 byte(s) in 3 object(s) allocated from:<br>
         #0 0x00010e9d1f9d in _Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x6af9d)<br>
         #1 0x00010df89544 in _ZNSt3__111make_uniqueB8ne200100INS_10shared_ptrIN4rime9CandidateEEEJRKS4_ETnNS_9enable_ifIXntsr8is_arrayIT_EE5valueEiE4typeELi0EEENS_10unique_ptrIS8_NS_14default_deleteIS8_EEEEDpOT0_ unique_ptr.h:767<br>
         #2 0x00010df893db in rime::QjsCandidate::Wrap(JSContext*, std::__1::shared_ptr<rime::Candidate> const&) qjs_candidate.cc:64<br>
         #3 0x00010df86a54 in rime::QuickJSTranslation::DoFilter(JSValue const&, JSValue const&, JSValue const&) qjs_translation.cc:30<br>
         #4 0x00010df8685f in rime::QuickJSTranslation::QuickJSTranslation(std::__1::shared_ptr<rime::Translation>, JSValue const&, JSValue const&, JSValue const&) qjs_translation.cc:18<br>
         #5 0x00010df87180 in rime::QuickJSTranslation::QuickJSTranslation(std::__1::shared_ptr<rime::Translation>, JSValue const&, JSValue const&, JSValue const&) qjs_translation.cc:17<br>
         #6 0x00010df6a49a in void std::__1::allocator<rime::QuickJSTranslation>::construct[abi:ne200100]<rime::QuickJSTranslation, std::__1::shared_ptr<rime::Translation>&, JSValue, rime::JSValueRAII&, rime::JSValueRAII&>(rime::QuickJSTranslation*, std::__1::shared_ptr<rime::Translation>&, JSValue&&, rime::JSValueRAII&, rime::JSValueRAII&) allocator.h:153<br>
         #7 0x00010df6a3fc in _ZNSt3__116allocator_traitsINS_9allocatorIN4rime18QuickJSTranslationEEEE9constructB8ne200100IS3_JRNS_10shared_ptrINS2_11TranslationEEE7JSValueRNS2_11JSValueRAIIESD_ETnNS_9enable_ifIXsr15__has_constructIS4_PT_DpT0_EE5valueEiE4typeELi0EEEvRS4_SG_DpOSH_ allocator_traits.h:309<br>
         #8 0x00010df6a37f in _ZNSt3__120__shared_ptr_emplaceIN4rime18QuickJSTranslationENS_9allocatorIS2_EEEC2B8ne200100IJRNS_10shared_ptrINS1_11TranslationEEE7JSValueRNS1_11JSValueRAIIESD_ES4_TnNS_9enable_ifIXntsr7is_sameINT0_10value_typeENS_19__for_overwrite_tagEEE5valueEiE4typeELi0EEES4_DpOT_ shared_ptr.h:161<br>
         #9 0x00010df6a2f4 in _ZNSt3__120__shared_ptr_emplaceIN4rime18QuickJSTranslationENS_9allocatorIS2_EEEC1B8ne200100IJRNS_10shared_ptrINS1_11TranslationEEE7JSValueRNS1_11JSValueRAIIESD_ES4_TnNS_9enable_ifIXntsr7is_sameINT0_10value_typeENS_19__for_overwrite_tagEEE5valueEiE4typeELi0EEES4_DpOT_ shared_ptr.h:158<br>
         #10 0x00010df6a221 in _ZNSt3__115allocate_sharedB8ne200100IN4rime18QuickJSTranslationENS_9allocatorIS2_EEJRNS_10shared_ptrINS1_11TranslationEEE7JSValueRNS1_11JSValueRAIIESB_ETnNS_9enable_ifIXntsr8is_arrayIT_EE5valueEiE4typeELi0EEENS5_ISD_EERKT0_DpOT1_ shared_ptr.h:733<br>
         #11 0x00010df6a19c in _ZNSt3__111make_sharedB8ne200100IN4rime18QuickJSTranslationEJRNS_10shared_ptrINS1_11TranslationEEE7JSValueRNS1_11JSValueRAIIES9_ETnNS_9enable_ifIXntsr8is_arrayIT_EE5valueEiE4typeELi0EEENS3_ISB_EEDpOT0_ shared_ptr.h:741<br>
         #12 0x00010df685d7 in std::__1::shared_ptr<rime::QuickJSTranslation> rime::New<rime::QuickJSTranslation, std::__1::shared_ptr<rime::Translation>&, JSValue, rime::JSValueRAII&, rime::JSValueRAII&>(std::__1::shared_ptr<rime::Translation>&, JSValue&&, rime::JSValueRAII&, rime::JSValueRAII&) common.h:76<br>
         #13 0x00010df67a3d in QuickJSTranslationTest_FilterCandidates_Test::TestBody() translation.test.cpp:53<br>
         #14 0x00010dfb8f47 in void testing::internal::HandleExceptionsInMethodIfSupported<testing::Test, void>(testing::Test*, void (testing::Test::*)(), char const*)+0x47 (librime-qjs-tests:x86_64+0x100076f47)<br>
         #15 0x00010dfb8e9e in testing::Test::Run()+0x3be (librime-qjs-tests:x86_64+0x100076e9e)<br>
         #16 0x00010dfba69f in testing::TestInfo::Run()+0x44f (librime-qjs-tests:x86_64+0x10007869f)<br>
         #17 0x00010dfbb5af in testing::TestSuite::Run()+0x52f (librime-qjs-tests:x86_64+0x1000795af)<br>
         #18 0x00010dfcb978 in testing::internal::UnitTestImpl::RunAllTests()+0x828 (librime-qjs-tests:x86_64+0x100089978)<br>
         #19 0x00010dfcafc7 in bool testing::internal::HandleExceptionsInMethodIfSupported<testing::internal::UnitTestImpl, bool>(testing::internal::UnitTestImpl*, bool (testing::internal::UnitTestImpl::*)(), char const*)+0x47 (librime-qjs-tests:x86_64+0x100088fc7)<br>
         #20 0x00010dfcaf4b in testing::UnitTest::Run()+0x6b (librime-qjs-tests:x86_64+0x100088f4b)<br>
         #21 0x00010df66860 in RUN_ALL_TESTS() gtest.h:2317<br>
         #22 0x00010df6679f in main rime-qjs-test-main.test.cpp:68<br>
         #23 0x000112bd852d in start+0x1cd (dyld:x86_64+0x552d)<br>
         </blockquote>
       </details>

     - <details>
         <summary>leak at librime</summary>
         <blockquote>
         Direct leak of 72 byte(s) in 1 object(s) allocated from:<br>
             #0 0x00010c44df9d in _Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x6af9d)<br>
             #1 0x00010d6398f4 in rime_dict_initialize() dict_module.cc:38<br>
             #2 0x00010d56253b in rime::ModuleManager::LoadModule(rime_module_t*) module.cc:32<br>
             #3 0x00010d5774de in rime::LoadModules(char const**) setup.cc:37<br>
             #4 0x00010d5773af in rime::rime_default_initialize() setup.cc:30<br>
             #5 0x00010d56253b in rime::ModuleManager::LoadModule(rime_module_t*) module.cc:32<br>
             #6 0x00010d5774de in rime::LoadModules(char const**) setup.cc:37<br>
             #7 0x00010b66947a in main rime-qjs-test-main.test.cpp:20<br>
             #8 0x00010c33052d in start+0x1cd (dyld:x86_64+0x552d)<br>
         </blockquote>
       </details>

     - <details>
         <summary>leak at librime-lua</summary>
         <blockquote>
         Indirect leak of 296 byte(s) in 10 object(s) allocated from:<br>
             #0 0x00010c43b457 in realloc+0x87 (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x58457)<br>
             #1 0x00011088bc4e in l_alloc lauxlib.c:1033<br>
             #2 0x0001108d08e6 in luaM_malloc_ lmem.c:206<br>
             #3 0x0001108b4d74 in luaC_newobjdt lgc.c:260<br>
             #4 0x0001108b4f76 in luaC_newobj lgc.c:271<br>
             #5 0x0001108ef549 in createstrobj lstring.c:148<br>
             #6 0x0001108efea1 in internshrstr lstring.c:209<br>
             #7 0x0001108ef32b in luaS_newlstr lstring.c:224<br>
             #8 0x0001108f0289 in luaS_new lstring.c:254<br>
             #9 0x00011087e2e3 in auxgetstr lapi.c:641<br>
             #10 0x00011087ec26 in lua_getfield lapi.c:691<br>
             #11 0x00011088b89d in luaL_requiref lauxlib.c:986<br>
             #12 0x0001108c1083 in luaL_openlibs linit.c:61<br>
             #13 0x000110878144 in LuaImpl::pmain(lua_State*) lua.cc:201<br>
             #14 0x0001108ab492 in precallC ldo.c:529<br>
             #15 0x0001108abb46 in luaD_precall ldo.c:595<br>
             #16 0x0001108ac5ae in ccall ldo.c:635<br>
             #17 0x0001108ac6c7 in luaD_callnoyield ldo.c:655<br>
             #18 0x000110882f04 in lua_callk lapi.c:1020<br>
             #19 0x000110878125 in Lua::Lua() lua.cc:226<br>
             #20 0x0001108781f4 in Lua::Lua() lua.cc:218<br>
             #21 0x000110774955 in rime_lua_initialize() modules.cc:102<br>
             #22 0x00010d56253b in rime::ModuleManager::LoadModule(rime_module_t*) module.cc:32<br>
             #23 0x00010d77778b in rime::PluginManager::LoadPlugins(rime::path) plugins_module.cc:61<br>
             #24 0x00010d7781a8 in rime_plugins_initialize() plugins_module.cc:123<br>
             #25 0x00010d56253b in rime::ModuleManager::LoadModule(rime_module_t*) module.cc:32<br>
             #26 0x00010d5774de in rime::LoadModules(char const**) setup.cc:37<br>
             #27 0x00010b66947a in main rime-qjs-test-main.test.cpp:20<br>
             #28 0x00010c33052d in start+0x1cd (dyld:x86_64+0x552d)<br>
         </blockquote>
       </details>


### Expand the C++ Macros to debug

1. Run the macro expansion command on your source file: `bash ./tools/ expand-macr.sh src/types/qjs_candidates.cc`
2. Find the expanded code in the output file located in the `build/expanded` directory.
3. Copy the relevant expanded code section back to your source file for debugging purposes.

## References

### Related Documentation

- [Rime with Mac](https://github.com/rime/librime/blob/master/README-mac.md)
