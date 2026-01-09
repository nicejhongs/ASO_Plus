# ASO_Plus - 2D 슈팅 게임

크로스 플랫폼 2D 슈팅 게임 (Windows, Android, iOS 지원 예정)

## 필요한 것들

### Windows
- CMake 3.15 이상
- Visual Studio 2019 이상 (또는 MinGW)
- SDL2 라이브러리

### SDL2 설치 (Windows)

#### 방법 1: vcpkg 사용 (권장)
```powershell
# vcpkg 설치 (아직 없다면)
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# SDL2 설치
.\vcpkg install sdl2:x64-windows

# CMake에 vcpkg 통합
.\vcpkg integrate install
```

#### 방법 2: 수동 설치
1. https://www.libsdl.org/download-2.0.php 에서 SDL2 개발 라이브러리 다운로드
2. 압축 해제하고 적절한 위치에 설치
3. 환경 변수 설정 또는 CMake에 경로 지정

## 빌드 방법

### Windows (vcpkg 사용시)
```powershell
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg 경로]/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

### Windows (수동 SDL2 설치시)
```powershell
mkdir build
cd build
cmake .. -DSDL2_DIR="[SDL2 설치 경로]/cmake"
cmake --build . --config Release
```

## 실행 방법

빌드 후 생성된 실행 파일을 실행:
```powershell
.\build\Release\ASO_Plus.exe
```

## 게임 조작법

- **방향키 / WASD**: 플레이어 이동
- **스페이스바**: 총알 발사
- **ESC / 창 닫기**: 게임 종료

## 프로젝트 구조

```
ASO_Plus/
├── src/
│   ├── include/          # 헤더 파일
│   │   ├── Game.h
│   │   ├── Player.h
│   │   ├── Enemy.h
│   │   └── Bullet.h
│   ├── main.cpp          # 메인 엔트리 포인트
│   ├── Game.cpp          # 게임 메인 로직
│   ├── Player.cpp        # 플레이어 클래스
│   ├── Enemy.cpp         # 적 클래스
│   └── Bullet.cpp        # 총알 클래스
├── CMakeLists.txt        # CMake 빌드 설정
└── README.md             # 이 파일
```

## 향후 계획

- [ ] 점수 시스템
- [ ] 사운드 및 음악
- [ ] 파워업 아이템
- [ ] 다양한 적 패턴
- [ ] 보스전
- [ ] Android 빌드 설정
- [ ] iOS 빌드 설정
- [ ] 게임 패드 지원

## 라이선스

MIT License
