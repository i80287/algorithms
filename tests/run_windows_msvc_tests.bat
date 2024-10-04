if not exist ".\cmake-build-tests-windows-msvc" mkdir cmake-build-tests-windows-msvc

copy ..\number_theory\u64-primes.txt .\cmake-build-tests-windows-msvc\u64-primes.txt
copy ..\number_theory\u128-primes.txt .\cmake-build-tests-windows-msvc\u128-primes.txt
copy ..\tf_idf_actrie\Anglo_Saxon_Chronicle.txt .\cmake-build-tests-windows-msvc\Anglo_Saxon_Chronicle.txt

cd .\cmake-build-tests-windows-msvc

cmake -G "Visual Studio 17 2022" -A x64 -S .. -B .
