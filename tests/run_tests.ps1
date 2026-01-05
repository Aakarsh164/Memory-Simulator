$root = Split-Path -Parent $MyInvocation.MyCommand.Definition
$bin = Join-Path $root "..\bin\memsim_test.exe"
$inputs = Get-ChildItem -Path (Join-Path $root "inputs") -Filter *.txt
foreach ($f in $inputs) {
    $inpath = $f.FullName
    $outpath = Join-Path $root "output\$($f.BaseName)_output.txt"
    Get-Content $inpath | & $bin | Out-File $outpath -Encoding ascii
    Write-Host "Ran:" $f.Name "->" $outpath
}
# simple compare to expected if exists
$expectedDir = Join-Path $root "expected"
Get-ChildItem (Join-Path $root "output") -Filter *_output.txt | ForEach-Object {
    $name = $_.Name -replace "_output.txt",""
    $exp = Join-Path $expectedDir "$name`_expected.txt"
    if (Test-Path $exp) {
        $a = Get-Content $_.FullName
        $b = Get-Content $exp
        if ($a -eq $b) { Write-Host "$name: OK" } else { Write-Host "$name: MISMATCH" }
    }
}
