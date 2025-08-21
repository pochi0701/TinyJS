# CMakeList.txt : TinyJS の CMake プロジェクト。ソースを含めて、次を定義します:
# プロジェクト専用ロジックはこちらです。
#
cmake_minimum_required (VERSION 3.8)

# サポートされている場合は、MSVC コンパイラのホット リロードを有効にします。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("TinyJS")

# ソースをこのプロジェクトの実行可能ファイルに追加します。
add_executable (TinyJS 
"run_tests.cpp"
"TinyJS.cpp"
"TinyJS_Functions.cpp"
"TinyJS_MathFunctions.cpp"
"ltn_String.cpp"
"TinyJS.h"
"TinyJS_Functions.h"
"TinyJS_MathFunctions.h"
"ltn_String.h"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET TinyJS PROPERTY CXX_STANDARD 20)
endif()

# TODO: テストを追加し、必要な場合は、ターゲットをインストールします。
