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
    core/media/models/MediaListModel.cpp
    core/media/managers/MediaManager.cpp
    core/media/models/MediaItem.cpp
    core/media/providers/AudioWaveformProvider.cpp
    core/rendering/RenderEngine.cpp
    core/rendering/VideoRenderer.cpp
    core/timeline/models/TimelineClipModel.cpp
    core/timeline/managers/TimelineManager.cpp
    core/timeline/models/Track.cpp
    core/timeline/models/Clip.cpp
)

set(QT_QML_GENERATE_QMLLS_INI ON)
set_source_files_properties(ui/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(VideoStudio
    URI "VideoStudioUI"
    VERSION 1.1
    QML_FILES
        ui/Theme.qml
        ui/Main.qml
        ui/header/menus/HeaderMenu.qml
        ui/header/dialogs/ExportDialog.qml
        ui/header/utils/ExportUtils.js
        ui/header/dialogs/EmptyTimelineDialog.qml
        ui/header/menus/AppMenu.qml
        ui/header/menus/AppMenuItem.qml
        ui/media_pool/MediaPool.qml
        ui/media_pool/MediaPoolFooter.qml
        ui/media_pool/EffectHub.qml
        ui/media_pool/AIAssist.qml
        ui/monitors/SourceMonitor.qml
        ui/monitors/ProgramMonitor.qml
        ui/monitors/overlays/LiveSubtitleOverlay.qml
        ui/monitors/overlays/RegionSelectorOverlay.qml
        ui/inspector/Inspector.qml
        ui/inspector/components/ColorWheel.qml
        ui/inspector/components/EffectSlider.qml
        ui/inspector/components/AccordionHeader.qml
        ui/inspector/tabs/BasicTab.qml
        ui/inspector/tabs/ColorTab.qml
        ui/inspector/tabs/BlurTab.qml
        ui/inspector/tabs/StylizeTab.qml
        ui/inspector/tabs/AudioTab.qml
        ui/inspector/components/NeonEqualizer.qml
        ui/timeline_view/TimelineView.qml
        ui/timeline_view/components/TimelineToolbar.qml
        ui/timeline_view/TimelineTracks.qml
        ui/timeline_view/components/TimelineCanvas.qml
        ui/timeline_view/components/TimelineClipItem.qml
        ui/timeline_view/components/TimelineClipContextMenu.qml
        ui/timeline_view/components/TimelineHeader.qml
        ui/timeline_view/components/TimelineRuler.qml
        ui/timeline_view/components/TimelineTrackHeaders.qml
        ui/timeline_view/components/AudioMeter.qml
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
        core/actions/managers/ActionManager.cpp
        core/actions/managers/ActionManager.h
        core/actions/managers/MenuManager.cpp
        core/actions/managers/MenuManager.h
        core/actions/handlers/FileActions.cpp
        core/actions/handlers/FileActions.h
        core/actions/handlers/EditActions.cpp
        core/actions/handlers/EditActions.h
        core/actions/handlers/ClipActions.cpp
        core/actions/handlers/ClipActions.h
        core/actions/handlers/SequenceActions.cpp
        core/actions/handlers/SequenceActions.h
        core/actions/handlers/ViewActions.cpp
        core/actions/handlers/ViewActions.h
        core/actions/handlers/ToolActions.cpp
        core/actions/handlers/ToolActions.h
        core/actions/handlers/HelpActions.cpp
        core/actions/handlers/HelpActions.h
        core/effects/processors/EffectProcessor.cpp
        core/effects/processors/EffectProcessor.h
        core/effects/controllers/EffectController.cpp
        core/effects/controllers/EffectController.h
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
        core/vocal/VocalIsolator.cpp
        core/vocal/VocalIsolator.h
        ui/inspector/controllers/InspectorController.cpp
        ui/inspector/controllers/InspectorController.h
        ui/media_pool/MediaPoolController.cpp
        ui/media_pool/MediaPoolController.h
        core/timeline/commands/TimelineCommands.cpp
        core/timeline/commands/TimelineCommands.h
        ui/timeline_view/controllers/TimelineController.cpp
        ui/timeline_view/controllers/TimelineController.h
        ui/timeline_view/controllers/ClipTrimController.cpp
        ui/timeline_view/controllers/ClipTrimController.h
        core/export/exporters/VideoExporter.cpp
        core/export/exporters/VideoExporter.h
        core/export/renderers/AdvancedSubtitleRenderer.cpp
        core/export/renderers/AdvancedSubtitleRenderer.h
        core/export/encoders/HardwareEncoder.h
        core/export/encoders/NvidiaEncoder.cpp
        core/export/encoders/NvidiaEncoder.h
        core/export/encoders/AmdEncoder.cpp
        core/export/encoders/AmdEncoder.h
        core/export/encoders/IntelEncoder.cpp
        core/export/encoders/IntelEncoder.h
        core/export/encoders/HardwareDetector.cpp
        core/export/encoders/HardwareDetector.h
        core/playback/engines/AudioEngine.cpp
        core/playback/engines/AudioEngine.h
        core/playback/engines/PlaybackEngine.cpp
        core/playback/engines/PlaybackEngine.h
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
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/inspector/controllers
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/media_pool
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/timeline_view
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/timeline_view/controllers
    ${CMAKE_CURRENT_SOURCE_DIR}/core/export
    ${CMAKE_CURRENT_SOURCE_DIR}/core/export/exporters
    ${CMAKE_CURRENT_SOURCE_DIR}/core/playback
    ${CMAKE_CURRENT_SOURCE_DIR}/core/playback/engines
    ${CMAKE_CURRENT_SOURCE_DIR}/core/subtitle
    ${CMAKE_CURRENT_SOURCE_DIR}/core/effects
    ${CMAKE_CURRENT_SOURCE_DIR}/core/effects/controllers
    ${CMAKE_CURRENT_SOURCE_DIR}/core/effects/models
    ${CMAKE_CURRENT_SOURCE_DIR}/core/vocal
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
        "${CMAKE_CURRENT_SOURCE_DIR}/core/vocal/demucs_runner.py"
        "$<TARGET_FILE_DIR:VideoStudio>/demucs_runner.py"
    COMMENT "Copying Demucs runner"
)

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
    core/media/models/MediaItem.cpp
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
    core/playback/engines/AudioEngine.cpp
    core/playback/engines/PlaybackEngine.cpp
    core/effects/processors/EffectProcessor.cpp
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
    core/timeline/commands/TimelineCommands.cpp
    core/timeline/commands/TimelineCommands.h
    ui/timeline_view/controllers/TimelineController.cpp
    ui/timeline_view/controllers/TimelineController.h
    core/actions/managers/ActionManager.cpp
    core/actions/managers/ActionManager.h
    core/vocal/VocalIsolator.cpp
    core/vocal/VocalIsolator.h
    core/timeline/models/TimelineClipModel.cpp
    core/timeline/models/TimelineClipModel.h
)

