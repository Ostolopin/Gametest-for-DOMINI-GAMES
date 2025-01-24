cmake_minimum_required (VERSION 3.8)

# Включим поддержку EditAndContinue для отладки в MSVC (опционально)
if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    #set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT 
    #    "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,"
    #    "$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,"
    #    "$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>"
    #)
endif()

project ("Gametest")

# ================================
# Подключаем OpenGL и GLUT (freeglut)
# ================================
find_package(OpenGL REQUIRED)
find_package(FreeGLUT CONFIG REQUIRED)

# Создаём исполняемый файл из исходников
add_executable(Gametest 
    "Gametest.cpp"
    
    # если есть другие cpp-файлы, добавьте их тоже
  "ChessPiece.cpp" "Player.cpp" "AIPlayer.cpp" "Board.cpp" "Game.cpp")

# Ставим стандарт C++ (если нужен не 20, меняем на 17/11/23 и т.п.)
if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET Gametest PROPERTY CXX_STANDARD 20)
endif()

# ================================
# Линкуем библиотеки OpenGL/GLUT
# ================================
target_link_libraries(Gametest PRIVATE
    OpenGL::GL     # предоставляет find_package(OpenGL)
    FreeGLUT::freeglut     # предоставляет find_package(GLUT)
)

# TODO: Add tests and install targets if needed.
