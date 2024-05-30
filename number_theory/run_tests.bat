if not exist ".\build_tests" mkdir build_tests
copy .\u64-primes.txt .\build_tests\u64-primes.txt
copy .\u128-primes.txt .\build_tests\u128-primes.txt
cd .\build_tests
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo -S .. -B .
make all --jobs 4
make test
