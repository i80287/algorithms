set build_dir=cmake-build-tests-windows-msys2-mingw-w64

if not exist ".\%build_dir%" mkdir %build_dir%

copy "..\number_theory\u64-primes.txt" ".\%build_dir%\u64-primes.txt"
copy "..\number_theory\u128-primes.txt" ".\%build_dir%\u128-primes.txt"
copy "..\tf_idf_actrie\Anglo_Saxon_Chronicle.txt" ".\%build_dir%\Anglo_Saxon_Chronicle.txt"

cd ".\%build_dir%"

@REM "MSYS Makefiles" - may use clang-cl instead of gcc / clang
@REM "MinGW Makefiles" - may use clang-cl instead of gcc / clang
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo -S .. -B .
if %errorlevel% neq 0 exit /b %errorlevel%

make all --jobs %NUMBER_OF_PROCESSORS%
if %errorlevel% neq 0 exit /b %errorlevel%

SET CTEST_OUTPUT_ON_FAILURE=1
make test
if %errorlevel% neq 0 exit /b %errorlevel%
