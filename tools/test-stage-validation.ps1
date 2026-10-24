# Exercise malformed TMX cases without modifying the project's maps.
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$fixtureRoot = Join-Path $projectRoot 'tmp/stage-validation-fixture'
New-Item -ItemType Directory -Force (Join-Path $fixtureRoot 'src'), (Join-Path $fixtureRoot 'res') | Out-Null
Copy-Item (Join-Path $projectRoot 'src/stage.c') (Join-Path $fixtureRoot 'src/stage.c')
Copy-Item (Join-Path $projectRoot 'res/resources.res') (Join-Path $fixtureRoot 'res/resources.res')
Copy-Item (Join-Path $projectRoot 'res/level_test2.tmx') (Join-Path $fixtureRoot 'res/level_test2.tmx')
$target = Join-Path $fixtureRoot 'res/level_transition_test.tmx'
$original = Get-Content (Join-Path $projectRoot 'res/level_transition_test.tmx') -Raw
$cases = @(
    @{ Name = 'missing destination'; Pattern = 'name="nextStage" type="int" value="1"'; Replacement = 'name="nextStage" type="int" value="99"'; Error = 'invalid destination' },
    @{ Name = 'self destination'; Pattern = 'name="nextStage" type="int" value="1"'; Replacement = 'name="nextStage" type="int" value="2"'; Error = 'invalid destination' },
    @{ Name = 'zero width'; Pattern = 'width="24" height="32"'; Replacement = 'width="0" height="32"'; Error = 'outside gameplay bounds' },
    @{ Name = 'missing exportSize'; Pattern = 'name="exportSize"'; Replacement = 'name="unused"'; Error = 'exportSize' },
    @{ Name = 'rotated rectangle'; Pattern = 'width="24" height="32"'; Replacement = 'width="24" height="32" rotation="10"'; Error = 'unrotated rectangle' },
    @{ Name = 'missing spawn'; Pattern = 'name="Spawns"'; Replacement = 'name="Unused"'; Error = 'exactly one PlayerSpawn' },
    @{ Name = 'incorrect registry width'; Pattern = 'width="87" height="25" tilewidth'; Replacement = 'width="88" height="25" tilewidth'; Error = 'registry width' }
)
foreach ($case in $cases) {
    Set-Content $target ($original.Replace($case.Pattern, $case.Replacement))
    $rejected = $false
    try { & (Join-Path $PSScriptRoot 'validate-stages.ps1') -ProjectRoot $fixtureRoot | Out-Null }
    catch { if ($_.Exception.Message -notlike "*$($case.Error)*") { throw }; $rejected = $true }
    if (!$rejected) { throw "Validation accepted $($case.Name)" }
    Write-Host "Rejected $($case.Name): OK"
}
& (Join-Path $PSScriptRoot 'validate-stages.ps1') -ProjectRoot $projectRoot
