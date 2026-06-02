cmake_minimum_required(VERSION 3.16)
project(VideoStudio VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 QuickDialogs2 Widgets Multimedia)

add_executable(VideoStudio
    core/main.cpp
    core/utils/MediaImageProvider.cpp
    core/media/MediaListModel.cpp
    core/media/MediaManager.cpp
    core/media/MediaItem.cpp
    core/media/AudioWaveformProvider.cpp
    core/rendering/RenderEngine.cpp
    core/rendering/VideoRenderer.cpp
    core/timeline/TimelineClipModel.cpp
    core/timeline/TimelineManager.cpp
    core/timeline/Track.cpp
    core/timeline/Clip.cpp
)

set(QT_QML_GENERATE_QMLLS_INI ON)
set_source_files_properties(ui/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(VideoStudio
    URI "VideoStudioUI"
    VERSION 1.1
    QML_FILES
        ui/Theme.qml
        ui/Main.qml
        ui/header/HeaderMenu.qml
        ui/header/ExportDialog.qml
        ui/header/ExportUtils.js
        ui/header/EmptyTimelineDialog.qml
        ui/header/AppMenu.qml
        ui/header/AppMenuItem.qml
        ui/media_pool/MediaPool.qml
        ui/media_pool/MediaPoolFooter.qml
        ui/media_pool/EffectHub.qml
        ui/media_pool/AIAssist.qml
        ui/monitors/SourceMonitor.qml
        ui/monitors/ProgramMonitor.qml
        ui/monitors/LiveSubtitleOverlay.qml
        ui/monitors/RegionSelectorOverlay.qml
        ui/inspector/Inspector.qml
        ui/inspector/ColorWheel.qml
        ui/inspector/EffectSlider.qml
        ui/inspector/AccordionHeader.qml
        ui/inspector/BasicTab.qml
        ui/inspector/ColorTab.qml
        ui/inspector/BlurTab.qml
        ui/inspector/StylizeTab.qml
        ui/inspector/AudioTab.qml
        ui/inspector/NeonEqualizer.qml
        ui/timeline_view/TimelineView.qml
        ui/timeline_view/TimelineToolbar.qml
        ui/timeline_view/TimelineTracks.qml
        ui/timeline_view/TimelineCanvas.qml
        ui/timeline_view/TimelineClipItem.qml
        ui/timeline_view/TimelineHeader.qml
        ui/timeline_view/TimelineRuler.qml
        ui/timeline_view/TimelineTrackHeaders.qml
        ui/timeline_view/AudioMeter.qml
        ui/settings/TimelineSettingsMenu.qml
        ui/main_setting/MainSettings.qml
    RESOURCES
        assets/check.svg
        assets/chevron-down.svg
        assets/chevron-up.svg
        assets/cloud.svg
        assets/eye.svg
        assets/film.svg
        assets/folder.svg
        assets/grid-view.svg
        assets/keyboard.svg
        assets/link.svg
        assets/list-view.svg
        assets/lock.svg
        assets/marker-pin.svg
        assets/mic.svg
        assets/monitor.svg
        assets/monitor-play.svg
        assets/panel.svg
        assets/pause.svg
        assets/play.svg
        assets/redo.svg
        assets/scissors.svg
        assets/settings.svg
        assets/skip-back.svg
        assets/skip-forward.svg
        assets/step-back.svg
        assets/step-forward.svg
        assets/sync-lock.svg
        assets/split.svg
        assets/delete-left.svg
        assets/delete-right.svg
        assets/trash.svg
        assets/trim-handle.svg
        assets/tool-hand.svg
        assets/tool-magnet.svg
        assets/tool-pointer.svg
        assets/tool-razor.svg
        assets/tool-ripple.svg
        assets/tool-slip.svg
        assets/tool-zoom.svg
        assets/undo.svg
        assets/users.svg
        assets/volume-x.svg
        assets/trending_effect.png
        assets/retro_effect.png
        assets/party_effect.png
    SOURCES
        core/actions/ActionManager.cpp
        core/actions/ActionManager.h
        core/actions/MenuManager.cpp
        core/actions/MenuManager.h
        core/actions/FileActions.cpp
        core/actions/FileActions.h
        core/actions/EditActions.cpp
        core/actions/EditActions.h
        core/actions/ClipActions.cpp
        core/actions/ClipActions.h
        core/actions/SequenceActions.cpp
        core/actions/SequenceActions.h
        core/actions/ViewActions.cpp
        core/actions/ViewActions.h
        core/actions/ToolActions.cpp
        core/actions/ToolActions.h
        core/actions/HelpActions.cpp
        core/actions/HelpActions.h
        core/effects/EffectProcessor.cpp
        core/effects/EffectProcessor.h
        core/effects/EffectController.cpp
        core/effects/EffectController.h
        core/effects/models/TransformEffect.cpp
        core/effects/models/TransformEffect.h
        core/effects/models/ColorEffect.cpp
        core/effects/models/ColorEffect.h
        core/effects/models/BlurEffect.cpp
        core/effects/models/BlurEffect.h
        core/effects/models/StylizeEffect.cpp
        core/effects/models/StylizeEffect.h
        core/effects/models/AudioEffect.cpp
        core/effects/models/AudioEffect.h
        ui/inspector/InspectorController.cpp
        ui/inspector/InspectorController.h
        ui/media_pool/MediaPoolController.cpp
        ui/media_pool/MediaPoolController.h
        ui/timeline_view/TimelineController.cpp
        ui/timeline_view/TimelineController.h
        ui/timeline_view/ClipTrimController.cpp
        ui/timeline_view/ClipTrimController.h
        core/export/VideoExporter.cpp
        core/export/VideoExporter.h
        core/export/AdvancedSubtitleRenderer.cpp
        core/export/AdvancedSubtitleRenderer.h
        core/export/HardwareEncoder.h
        core/export/NvidiaEncoder.cpp
        core/export/NvidiaEncoder.h
        core/export/AmdEncoder.cpp
        core/export/AmdEncoder.h
        core/export/IntelEncoder.cpp
        core/export/IntelEncoder.h
        core/export/HardwareDetector.cpp
        core/export/HardwareDetector.h
        core/playback/AudioEngine.cpp
        core/playback/AudioEngine.h
        core/playback/PlaybackEngine.cpp
        core/playback/PlaybackEngine.h
        core/subtitle/SrtParser.cpp
        core/subtitle/SrtParser.h
        core/subtitle/SubtitleController.cpp
        core/subtitle/SubtitleController.h
)

target_link_libraries(VideoStudio PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::QuickDialogs2
    Qt6::Widgets
    Qt6::Multimedia
    avcodec
    avformat
    avfilter
    avutil
    swresample
    swscale
)

target_link_directories(VideoStudio PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ffmpeg/lib
)

target_include_directories(VideoStudio PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/inspector
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/media_pool
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/timeline_view
    ${CMAKE_CURRENT_SOURCE_DIR}/core/export
    ${CMAKE_CURRENT_SOURCE_DIR}/core/playback
    ${CMAKE_CURRENT_SOURCE_DIR}/core/subtitle
    ${CMAKE_CURRENT_SOURCE_DIR}/core/effects
    ${CMAKE_CURRENT_SOURCE_DIR}/core/effects/models
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ffmpeg/include
)

file(GLOB FFMPEG_RUNTIME_DLLS
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/ffmpeg/bin/*.dll"
)
file(GLOB FFMPEG_RUNTIME_TOOLS
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/ffmpeg/bin/*.exe"
)

if(FFMPEG_RUNTIME_DLLS)
    add_custom_command(TARGET VideoStudio POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FFMPEG_RUNTIME_DLLS}
            $<TARGET_FILE_DIR:VideoStudio>
        COMMENT "Copying FFmpeg runtime DLLs"
    )
endif()

if(FFMPEG_RUNTIME_TOOLS)
    add_custom_command(TARGET VideoStudio POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FFMPEG_RUNTIME_TOOLS}
            $<TARGET_FILE_DIR:VideoStudio>
        COMMENT "Copying FFmpeg command line tools"
    )
endif()

add_custom_command(TARGET VideoStudio POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Qt6::Widgets>
        $<TARGET_FILE:Qt6::Multimedia>
        $<TARGET_FILE:Qt6::QuickDialogs2>
        $<TARGET_FILE:Qt6::QuickDialogs2QuickImpl>
        $<TARGET_FILE:Qt6::QuickDialogs2Utils>
        $<TARGET_FILE_DIR:VideoStudio>
    COMMENT "Copying Qt runtime DLLs"
)

add_custom_command(TARGET VideoStudio POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${Qt6_DIR}/../../../qml/QtQuick/Dialogs"
        "$<TARGET_FILE_DIR:VideoStudio>/QtQuick/Dialogs"
    COMMENT "Copying QtQuick.Dialogs QML plugin"
)

add_custom_command(TARGET VideoStudio POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:VideoStudio>/multimedia"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${Qt6_DIR}/../../../plugins/multimedia"
        "$<TARGET_FILE_DIR:VideoStudio>/multimedia"
    COMMENT "Copying Qt Multimedia plugins"
)

add_executable(MediaItemProbe EXCLUDE_FROM_ALL
    tests/MediaItemProbe.cpp
    core/media/MediaItem.cpp
)

target_link_libraries(MediaItemProbe PRIVATE
    Qt6::Core
    Qt6::Gui
    avcodec
    avformat
    avutil
    swscale
)

target_link_directories(MediaItemProbe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ffmpeg/lib
)

target_include_directories(MediaItemProbe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ffmpeg/include
)

if(FFMPEG_RUNTIME_DLLS)
    add_custom_command(TARGET MediaItemProbe POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FFMPEG_RUNTIME_DLLS}
            $<TARGET_FILE_DIR:MediaItemProbe>
        COMMENT "Copying FFmpeg runtime DLLs for MediaItemProbe"
    )
endif()

add_executable(PlaybackProbe EXCLUDE_FROM_ALL
    tests/PlaybackProbe.cpp
    core/playback/AudioEngine.cpp
    core/playback/PlaybackEngine.cpp
    core/effects/EffectProcessor.cpp
)

target_link_libraries(PlaybackProbe PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Multimedia
    avcodec
    avformat
    avutil
    swresample
    swscale
)

target_link_directories(PlaybackProbe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ffmpeg/lib
)

target_include_directories(PlaybackProbe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core/playback
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ffmpeg/include
)

if(FFMPEG_RUNTIME_DLLS)
    add_custom_command(TARGET PlaybackProbe POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FFMPEG_RUNTIME_DLLS}
            $<TARGET_FILE_DIR:PlaybackProbe>
        COMMENT "Copying FFmpeg runtime DLLs for PlaybackProbe"
    )
endif()

add_custom_command(TARGET PlaybackProbe POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Qt6::Multimedia>
        $<TARGET_FILE_DIR:PlaybackProbe>
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:PlaybackProbe>/multimedia"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${Qt6_DIR}/../../../plugins/multimedia"
        "$<TARGET_FILE_DIR:PlaybackProbe>/multimedia"
    COMMENT "Copying Qt Multimedia runtime for PlaybackProbe"
)

add_executable(TimelineProbe EXCLUDE_FROM_ALL
    tests/TimelineProbe.cpp
    ui/timeline_view/TimelineController.cpp
    ui/timeline_view/TimelineController.h
    core/timeline/TimelineClipModel.cpp
    core/timeline/TimelineClipModel.h
)

target_link_libraries(TimelineProbe PRIVATE
    Qt6::Core
    Qt6::Qml
)

target_include_directories(TimelineProbe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/timeline_view
    ${CMAKE_CURRENT_SOURCE_DIR}/core/export
)

add_executable(TimelineQmlProbe EXCLUDE_FROM_ALL
    tests/TimelineQmlProbe.cpp
    ui/timeline_view/TimelineController.cpp
    ui/timeline_view/TimelineController.h
    ui/timeline_view/ClipTrimController.cpp
    ui/timeline_view/ClipTrimController.h
    core/timeline/TimelineClipModel.cpp
    core/timeline/TimelineClipModel.h
)

target_link_libraries(TimelineQmlProbe PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)

target_include_directories(TimelineQmlProbe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/timeline_view
    ${CMAKE_CURRENT_SOURCE_DIR}/core/export
)

add_executable(ExportProbe EXCLUDE_FROM_ALL
    tests/ExportProbe.cpp
    core/export/VideoExporter.cpp
    core/export/VideoExporter.h
    core/export/AdvancedSubtitleRenderer.cpp
    core/export/AdvancedSubtitleRenderer.h
    core/export/HardwareEncoder.h
    core/export/NvidiaEncoder.cpp
    core/export/NvidiaEncoder.h
    core/export/AmdEncoder.cpp
    core/export/AmdEncoder.h
    core/export/IntelEncoder.cpp
    core/export/IntelEncoder.h
    core/export/HardwareDetector.cpp
    core/export/HardwareDetector.h
    core/subtitle/SrtParser.cpp
    core/subtitle/SrtParser.h
    ui/timeline_view/TimelineController.cpp
    ui/timeline_view/TimelineController.h
    core/timeline/TimelineClipModel.cpp
    core/timeline/TimelineClipModel.h
)

target_link_libraries(ExportProbe PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Widgets
)

target_include_directories(ExportProbe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core/export
    ${CMAKE_CURRENT_SOURCE_DIR}/core/subtitle
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/timeline_view
)

if(FFMPEG_RUNTIME_DLLS)
    add_custom_command(TARGET ExportProbe POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FFMPEG_RUNTIME_DLLS}
            $<TARGET_FILE_DIR:ExportProbe>
        COMMENT "Copying FFmpeg runtime DLLs for ExportProbe"
    )
endif()

if(FFMPEG_RUNTIME_TOOLS)
    add_custom_command(TARGET ExportProbe POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FFMPEG_RUNTIME_TOOLS}
            $<TARGET_FILE_DIR:ExportProbe>
        COMMENT "Copying FFmpeg command line tools for ExportProbe"
    )
endif()

set_property(TARGET VideoStudio PROPERTY WIN32_EXECUTABLE FALSE)
