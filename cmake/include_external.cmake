# gd code, general code to manage data
file(GLOB external_gd ${CMAKE_SOURCE_DIR}/external/gd/*.cpp)

# imgui code, user interface that works in both linux and windows
file(GLOB external_imgui 
   ${CMAKE_SOURCE_DIR}/external/imgui/*.cpp
)