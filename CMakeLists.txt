cmake_minimum_required(VERSION 3.16)
project(VideoStudio VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 QuickDialogs2 Widgets Multimedia)

# Disable VST3 SDK sample applications and validator to prevent target collisions
set(SMTG_ENABLE_VST3_HOSTING_EXAMPLES OFF CACHE BOOL "Disable VST3 hosting samples" FORCE)
set(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES OFF CACHE BOOL "Disable VST3 plugin samples" FORCE)
set(SMTG_ENABLE_VSTGUI_SUPPORT OFF CACHE BOOL "Disable VSTGUI support" FORCE)
set(SMTG_RUN_VST_VALIDATOR OFF CACHE BOOL "Disable VST validator" FORCE)

include(FetchContent)



# 1. rLottie (Vector Animation Engine)
FetchContent_Declare(rlottie
    GIT_REPOSITORY https://github.com/Samsung/rlottie.git
    GIT_TAG master
)

# 2. Lua & Sol2 (Scripting Engine for Effect Chains)
FetchContent_Declare(lua
    GIT_REPOSITORY https://github.com/marovira/lua.git
    GIT_TAG master
)
FetchContent_Declare(sol2
    GIT_REPOSITORY https://github.com/ThePhD/sol2.git
    GIT_TAG v3.3.0
)

# 3. Steinberg VST3 SDK (Professional Audio DSP)
FetchContent_Declare(vst3sdk
    GIT_REPOSITORY https://github.com/steinbergmedia/vst3sdk.git
    GIT_TAG master
)

# 4. Modern C++ JSON (For Lottie and Config Parsing)
FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

FetchContent_MakeAvailable(lua sol2 rlottie json vst3sdk)


add_executable(VideoStudio
    app.rc
    core/main.cpp
    crash/CrashHandler.cpp
    core/utils/MediaImageProvider.cpp
    core/media/models/MediaListModel.cpp
    core/media/managers/MediaManager.cpp
    core/media/models/MediaItem.cpp
    core/media/providers/AudioWaveformProvider.cpp
    core/media/components/HardwareWaveformItem.cpp
    core/rendering/RenderEngine.cpp
    core/rendering/VideoRenderer.cpp
    core/timeline/models/TimelineClipModel.cpp
    core/timeline/managers/TimelineManager.cpp
    core/timeline/models/Track.cpp
    core/timeline/models/Clip.cpp
)

if(WIN32)
    add_custom_target(StopVideoStudio ALL
        COMMAND taskkill /F /IM VideoStudio.exe /T 2> NUL || cd .
    )
    add_dependencies(VideoStudio StopVideoStudio)
