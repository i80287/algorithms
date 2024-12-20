set build_dir=cmake-build-tests-windows-msvc

if not exist ".\%build_dir%" mkdir %build_dir%

copy "..\number_theory\u64-primes.txt" ".\%build_dir%\u64-primes.txt"
copy "..\number_theory\u128-primes.txt" ".\%build_dir%\u128-primes.txt"
copy "..\tf_idf_actrie\Anglo_Saxon_Chronicle.txt" ".\%build_dir%\Anglo_Saxon_Chronicle.txt"
if %errorlevel% neq 0 exit /b %errorlevel%

cmake -G "Visual Studio 17 2022" -A x64 -S . -B "%build_dir%"
if %errorlevel% neq 0 exit /b %errorlevel%

@REM msbuild tests.sln -property:Configuration=RelWithDebInfo -noWarn:D9025 -warnaserror -maxcpucount:%NUMBER_OF_PROCESSORS%
cmake --build "%build_dir%" --config RelWithDebInfo --parallel
if %errorlevel% neq 0 exit /b %errorlevel%

cd ".\%build_dir%"
if %errorlevel% neq 0 exit /b %errorlevel%

msbuild RUN_TESTS.vcxproj -property:Configuration=RelWithDebInfo
if %errorlevel% neq 0 exit /b %errorlevel%
