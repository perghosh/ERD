# CMakeList.txt : CMake project for ImGui user interface for ERD
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project("ErdImgui" LANGUAGES CXX C)

include(include_external)

add_subdirectory("playground") # use this to test code for later use in web server
add_subdirectory("tests") # tests to check logic in web server


message( STATUS )
message( STATUS "# ----- ------------------------------------------------------" )
message( STATUS "# ----- ImGui applicaiton" )
message( STATUS )

# ============================================================================= set_compiler_options
# Set global compiler options
function( set_compiler_options )
   if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
      target_compile_options(${TARGET_NAME_} PRIVATE "/bigobj")
   endif()
endfunction()



# core ERD imgui domain files
file(GLOB ERD_IMGUI_ 
   *.cpp *.h                  # all cpp files in current directory
) 


set( USE_TARGET_ ON ) # ========================================================= ${TARGET_NAME_}
if( USE_TARGET_ )
   set(TARGET_NAME_ ${PROJECT_NAME})

   add_executable(${TARGET_NAME_}
      "${source_application_root}"
      "${ERD_IMGUI_}"
      "${external_imgui}"
      "${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_win32.cpp"
      "${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_dx12.cpp"
   )

   target_include_directories(${TARGET_NAME_} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../source)
   target_include_directories(${TARGET_NAME_} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../external)
   target_include_directories(${TARGET_NAME_} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../external/imgui)
   target_link_libraries(${TARGET_NAME_} PRIVATE "d3d12.lib" "dxgi.lib" "d3dcompiler.lib")
   set_compiler_options()
endif()