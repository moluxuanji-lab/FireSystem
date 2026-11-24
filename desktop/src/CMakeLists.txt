set(APP_TARGET ${PROJECT_NAME}App)

qt_add_executable(${APP_TARGET}
    main.cpp
)

qt_add_qml_module(${APP_TARGET}
    URI FireSystem.UI
    VERSION 1.0
    QML_FILES
        Main.qml
)

set_target_properties(${APP_TARGET} PROPERTIES
    OUTPUT_NAME "${PROJECT_NAME}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_compile_definitions(${APP_TARGET}
    PRIVATE
        APP_NAME="${PROJECT_NAME}"
        APP_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(${APP_TARGET}
    PRIVATE
        Qt6::Quick
        Qt6::QuickControls2
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
