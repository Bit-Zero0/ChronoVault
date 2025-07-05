cmake_minimum_required(VERSION 3.5)

project(ChronoVault VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# 所有的源文件路径都是相对于当前 CMakeLists.txt 的，所以不需要前缀
set(PROJECT_SOURCES
    main.cpp
    core/TodoItem.h
    gui/addtododialog.cpp
    gui/MainWindow.cpp
    gui/todolistitemwidget.cpp

    core/TodoItem.h
    core/Anniversary.h
    core/SharedFile.h
    core/TodoList.h             # <-- 添加

    # Services
    services/TodoService.h       # <-- 添加
    services/TodoService.cpp     # <-- 添加


)

# 资源文件也一样
set(PROJECT_RESOURCES
    icons.qrc
)

# 根据Qt版本创建可执行文件
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ChronoVault
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${PROJECT_RESOURCES}
        core/TodoList.h
        gui/taskdetailwidget.h gui/taskdetailwidget.cpp
        gui/subtaskitemwidget.h gui/subtaskitemwidget.cpp
        gui/TodoListNameWidget.h gui/TodoListNameWidget.cpp
    )
else()
    add_executable(ChronoVault
        ${PROJECT_SOURCES}
        ${PROJECT_RESOURCES}
    )
endif()

# 核心步骤：将当前目录 (即 CMakeLists.txt 所在的目录) 添加到头文件搜索路径中
target_include_directories(ChronoVault PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接Qt库
target_link_libraries(ChronoVault PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)


# --- 以下为平台相关的设置，保持不变 ---

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ChronoVault)
endif()
set_target_properties(ChronoVault PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ChronoVault
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ChronoVault)
endif()
