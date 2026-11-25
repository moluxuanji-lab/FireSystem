find_package(Qt6 6.8 REQUIRED COMPONENTS Test QuickTest)

add_executable(tst_settings_core
    tst_settings_core.cpp
)

target_link_libraries(tst_settings_core
    PRIVATE
        FireSystemCore
        Qt6::Core
        Qt6::Network
        Qt6::Test
)

add_test(NAME tst_settings_core COMMAND tst_settings_core -o -,txt)

add_executable(tst_settings_ui
    qml/tst_settings_main.cpp
)

target_include_directories(tst_settings_ui
    PRIVATE
        "${CMAKE_SOURCE_DIR}/third_party/FluentUI/src"
)

target_compile_definitions(tst_settings_ui
    PRIVATE
        FLUENTUI_BUILD_STATIC_LIB
        QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/qml"
)

target_link_libraries(tst_settings_ui
    PRIVATE
        FireSystemCore
        FireSystemUI
        FireSystemUIplugin
        Qt6::Core
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Widgets
        Qt6::QuickTest
        fluentui
        fluentuiplugin
)

add_test(NAME tst_settings_ui
    COMMAND tst_settings_ui -input "${CMAKE_CURRENT_SOURCE_DIR}/qml" -o -,txt
)

set_tests_properties(tst_settings_ui PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_CONTROLS_STYLE=Basic"
)
