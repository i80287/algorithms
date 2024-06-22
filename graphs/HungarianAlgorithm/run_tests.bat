if not exist ".\build_tests" mkdir build_tests
cd .\build_tests
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo -S .. -B .
make all --jobs 4
make test
