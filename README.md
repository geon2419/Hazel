# Hazel - A Modern 3D Engine

## Development

### Common Setup

1. 저장소 클론 후 Root 경로로 이동합니다.
2. 서브모듈을 초기화합니다: `git submodule update --init --recursive`
3. pre-commit 훅 경로를 설정합니다: `git config core.hooksPath .githooks`

### Windows Setup/Build (x64)

1. Visual Studio 2026(Desktop development with C++) 및 MSBuild, LLVM를 설치합니다.
2. `premake5`를 준비합니다.
`GenerateProjects.bat`는 `vendor\bin\premake\premake5.exe` 경로를 사용합니다.
3. Visual Studio 솔루션을 생성합니다: `.\GenerateProjects.bat` (또는 `premake5 vs2026`)
4. 테스트를 빌드합니다:
`msbuild .\Hazel.sln /m /t:Hazel-Test /p:Configuration=Debug /p:Platform=x64`
5. 샌드박스를 빌드합니다:
`msbuild .\Hazel.sln /m /t:Sandbox /p:Configuration=Debug /p:Platform=x64`
6. 실행 파일:
`.\bin\Debug-windows-x64\Hazel-Test\Hazel-Test.exe`
`.\bin\Debug-windows-x64\Sandbox\Sandbox.exe`

### macOS Setup/Build (arm64, dev scope)

1. Xcode 및 Command Line Tools를 설치합니다.
2. `premake5`를 설치합니다.
3. Xcode 프로젝트를 생성합니다: `./GenerateProjects.sh` (또는 `premake5 xcode4`)
4. 테스트를 빌드합니다:
`xcodebuild -workspace Hazel.xcworkspace -scheme Hazel-Test -configuration Debug -derivedDataPath build/xcode build`
5. 샌드박스를 빌드합니다:
`xcodebuild -workspace Hazel.xcworkspace -scheme Sandbox -configuration Debug -derivedDataPath build/xcode build`
6. 실행 파일:
`./bin/Debug-macosx-AARCH64/Hazel-Test/Hazel-Test`
`./bin/Debug-macosx-AARCH64/Sandbox/Sandbox`

### Lint

- Lint(Auto format): `./tools/lint-local.sh`
- Lint(Check only): `./tools/lint-local.sh --check`
- Pre-Commit(Check staged changes only): `./tools/lint-precommit.sh`
- Windows CI lint(Warning gate): `pwsh ./tools/lint-ci-windows.ps1 -Mode all -Enforcement advisory`

## Reference

The Cherno [Game Engine series](https://www.youtube.com/playlist?list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT)

## License

이 프로젝트는 학습 목적으로 만들어졌으며, 상업적 용도로 사용되지 않습니다.
