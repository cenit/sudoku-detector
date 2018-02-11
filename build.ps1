#!/usr/bin/env pwsh

#Remove-Item build -Force -Recurse -ErrorAction SilentlyContinue
New-Item -Path .\build -ItemType directory -Force
Set-Location build

#cmake -G "Visual Studio 15 2017 Win64" "-DCMAKE_TOOLCHAIN_FILE=C:\Users\stefano\Codice\vcpkg_cenit\scripts\buildsystems\vcpkg.cmake" "-DVCPKG_TARGET_TRIPLET=$env:VCPKG_DEFAULT_TRIPLET" ..
cmake -G "Visual Studio 15 2017" "-DCMAKE_TOOLCHAIN_FILE=C:\Users\stefano\Codice\vcpkg_cenit\scripts\buildsystems\vcpkg.cmake" "-DVCPKG_TARGET_TRIPLET=x86-windows" ..

cmake --build . --config Release --target install

Set-Location ..
