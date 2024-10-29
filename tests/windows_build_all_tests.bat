set build_dir=cmake-build-tests-windows-clang-cl

if not exist ".\%build_dir%" mkdir %build_dir%

cd ".\%build_dir%"

if %errorlevel% neq 0 exit /b %errorlevel%

cmake -G "Visual Studio 17 2022" -A x64 -T ClangCL -S .. -B .
if %errorlevel% neq 0 exit /b %errorlevel%

msbuild tests.sln -property:Configuration=RelWithDebInfo -noWarn:D9025 -warnaserror -maxcpucount:%NUMBER_OF_PROCESSORS%
if %errorlevel% neq 0 exit /b %errorlevel%

cd ..

set build_dir=cmake-build-tests-windows-msvc

if not exist ".\%build_dir%" mkdir %build_dir%

cd ".\%build_dir%"

if %errorlevel% neq 0 exit /b %errorlevel%

cmake -G "Visual Studio 17 2022" -A x64 -S .. -B .
if %errorlevel% neq 0 exit /b %errorlevel%

msbuild tests.sln -property:Configuration=RelWithDebInfo -noWarn:D9025 -warnaserror -maxcpucount:%NUMBER_OF_PROCESSORS%
if %errorlevel% neq 0 exit /b %errorlevel%