endif()

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
        ui/header/dialogs/CoverEditDialog.qml
        ui/header/dialogs/CoverSelectDialog.qml
        ui/header/dialogs/feature/CoverColorPicker.qml
        ui/header/dialogs/feature/Divider.qml
        ui/header/dialogs/feature/FeatureSlider.qml
        ui/header/dialogs/feature/FeatureSwitch.qml
        ui/header/dialogs/feature/GeglOperationsCard.qml
        ui/header/dialogs/feature/InteractiveToolsCard.qml
        ui/header/dialogs/feature/PaintDynamicsCard.qml
        ui/header/dialogs/feature/PdbPluginsCard.qml
        ui/header/dialogs/feature/XcfProjectCard.qml
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
        ui/inspector/tabs/ChromaTab.qml
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
        ui/timeline_view/components/TimelineTrackControl.qml
        ui/timeline_view/components/TrackGapBox.qml
        ui/timeline_view/components/AudioMeter.qml
        ui/settings/TimelineSettingsMenu.qml
        ui/main_setting/MainSettings.qml
        ui/main_setting/components/SettingsCard.qml
        ui/main_setting/components/SettingsColorRow.qml
        ui/main_setting/components/SettingsComboBoxRow.qml
        ui/main_setting/components/SettingsRow.qml
        ui/main_setting/components/SettingsToggleRow.qml
        ui/main_setting/components/ShortcutRow.qml
        ui/main_setting/components/SidebarItem.qml
    RESOURCES
        assets/track-video.svg
        assets/track-audio.svg
        assets/track-subtitle.svg
        assets/ai-voice.svg
        assets/check.svg
        assets/chevron-down.svg
        assets/chevron-up.svg
        assets/cloud.svg
        assets/eye.svg
        assets/eye-off.svg
        assets/edit-2.svg
        assets/film.svg
        assets/folder.svg
        assets/grid-view.svg
        assets/keyboard.svg
        assets/link.svg
        assets/list-view.svg
        assets/lock.svg
        assets/unlock.svg
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
        assets/volume-2.svg
        assets/volume-x.svg
        assets/more-horizontal.svg
        assets/arrow-left.svg
        assets/cpu.svg
        assets/grid.svg
        assets/sliders.svg
        assets/star.svg
        assets/sticker-boom.svg
        assets/sticker-fire.svg
        assets/sticker-pin.svg
        assets/sticker-sparkle.svg
        assets/trim-handle-left.svg
        assets/trim-handle-right.svg
        assets/type.svg
        assets/x.svg
        assets/trending_effect.png
        assets/retro_effect.png
        assets/party_effect.png
        assets/app_icon.png
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
        core/effect_hub/models/EffectHubModel.cpp
        core/effect_hub/models/EffectHubModel.h
        core/effects/models/TransformEffect.cpp
        core/effects/models/TransformEffect.h
        core/effects/models/ChromaKeyEffect.cpp
        core/effects/models/ChromaKeyEffect.h
        core/effects/models/ColorEffect.cpp
        core/effects/models/ColorEffect.h
        core/effects/models/BlurEffect.cpp
        core/effects/models/BlurEffect.h
        core/effects/models/StylizeEffect.cpp
        core/effects/models/StylizeEffect.h
        core/effects/models/AudioEffect.cpp
        core/effects/models/AudioEffect.h
        core/vocal/AIVoiceGenerator.cpp
        core/vocal/AIVoiceGenerator.h
        core/vocal/VocalIsolator.cpp
        core/vocal/VocalIsolator.h
        core/vocal/engine/io/WavProcessor.cpp
        core/vocal/engine/io/WavProcessor.h
        core/vocal/engine/io/MediaAudioDecoder.cpp
        core/vocal/engine/io/MediaAudioDecoder.h
        core/vocal/engine/dsp/VocalRemover.cpp
        core/vocal/engine/dsp/VocalRemover.h
        core/vocal/engine/dsp/Fft.cpp
        core/vocal/engine/dsp/Fft.h
        core/vocal/engine/ai/AiVocalModel.cpp
        core/vocal/engine/ai/AiVocalModel.h
        ui/inspector/controllers/InspectorController.cpp
        ui/inspector/controllers/InspectorController.h
        ui/media_pool/MediaPoolController.cpp
        ui/media_pool/MediaPoolController.h
        core/timeline/commands/TimelineCommands.cpp
        core/timeline/commands/TimelineCommands.h
        core/timeline/controllers/TimelineAutoEditor.cpp
        core/timeline/controllers/TimelineAutoEditor.h
        core/project/ProjectManager.cpp
        core/project/ProjectManager.h
        ui/timeline_view/controllers/TimelineController.cpp
        ui/timeline_view/controllers/TimelineController.h
        ui/timeline_view/controllers/ClipTrimController.cpp
        ui/timeline_view/controllers/ClipTrimController.h
        ui/timeline_view/controllers/ClipDragController.cpp
        ui/timeline_view/controllers/ClipDragController.h
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
        core/editor/gegl/GeglGraphNode.cpp
        core/editor/gegl/GeglGraphNode.h
        core/editor/gegl/GeglOperationEngine.cpp
        core/editor/gegl/GeglOperationEngine.h
        core/editor/gegl/GeglOperationManager.cpp
        core/editor/gegl/GeglOperationManager.h
        core/editor/gegl/ColorAdjustmentOperation.cpp
        core/editor/gegl/ColorAdjustmentOperation.h
        core/editor/gegl/GenerativeFillOperation.cpp
        core/editor/gegl/GenerativeFillOperation.h
        core/editor/gegl/TransformDeformOperation.cpp
        core/editor/gegl/TransformDeformOperation.h
        core/editor/io/XcfFileStreamParser.cpp
        core/editor/io/XcfFileStreamParser.h
        core/editor/io/XcfProjectManager.cpp
        core/editor/io/XcfProjectManager.h
        core/editor/models/EditorImageModel.cpp
        core/editor/models/EditorImageModel.h
        core/editor/models/EditorLayerModel.cpp
        core/editor/models/EditorLayerModel.h
        core/editor/models/EditorUndoStack.cpp
        core/editor/models/EditorUndoStack.h
        core/editor/paint/PaintDynamicsEngine.cpp
        core/editor/paint/PaintDynamicsEngine.h
        core/editor/plugins/PdbProcedureManager.cpp
        core/editor/plugins/PdbProcedureManager.h
        core/editor/plugins/PdbRegistry.cpp
        core/editor/plugins/PdbRegistry.h
        core/editor/plugins/PluginIpcManager.cpp
        core/editor/plugins/PluginIpcManager.h
        core/editor/tools/SelectionToolController.cpp
        core/editor/tools/SelectionToolController.h
        core/editor/tools/ToolController.cpp
        core/editor/tools/ToolController.h
        core/editor/tools/ToolOptionsModel.cpp
        core/editor/tools/ToolOptionsModel.h
        core/editor/tools/TransformToolController.cpp
        core/editor/tools/TransformToolController.h
        core/editor/tools/UtilityToolController.cpp
        core/editor/tools/UtilityToolController.h
        core/editor/display/DisplayViewportModel.cpp
        core/editor/display/DisplayViewportModel.h
        core/editor/EditorBackendController.cpp
        core/editor/EditorBackendController.h
        core/effects/processors/GifFrameDecoder.cpp
        core/effects/processors/GifFrameDecoder.h
        core/effects/processors/GimpProcessor.cpp
        core/effects/processors/GimpProcessor.h
        core/setting/general/GeneralSettings.cpp
        core/setting/general/GeneralSettings.h
        core/setting/appearance/AppearanceSettings.cpp
        core/setting/appearance/AppearanceSettings.h
        core/setting/timeline/TimelineSettings.cpp
        core/setting/timeline/TimelineSettings.h
        core/setting/playback/PlaybackSettings.cpp
        core/setting/playback/PlaybackSettings.h
        core/setting/audio/AudioSettings.cpp
        core/setting/audio/AudioSettings.h
        core/setting/cache/CacheSettings.cpp
        core/setting/cache/CacheSettings.h
        core/setting/keyboard/KeyboardSettings.cpp
        core/setting/keyboard/KeyboardSettings.h
        core/setting/SettingsController.cpp
        core/setting/SettingsController.h
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
    Qt6::Concurrent
    avcodec
    avformat
    avfilter
    avutil
    swresample
    swscale
    onnxruntime
    dbghelp
    lua_static
    sol2
)

