Get-ChildItem -Recurse -Path glRemixRenderer, glRemixShim, shared `
    -Include *.h, *.cpp, *.inl | 
    ForEach-Object { clang-format -i $_.FullName }

