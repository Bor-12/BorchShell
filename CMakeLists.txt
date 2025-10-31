cmake_minimum_required(VERSION 3.10)
project(borchShell C)


set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)


set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(TEST_DIR ${CMAKE_SOURCE_DIR}/test)
set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin)


file(MAKE_DIRECTORY ${BIN_DIR})


set(SOURCES
    ${SRC_DIR}/main.c
    ${SRC_DIR}/parser.c
)


include_directories(${INCLUDE_DIR})


add_executable(${PROJECT_NAME} ${SOURCES})


add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:${PROJECT_NAME}> ${BIN_DIR}/${PROJECT_NAME}
)


add_executable(test_parser
    ${TEST_DIR}/test_parser.c
    ${SRC_DIR}/parser.c
)

target_include_directories(test_parser PRIVATE ${INCLUDE_DIR})