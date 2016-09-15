set(GLAD_SOURCES glad/src/glad.c)
set(GLAD_INCLUDE_DIR glad/include)

set(QUARKE_SOURCES
    main.cc
#   pipe/stage.cc
    pipe/geometry_stage.cc
    mat/solid_material.cc
    geo/mesh.cc
    game/camera.cc
    game/game.cc
    game/scene.cc
    ${GLAD_SOURCES}
    )

add_executable(quarke ${QUARKE_SOURCES})
target_link_libraries(quarke glfw tinyobjloader)
target_include_directories(quarke PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(quarke PUBLIC ../third_party/tinyobjloader)
target_include_directories(quarke PUBLIC ${GLAD_INCLUDE_DIR})
