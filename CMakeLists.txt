cmake_minimum_required(VERSION 3.10)
project(gradus VERSION 1.0.0 LANGUAGES C)

# Установка стандарта C
set(CMAKE_C_STANDARD 99) # C23
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Опции сборки
option(BUILD_TESTS "Build test suite" ON)
option(BUILD_DOCS "Build documentation" OFF)
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

# Определение компилятора и флагов
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
    add_compile_options(-fstack-protector-strong -D_FORTIFY_SOURCE=2)
    
    # Опции отладки/релиза
    add_compile_options($<$<CONFIG:Debug>:-O0 -g3 -ggdb>)
    add_compile_options($<$<CONFIG:Release>:-O3 -DNDEBUG>)
    
    # Санитайзеры
    if(ENABLE_ASAN)
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()
    
    if(ENABLE_UBSAN)
        add_compile_options(-fsanitize=undefined)
        add_link_options(-fsanitize=undefined)
    endif()
elseif(MSVC)
    add_compile_options(/W4 /WX)
    add_compile_options($<$<CONFIG:Debug>:/Od /Zi>)
    add_compile_options($<$<CONFIG:Release>:/O2 /DNDEBUG>)
endif()

# Настройка для Windows
if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Основная программа
add_executable(gradus gradus_enhanced.c)

# Линковка с математической библиотекой
target_link_libraries(gradus m)

# Установка цели
install(TARGETS gradus
    RUNTIME DESTINATION bin
    COMPONENT runtime
)

# Создание пакетов
include(CPack)
set(CPACK_PACKAGE_NAME "gradus")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Temperature conversion utility")
set(CPACK_PACKAGE_VENDOR "Your Organization")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README_ENHANCED.md")

# Тесты
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Документация
if(BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
            ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
        add_custom_target(doc
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen" VERBATIM
        )
    endif()
endif()

# Цели для разработки
add_custom_target(format
    COMMAND clang-format -i gradus_enhanced.c
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Formatting source code"
)

add_custom_target(analyze
    COMMAND clang-tidy gradus_enhanced.c --
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running static analysis"
)