find_package(OpenGL REQUIRED)

set(QUARKE_SOURCES
    main.cc
#   pipe/stage.cc
    pipe/geometry_stage.cc
    geo/material.cc
    geo/mesh.cc
    game/game.cc
    game/scene.cc
    )

add_executable(quarke ${QUARKE_SOURCES})

target_link_libraries(quarke glfw tinyobjloader ${OPENGL_gl_LIBRARY})
target_compile_definitions(quarke PUBLIC -DGLFW_INCLUDE_GLCOREARB)
target_include_directories(quarke PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(quarke PUBLIC ../third_party/tinyobjloader)
target_include_directories(quarke PUBLIC ${OPENGL_INCLUDE_DIR})

# this is a bit dodgy. i hate manual runtime linking so i'm ok with this for now
target_compile_definitions(quarke PUBLIC -DGL_GLEXT_PROTOTYPES)
