
cmake_minimum_required(VERSION 2.6)
project(DiagramBuilder2)
set(CMAKE_CXX_FLAGS "-g -O0 -Wall -Wno-sign-compare --std=gnu++0x")
# hack to fix -DQT_NO_DEBUG being injected into an unknown variable 
set(PCH_ADDL_FLAGS "-DQT_NO_DEBUG") 
include(User.cmake OPTIONAL)
set(CMAKE_AUTOMOC TRUE)

set(QT_USE_QTSVG TRUE)
set(QT_USE_QTOPENGL TRUE)
set(QT_USE_QTXML TRUE)
find_package(Boost COMPONENTS system date_time)
find_package(Qt4   REQUIRED)
add_definitions(
    ${QT_DEFINITIONS}
    -DQT_STATICPLUGIN
)
include(${QT_USE_FILE})

include_directories(
	${Boost_INCLUDE_DIR}
)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
)
include("${CMAKE_SOURCE_DIR}/PrecompiledHeader.cmake")

QT4_ADD_RESOURCES(RESOURCE_OUTPUT resources.qrc)
QT4_WRAP_UI(UI_OUTPUT MainWindowUI.ui Preferences.ui BasicPropertyWidget.ui ExportComponentOptions.ui)

add_executable(
	DiagramBuilder2 MACOSX_BUNDLE

	DiagramItem.cpp
	DiagramComponent.cpp
	DiagramItemModel.cpp
	DiagramScene.cpp
	UUIDMapper.cpp
	ComponentFileReader.cpp
	DiagramContext.cpp
	DiagramView.cpp
	Clipboard.cpp
	TestTableModel.cpp
	PropertyWidget.cpp
	BasicPropertyWidget.cpp
	PropertyBinder.cpp
	ColorButton.cpp
	
	TabFocus/TabFocusableTextItem.cpp
	TabFocus/TabFocusRing.cpp
	
	Commands/InsertItemCommand.cpp
	Commands/ConnectItemCommand.cpp
	Commands/DeleteItemCommand.cpp
	Commands/RotateItemCommand.cpp
	Commands/MoveItemCommand.cpp
	Commands/InsertItemsCommand.cpp
	
	DiagramIO/InfoDiagramLoader.cpp
	DiagramIO/ComponentFile.cpp

	Util/FunctionSlot.cpp
	Util/QtUtil.cpp
	Util/Log.cpp
	Util/TestLevenshtein.cpp
	Util/Demangle.cpp
	Util/Printable.cpp
	Util/Synchronizer.cpp

	
	Components/Util/HandleItem.cpp
	Components/ConnectorComponent.cpp
	Components/SVGComponent.cpp
	Components/TextComponent.cpp
	Components/PathComponent.cpp
	Components/Path/PathItem.cpp
	Components/PathConnectorComponent.cpp
	Components/BoxComponent.cpp
	Components/ImageComponent.cpp
	
	
	Main/GridMenuAction.cpp
	Main/GridMenu.cpp
	Main/Application.cpp
	Main/MainWindow.cpp
	Main/Toolbox.cpp
	Main/PreferencesDialog.cpp
	Main/GenericPropertyWidget.cpp
	Main/ExportComponentOptions.cpp
	
	ThirdParty/FlowLayout.cpp
	ThirdParty/LineEdit.cpp
	
	Plugin/BasicPlugin.cpp
	Plugin/DiagramComponentPlugin.cpp
	
    ${UI_OUTPUT}
	${RESOURCE_OUTPUT}
)


add_precompiled_header(DiagramBuilder2 Prefix.hpp) 
message(WARNING ${RESOURCE_OUTPUT})

set_target_properties(
	DiagramBuilder2 PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "bin"
	LIBRARY_OUTPUT_DIRECTORY "lib"
)

message("system: " ${CMAKE_SYSTEM_NAME})
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(ADDL_LIBS lcms lzma)
else()
	set(ADDL_LIBS)
endif()

target_link_libraries(
	DiagramBuilder2
	${Boost_LIBRARIES}
	${QT_LIBRARIES}
	${ADDL_LIBS}
)


#install(
#    TARGETS DiagramBuilder2
#    BUNDLE DESTINATION . COMPONENT Runtime
#)

#set(APPS "\${CMAKE_INSTALL_PREFIX}/DiagramBuilder2.app")
#install(
#    CODE "
#    include(BundleUtilities)
#    fixup_bundle(\"${APPS}\" \"\" \"\")
#    "
#    COMPONENT Runtime
#)

#set(CPACK_BINARY_DRAGNDROP ON)
#include(CPack)