target_link_directories(VideoStudio PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ffmpeg/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/onnxruntime/lib
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
    ${CMAKE_CURRENT_SOURCE_DIR}/core/project
    ${CMAKE_CURRENT_SOURCE_DIR}/core/playback
    ${CMAKE_CURRENT_SOURCE_DIR}/core/playback/engines
    ${CMAKE_CURRENT_SOURCE_DIR}/core/subtitle
    ${CMAKE_CURRENT_SOURCE_DIR}/core/effect_hub/models
    ${CMAKE_CURRENT_SOURCE_DIR}/core/effects
    ${CMAKE_CURRENT_SOURCE_DIR}/core/effects/controllers
    ${CMAKE_CURRENT_SOURCE_DIR}/core/effects/models
    ${CMAKE_CURRENT_SOURCE_DIR}/core/vocal
    ${CMAKE_CURRENT_SOURCE_DIR}/core/vocal/engine
    ${CMAKE_CURRENT_SOURCE_DIR}/core/vocal/engine/ai
    ${CMAKE_CURRENT_SOURCE_DIR}/core/vocal/engine/dsp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/vocal/engine/io
    ${CMAKE_CURRENT_SOURCE_DIR}/core/export/encoders
    ${CMAKE_CURRENT_SOURCE_DIR}/core/export/renderers
    ${CMAKE_CURRENT_SOURCE_DIR}/core/effects/processors
    ${CMAKE_CURRENT_SOURCE_DIR}/core/editor
    ${CMAKE_CURRENT_SOURCE_DIR}/core/editor/display
    ${CMAKE_CURRENT_SOURCE_DIR}/core/editor/gegl
    ${CMAKE_CURRENT_SOURCE_DIR}/core/editor/io
    ${CMAKE_CURRENT_SOURCE_DIR}/core/editor/models
    ${CMAKE_CURRENT_SOURCE_DIR}/core/editor/paint
    ${CMAKE_CURRENT_SOURCE_DIR}/core/editor/plugins
    ${CMAKE_CURRENT_SOURCE_DIR}/core/editor/tools
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ffmpeg/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/onnxruntime/include
    ${CMAKE_CURRENT_SOURCE_DIR}/core/gimp_editor_backend
    ${CMAKE_CURRENT_SOURCE_DIR}/core/gimp_editor_backend/cpp_core
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gimp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gimp/app
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gimp/libgimp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gimp/libgimpbase
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gimp/libgimpmath
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gimp/libgimpcolor
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gimp/libgimpconfig
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gimp/libgimpmodule
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gimp/libgimpthumb
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gimp/libgimpwidgets
)



