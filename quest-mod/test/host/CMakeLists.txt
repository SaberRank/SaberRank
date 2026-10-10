cmake_minimum_required(VERSION 3.21)

project(scoresaber_replay_host_tests LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SCORESABER_HOST_ASAN "Build host replay tests with AddressSanitizer" OFF)

get_filename_component(REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)

file(GLOB PAVLOV_SOURCES CONFIGURE_DEPENDS
    "${REPO_ROOT}/src/Utils/lzma/pavlov/*.c"
)

add_executable(replay_roundtrip_test
    replay_roundtrip_test.cpp
    "${REPO_ROOT}/src/Features/Replays/Format/HsvReplayConfigCodec.cpp"
    "${REPO_ROOT}/src/Features/Replays/Format/ReplayExtensionPayloads.cpp"
    "${REPO_ROOT}/src/Features/Replays/Format/ReplayFile.cpp"
    "${REPO_ROOT}/src/Features/Replays/Format/ReplayReader.cpp"
    "${REPO_ROOT}/src/Features/Replays/Format/ReplayScoringTypes.cpp"
    "${REPO_ROOT}/src/Features/Replays/Format/ReplayWriter.cpp"
    "${REPO_ROOT}/src/Utils/lzma/lzma.cpp"
    ${PAVLOV_SOURCES}
)

target_include_directories(replay_roundtrip_test PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/stubs"
    "${REPO_ROOT}/include"
    # the hsv codec pulls vendored rapidjson through the beatsaber-hook include path
    "${REPO_ROOT}/extern/includes"
)

file(GLOB LIVE_PROTO_SOURCES CONFIGURE_DEPENDS
    "${REPO_ROOT}/src/Features/Live/Protocol/Generated/*.cpp"
)

add_executable(live_proto_roundtrip_test
    live_proto_roundtrip_test.cpp
    "${REPO_ROOT}/src/Features/Live/Protocol/LudusProto.cpp"
    "${REPO_ROOT}/src/Features/Live/Protocol/ProtoWire.cpp"
    ${LIVE_PROTO_SOURCES}
)

target_include_directories(live_proto_roundtrip_test PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/stubs"
    "${REPO_ROOT}/include"
)

add_executable(websocket_framing_test
    websocket_framing_test.cpp
    "${REPO_ROOT}/src/Features/Live/Transport/WebSocketFraming.cpp"
)

target_include_directories(websocket_framing_test PRIVATE
    "${REPO_ROOT}/include"
)

add_executable(ludus_chat_buffer_test
    ludus_chat_buffer_test.cpp
    "${REPO_ROOT}/src/Features/Live/Ludus/Domain/LiveChatEntry.cpp"
    "${REPO_ROOT}/src/Features/Live/Ludus/Packets/LudusChatMessageBuffer.cpp"
)

target_include_directories(ludus_chat_buffer_test PRIVATE
    "${REPO_ROOT}/include"
)

if(SCORESABER_HOST_ASAN)
    foreach(host_test replay_roundtrip_test live_proto_roundtrip_test websocket_framing_test ludus_chat_buffer_test)
        target_compile_options(${host_test} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(${host_test} PRIVATE -fsanitize=address)
    endforeach()
endif()
