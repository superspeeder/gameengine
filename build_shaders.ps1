Get-ChildItem "assets\shaders\" -Exclude *.spv |
ForEach-Object {
    $inname = $_.FullName
    $outname = $_.FullName + ".spv"
    $output = glslc.exe $inname -o $outname 2>&1
    if (-not $?) {
        Write-Host "Failed to compile shader $inname" -ForegroundColor Red
        Write-Host "$output" -ForegroundColor Red
        exit 1
    }
    Write-Host "Compiled shader $inname -> $outname" -ForegroundColor Green
}