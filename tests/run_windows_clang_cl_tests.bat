if not exist ".\cmake-build-tests-windows-clang-cl" mkdir cmake-build-tests-windows-clang-cl

copy ..\number_theory\u64-primes.txt .\cmake-build-tests-windows-clang-cl\u64-primes.txt
copy ..\number_theory\u128-primes.txt .\cmake-build-tests-windows-clang-cl\u128-primes.txt
copy ..\tf_idf_actrie\Anglo_Saxon_Chronicle.txt .\cmake-build-tests-windows-clang-cl\Anglo_Saxon_Chronicle.txt

cd .\cmake-build-tests-windows-clang-cl

cmake -G "Visual Studio 17 2022" -A x64 -T ClangCL -S .. -B .
