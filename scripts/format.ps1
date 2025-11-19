Get-ChildItem -Recurse -Path glRemixRenderer, glRemixShim, shared `
    -Include *.h, *.cpp | 
    ForEach-Object { clang-format -i $_.FullName }

