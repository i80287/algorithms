set clang_cl_build_dir=cmake-build-tests-windows-clang-cl
set msvc_build_dir=cmake-build-tests-windows-msvc

cmake -G "Visual Studio 17 2022" -A x64 -T ClangCL -S . -B "%clang_cl_build_dir%"
if %errorlevel% neq 0 exit /b %errorlevel%

cmake --build "%clang_cl_build_dir%" --config RelWithDebInfo --parallel
if %errorlevel% neq 0 exit /b %errorlevel%

cmake -G "Visual Studio 17 2022" -A x64 -S . -B "%msvc_build_dir%"
if %errorlevel% neq 0 exit /b %errorlevel%

cmake --build "%msvc_build_dir%" --config RelWithDebInfo --parallel
if %errorlevel% neq 0 exit /b %errorlevel%
