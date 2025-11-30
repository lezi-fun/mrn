#!/bin/bash
echo "=== Build Debug Information ==="
echo "Current directory: $(pwd)"
echo "Build directory contents:"
ls -la build/ || echo "No build directory"
echo "Build/bin contents:"
ls -la build/bin/ || echo "No build/bin directory"
echo "All executable files in build:"
find build -type f -perm +111 -name "*" 2>/dev/null || find build -type f -executable -name "*" 2>/dev/null
echo "CMake cache:"
cat build/CMakeCache.txt | grep "CMAKE\|BINARY\|OUTPUT" || echo "No CMakeCache.txt"
