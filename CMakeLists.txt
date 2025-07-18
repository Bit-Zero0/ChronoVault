cmake_minimum_required(VERSION 3.5)

project(ChronoVault VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 使用一个清晰、完整的命令找到所有需要的 Qt6 模块
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Concurrent)

# 定义源文件列表
set(PROJECT_SOURCES
    main.cpp
    # Core
    core/TodoItem.h
    core/TodoItem.cpp
    core/TodoList.h
    core/TodoList.cpp
    core/AnniversaryItem.h
    core/SharedFile.h
    # Services
    services/TodoService.h
    services/TodoService.cpp
    services/AnniversaryService.h
    services/AnniversaryService.cpp
    # GUI
    gui/MainWindow.h
    gui/MainWindow.cpp
    gui/addtododialog.cpp
    gui/todolistitemwidget.h
    gui/todolistitemwidget.cpp
    gui/taskdetailwidget.h
    gui/taskdetailwidget.cpp
    gui/subtaskitemwidget.h
    gui/subtaskitemwidget.cpp
    gui/TodoListNameWidget.h
    gui/TodoListNameWidget.cpp
    gui/completedheaderwidget.h
    gui/completedheaderwidget.cpp
    gui/AnniversaryItemWidget.h gui/AnniversaryItemWidget.cpp
)

# 定义资源文件列表
set(PROJECT_RESOURCES
    icons.qrc
)

# 创建可执行文件
qt_add_executable(ChronoVault
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    ${PROJECT_RESOURCES}

    gui/AddAnniversaryDialog.h gui/AddAnniversaryDialog.cpp
    core/Reminder.h
    gui/TodoListSelectionDialog.h gui/TodoListSelectionDialog.cpp
    gui/ReminderSettingsDialog.h gui/ReminderSettingsDialog.cpp
    core/Reminder.cpp
    core/Moment.h core/Moment.cpp
    gui/AddMomentDialog.h gui/AddMomentDialog.cpp
    gui/AnniversaryDetailView.h gui/AnniversaryDetailView.cpp
    gui/MomentCardWidget.h gui/MomentCardWidget.cpp
    gui/MomentDetailDialog.h gui/MomentDetailDialog.cpp
    gui/ImageViewerDialog.h gui/ImageViewerDialog.cpp
    gui/ClickableLabel.h gui/ClickableLabel.cpp
    gui/ClickableTextBrowser.h




)

# 添加头文件搜索路径
target_include_directories(ChronoVault PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接所有需要的 Qt 库
target_link_libraries(ChronoVault PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Concurrent
)

# --- 以下为平台相关的设置 ---

# 【已修正】使用正确的 Qt6_VERSION 变量进行版本判断
if(Qt6_VERSION VERSION_LESS "6.1.0")
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
