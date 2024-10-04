if not exist ".\cmake-build-tests-windows-msys2-mingw-w64" mkdir cmake-build-tests-windows-msys2-mingw-w64

copy ..\number_theory\u64-primes.txt .\cmake-build-tests-windows-msys2-mingw-w64\u64-primes.txt
copy ..\number_theory\u128-primes.txt .\cmake-build-tests-windows-msys2-mingw-w64\u128-primes.txt
copy ..\tf_idf_actrie\Anglo_Saxon_Chronicle.txt .\cmake-build-tests-windows-msys2-mingw-w64\Anglo_Saxon_Chronicle.txt

cd .\cmake-build-tests-windows-msys2-mingw-w64

cmake -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo -S .. -B .
make all --jobs %NUMBER_OF_PROCESSORS%
SET CTEST_OUTPUT_ON_FAILURE=1
make test