target_link_libraries(TimelineProbe PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
)

target_include_directories(TimelineProbe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/timeline_view
    ${CMAKE_CURRENT_SOURCE_DIR}/core/export
)

add_executable(TimelineQmlProbe EXCLUDE_FROM_ALL
    tests/TimelineQmlProbe.cpp
    core/timeline/commands/TimelineCommands.cpp
    core/timeline/commands/TimelineCommands.h
    ui/timeline_view/controllers/TimelineController.cpp
    ui/timeline_view/controllers/TimelineController.h
    ui/timeline_view/controllers/ClipTrimController.cpp
    ui/timeline_view/controllers/ClipTrimController.h
    core/actions/managers/ActionManager.cpp
    core/actions/managers/ActionManager.h
    core/vocal/VocalIsolator.cpp
    core/vocal/VocalIsolator.h
    core/timeline/models/TimelineClipModel.cpp
    core/timeline/models/TimelineClipModel.h
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
    core/export/exporters/VideoExporter.cpp
    core/export/exporters/VideoExporter.h
    core/export/renderers/AdvancedSubtitleRenderer.cpp
    core/export/renderers/AdvancedSubtitleRenderer.h
    core/export/encoders/HardwareEncoder.h
    core/export/encoders/NvidiaEncoder.cpp
    core/export/encoders/NvidiaEncoder.h
    core/export/encoders/AmdEncoder.cpp
    core/export/encoders/AmdEncoder.h
    core/export/encoders/IntelEncoder.cpp
    core/export/encoders/IntelEncoder.h
    core/export/encoders/HardwareDetector.cpp
    core/export/encoders/HardwareDetector.h
    core/subtitle/SrtParser.cpp
    core/subtitle/SrtParser.h
    core/timeline/commands/TimelineCommands.cpp
    core/timeline/commands/TimelineCommands.h
    ui/timeline_view/controllers/TimelineController.cpp
    ui/timeline_view/controllers/TimelineController.h
    core/timeline/models/TimelineClipModel.cpp
    core/timeline/models/TimelineClipModel.h
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
