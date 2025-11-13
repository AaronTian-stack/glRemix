Get-ChildItem -Recurse -Path glRemixRenderer, glRemixShim, shared `
    -Include *.h, *.cpp, *.hlsl | 
    ForEach-Object { clang-format -i $_.FullName }

