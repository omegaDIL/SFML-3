﻿# CMakeList.txt : projet CMake pour SFML 3.0 template, incluez la source et définissez
# la logique spécifique au projet ici.
#
cmake_minimum_required (VERSION 3.8)

project (SFML_3_Template VERSION 1.0)

# Activez Rechargement à chaud pour les compilateurs MSVC si cela est pris en charge.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project("SFML_3_Template" VERSION 1.0 LANGUAGES CXX)

# Spécifiez la version C++ standard.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Définir les chemins d'inclusion et de bibliothèque pour SFML
set(SFML_INCLUDE_DIR "C:/informatique/librairie/SFML-3.0.0/include")
set(SFML_LIBRARY_DIR "C:/informatique/librairie/SFML-3.0.0/lib")

# Ajouter le chemin de configuration de SFML
set(SFML_DIR "C:/informatique/librairie/SFML-3.0.0/lib/cmake/SFML")
set(CMAKE_PREFIX_PATH "C:/informatique/librairie/SFML-3.0.0/lib/cmake/SFML")

# Ajoutez une source à l'exécutable de ce projet.
add_executable(SFML_3_Template "main.cpp" "GUI.hpp" "GUI.cpp" "Save.hpp" "Save.cpp" "Utils.hpp" "Utils.cpp" "Exceptions.hpp")

# Recherchez la bibliothèque SFML et ajoutez-la
find_package(SFML 3.0 COMPONENTS System Window Graphics REQUIRED)

target_include_directories(SFML_3_Template PRIVATE ${SFML_INCLUDE_DIR})
target_link_directories(SFML_3_Template PRIVATE ${SFML_LIBRARY_DIR})
target_link_libraries(SFML_3_Template PRIVATE sfml-system sfml-window sfml-graphics)
