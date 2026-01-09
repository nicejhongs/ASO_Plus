cmake_minimum_required(VERSION 3.15)
project(ASO_Plus)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SDL2 찾기
find_package(SDL2 REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)

# 소스 파일
set(SOURCES
    src/main.cpp
    src/Game.cpp
    src/Player.cpp
    src/Enemy.cpp
    src/Bullet.cpp
)

# 실행 파일 생성
add_executable(${PROJECT_NAME} ${SOURCES})

# 인클루드 디렉토리
target_include_directories(${PROJECT_NAME} PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/include
)

# 라이브러리 링크
target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${SDL2_LIBRARIES} 
    $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>
    $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
)

# Windows 전용 설정
if(WIN32)
    # SDL2 DLL을 실행 파일 옆에 복사
    if(SDL2_FOUND)
        # SDL2::SDL2가 타겟으로 정의된 경우
        if(TARGET SDL2::SDL2)
            get_target_property(SDL2_DLL_PATH SDL2::SDL2 IMPORTED_LOCATION)
            if(SDL2_DLL_PATH)
                add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${SDL2_DLL_PATH}
                    $<TARGET_FILE_DIR:${PROJECT_NAME}>
                )
            endif()
        endif()
    endif()
endif()

# Android 빌드 설정 (향후 추가)
# iOS 빌드 설정 (향후 추가)
