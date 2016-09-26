set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DQUARKE_DEBUG")

set(GLAD_SOURCES glad/src/glad.c)
set(GLAD_INCLUDE_DIR glad/include)

set(QUARKE_SOURCES
    main.cc
    pipe/fragment_stage.cc
    pipe/geometry_stage.cc
    pipe/phong_stage.cc
    pipe/ambient_stage.cc
    pipe/omni_shadow_stage.cc
    mat/solid_material.cc
    mat/textured_material.cc
    geo/mesh.cc
    geo/linked_mesh_collection.cc
    game/camera.cc
    game/game.cc
    game/scene.cc
    util/toytga.cc
    ${GLAD_SOURCES}
    )

add_executable(quarke ${QUARKE_SOURCES})
target_link_libraries(quarke glfw tinyobjloader)
target_include_directories(quarke PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(quarke PUBLIC ../third_party/tinyobjloader)
target_include_directories(quarke PUBLIC ${GLAD_INCLUDE_DIR})

# Copy over asset directories on modification.
add_custom_command(TARGET quarke POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/model
                   ${CMAKE_BINARY_DIR}/model)
add_custom_command(TARGET quarke POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/tex
                   ${CMAKE_BINARY_DIR}/tex)
