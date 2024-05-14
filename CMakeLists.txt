cmake_minimum_required(VERSION 3.15)
project(vtfontmaker)

set(
    MAIN_FILES
    "src/main.cpp"
    "src/application.cpp"
    "src/canvas.cpp"
    "src/capabilities.cpp"
    "src/charsets.cpp"
    "src/coloring.cpp"
    "src/common_dialog.cpp"
    "src/dialog.cpp"
    "src/font.cpp"
    "src/glyphs.cpp"
    "src/iso2022.cpp"
    "src/keyboard.cpp"
    "src/macros.cpp"
    "src/menu.cpp"
    "src/os.cpp"
    "src/status.cpp"
    "src/vt.cpp"
)

set(
    DOC_FILES
    "README.md"
    "LICENSE.txt"
)

if(WIN32)
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

add_executable(vtfontmaker ${MAIN_FILES})

set_target_properties(vtfontmaker PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED On)
source_group("Doc Files" FILES ${DOC_FILES})
