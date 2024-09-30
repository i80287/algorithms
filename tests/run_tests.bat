if not exist ".\cmake-build-tests-windows" mkdir cmake-build-tests-windows

copy ..\number_theory\u64-primes.txt .\cmake-build-tests-windows\u64-primes.txt
copy ..\number_theory\u128-primes.txt .\cmake-build-tests-windows\u128-primes.txt
copy ..\tf_idf_actrie\Anglo_Saxon_Chronicle.txt .\cmake-build-tests-windows\Anglo_Saxon_Chronicle.txt

cd .\cmake-build-tests-windows

cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo -S .. -B .
make all --jobs 4
make test
