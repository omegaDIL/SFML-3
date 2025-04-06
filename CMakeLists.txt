cmake_minimum_required (VERSION 3.20)
project("SFML_3_Template" VERSION 1.0 LANGUAGES CXX)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Compile dynamiquement. 
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
set(SFML_STATIC_LIBRARIES FALSE)

# Spécifiez la version C++ standard.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Activez Rechargement à chaud pour les compilateurs MSVC si cela est pris en charge.
if(POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    if(MSVC)
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
    endif()
endif()

# Ajout de la bibliothèque SFML.
include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG master)
FetchContent_MakeAvailable(SFML)

# Ajout des fichiers cpp et hpp.
file(GLOB_RECURSE source_files
    "src/*.cpp"
    "src/*.hpp"
)

add_executable(${PROJECT_NAME} ${source_files})
target_link_libraries(${PROJECT_NAME} PRIVATE SFML::System SFML::Window SFML::Graphics)