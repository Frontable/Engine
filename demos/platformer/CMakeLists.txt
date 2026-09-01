file(GLOB_RECURSE SANDBOX_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

add_executable(Platformer ${SANDBOX_SOURCES})

target_include_directories(Platformer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src  # ← fixed
)

target_link_libraries(Platformer PRIVATE Frost)

if(WIN32)
    target_compile_definitions(Platformer PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
endif()

add_custom_command(TARGET Platformer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Assets"
        "$<TARGET_FILE_DIR:Platformer>/Assets"  # ← fixed
    COMMENT "Copying Platformer assets"
)