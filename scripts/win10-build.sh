#!/bin/sh

# parse commandline options
for arg in $*; do
    case ${arg} in
	-v) opt_args="${opt_args} -clp:ShowCommandLine";;
	*) echo unknown option ${arg} ;;
    esac
done

# make cmake build batch
cat > .cmake.cmd <<EOF
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
cd %HOME%\src\c11atomic
if not exist build mkdir build
cd build
cmake -G "Visual Studio 16 2019" -A x64 ..
cmake --build . --config RelWithDebInfo -j -- ${opt_args}
RelWithDebInfo\test_threads.exe
RelWithDebInfo\test_atomics.exe
EOF

# sync source and execute batch
rsync -av --delete --exclude build ../c11atomic/ win10-vm:src/c11atomic/
ssh win10-vm src\\c11atomic\\.cmake.cmd
