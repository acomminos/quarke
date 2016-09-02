set(QUARKE_SOURCES
    main.cc
#   pipe/stage.cc
    pipe/geometry_stage.cc
    )

add_executable(quarke ${QUARKE_SOURCES})
target_link_libraries(quarke glfw tinyobjloader)
target_include_directories(quarke PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
