add_executable(Scheduler 
    "main.cpp" 
    "src/Scheduler.cpp"       # Добавить эту строку!
    "include/Scheduler.h"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Scheduler PROPERTY CXX_STANDARD 20)
endif()

target_include_directories(
    Scheduler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include 
)