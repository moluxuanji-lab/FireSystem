set(CORE_TARGET ${PROJECT_NAME}Core)
set(UI_TARGET ${PROJECT_NAME}UI)
set(APP_TARGET ${PROJECT_NAME}App)

qt_add_library(${CORE_TARGET} STATIC
    core/I18nCatalog.h
    core/I18nCatalog.cpp
    core/AccountManager.h
    core/AccountManager.cpp
    core/OfflineFileManager.h
    core/OfflineFileManager.cpp
    core/PreferencesManager.h
    core/PreferencesManager.cpp
    core/TemplateReviewManager.h
    core/TemplateReviewManager.cpp
    core/SettingsService.h
    core/SettingsService.cpp
)

target_include_directories(${CORE_TARGET}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(${CORE_TARGET}
    PUBLIC
        Qt6::Core
        Qt6::Network
)

set(QML_IMPORT_PATH
    "${CMAKE_BINARY_DIR}/qml"
    "${CMAKE_BINARY_DIR}/third_party/FluentUI/src"
    CACHE STRING "QML import paths" FORCE
)

qt_add_library(${UI_TARGET} STATIC)

set_source_files_properties(qml/components/WorkbenchStyle.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(${UI_TARGET}
    URI FireSystem.UI
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml/FireSystem/UI"
    IMPORT_PATH "${CMAKE_BINARY_DIR}/third_party/FluentUI/src"
    IMPORTS
        FluentUI
    RESOURCES
        qml/components/firesystem.svg
    QML_FILES
        Main.qml
        FluentUIDeps.qml
        qml/components/SectionCard.qml
        qml/components/KeyValueList.qml
        qml/components/NoticeBanner.qml
        qml/components/StatusChip.qml
        qml/components/PageHeader.qml
        qml/components/WorkbenchButton.qml
        qml/components/WorkbenchComboBox.qml
        qml/components/WorkbenchGlyphs.js
        qml/components/WorkbenchHeader.qml
        qml/components/WorkbenchIcon.qml
        qml/components/WorkbenchNavButton.qml
        qml/components/WorkbenchPage.qml
        qml/components/WorkbenchSidebar.qml
        qml/components/WorkbenchStyle.qml
        qml/pages/LoginPage.qml
        qml/pages/SettingsPage.qml
        qml/pages/SettingsAccountsPage.qml
        qml/pages/SettingsLicensePage.qml
        qml/pages/SettingsTemplatesPage.qml
        qml/pages/SettingsGeneralPage.qml
        qml/pages/SettingsAboutPage.qml
)

target_link_libraries(${UI_TARGET}
    PUBLIC
        ${CORE_TARGET}
        Qt6::Quick
        Qt6::QuickControls2
        fluentui
        fluentuiplugin
)

qt_add_executable(${APP_TARGET}
    main.cpp
)

set_target_properties(${APP_TARGET} PROPERTIES
    OUTPUT_NAME "${PROJECT_NAME}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_include_directories(${APP_TARGET}
    PRIVATE
        "${CMAKE_SOURCE_DIR}/third_party/FluentUI/src"
)

target_compile_definitions(${APP_TARGET}
    PRIVATE
        APP_NAME="${PROJECT_NAME}"
        APP_VERSION="${PROJECT_VERSION}"
        FLUENTUI_BUILD_STATIC_LIB
)

target_link_libraries(${APP_TARGET}
    PRIVATE
        ${CORE_TARGET}
        ${UI_TARGET}
        ${UI_TARGET}plugin
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Widgets
        Qt6::PrintSupport
        fluentui
        fluentuiplugin
)

if(WIN32)
    set(APP_INSTALL_BINDIR ".")
else()
    set(APP_INSTALL_BINDIR "${CMAKE_INSTALL_BINDIR}")
endif()

install(TARGETS ${APP_TARGET}
    COMPONENT App
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${APP_INSTALL_BINDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET ${APP_TARGET}
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

if(WIN32)
    install(CODE "set(QT_DEPLOY_BIN_DIR \".\")" COMPONENT App)
endif()

install(SCRIPT ${deploy_script} COMPONENT App)