file(GLOB FFMPEG_RUNTIME_DLLS
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/ffmpeg/bin/*.dll"
)
file(GLOB FFMPEG_RUNTIME_TOOLS
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/ffmpeg/bin/*.exe"
)

file(GLOB ONNX_RUNTIME_DLLS
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/onnxruntime/lib/*.dll"
)

if(FFMPEG_RUNTIME_DLLS)
    add_custom_command(TARGET VideoStudio POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FFMPEG_RUNTIME_DLLS}
            $<TARGET_FILE_DIR:VideoStudio>
        COMMENT "Copying FFmpeg runtime DLLs"
    )
endif()

if(ONNX_RUNTIME_DLLS)
    add_custom_command(TARGET VideoStudio POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${ONNX_RUNTIME_DLLS}
            $<TARGET_FILE_DIR:VideoStudio>
        COMMENT "Copying ONNX Runtime DLLs"
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

if(EXISTS "${Qt6_DIR}/../../../qml/QtQuick/Dialogs")
    add_custom_command(TARGET VideoStudio POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${Qt6_DIR}/../../../qml/QtQuick/Dialogs"
            "$<TARGET_FILE_DIR:VideoStudio>/QtQuick/Dialogs"
        COMMENT "Copying QtQuick.Dialogs QML plugin"
    )
endif()

if(EXISTS "${Qt6_DIR}/../../../plugins/multimedia")
    add_custom_command(TARGET VideoStudio POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:VideoStudio>/multimedia"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${Qt6_DIR}/../../../plugins/multimedia"
            "$<TARGET_FILE_DIR:VideoStudio>/multimedia"
        COMMENT "Copying Qt Multimedia plugins"
    )
endif()

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
    core/effects/processors/GifFrameDecoder.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/core/effects/processors
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

add_executable(ProgramMonitorProbe EXCLUDE_FROM_ALL
    tests/ProgramMonitorProbe.cpp
    core/playback/engines/AudioEngine.cpp
    core/playback/engines/PlaybackEngine.cpp
    core/effects/processors/EffectProcessor.cpp
    core/effects/processors/GifFrameDecoder.cpp
)

target_link_libraries(ProgramMonitorProbe PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Multimedia
    avcodec
    avformat
    avutil
    swresample
    swscale
)

target_link_directories(ProgramMonitorProbe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ffmpeg/lib
)

target_include_directories(ProgramMonitorProbe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core/playback
    ${CMAKE_CURRENT_SOURCE_DIR}/core/effects/processors
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ffmpeg/include
)

if(FFMPEG_RUNTIME_DLLS)
    add_custom_command(TARGET ProgramMonitorProbe POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FFMPEG_RUNTIME_DLLS}
            $<TARGET_FILE_DIR:ProgramMonitorProbe>
        COMMENT "Copying FFmpeg runtime DLLs for ProgramMonitorProbe"
    )
endif()

add_custom_command(TARGET ProgramMonitorProbe POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Qt6::Multimedia>
        $<TARGET_FILE_DIR:ProgramMonitorProbe>
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:ProgramMonitorProbe>/multimedia"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${Qt6_DIR}/../../../plugins/multimedia"
        "$<TARGET_FILE_DIR:ProgramMonitorProbe>/multimedia"
    COMMENT "Copying Qt Multimedia runtime for ProgramMonitorProbe"
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
add_custom_command(TARGET VideoStudio POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/core/vocal/uvr_runner.py"
        "$<TARGET_FILE_DIR:VideoStudio>/uvr_runner.py"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/core/vocal/edge_tts_runner.py"
        "$<TARGET_FILE_DIR:VideoStudio>/edge_tts_runner.py"
    COMMENT "Copying AI runners"
)
