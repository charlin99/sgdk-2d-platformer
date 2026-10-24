# Validate the stage registry against the authored TMX geometry before building.
param([string]$ProjectRoot = (Split-Path $PSScriptRoot -Parent))
$ErrorActionPreference = 'Stop'
$projectRoot = $ProjectRoot
$registry = Get-Content (Join-Path $projectRoot 'src/stage.c') -Raw
$resources = Get-Content (Join-Path $projectRoot 'res/resources.res') -Raw
$stages = @{}
foreach ($entry in [regex]::Matches($registry, '\{(\d+),\s*(\d+),\s*&(\w+),')) {
    $id = [int]$entry.Groups[1].Value
    if ($stages.ContainsKey($id)) { throw "Duplicate stage ID $id" }
    $symbol = $entry.Groups[3].Value
    $mapResource = [regex]::Match($resources, '(?m)^MAP\s+' + $symbol + '\s+"([^"]+)"')
    if (!$mapResource.Success) { throw "Stage $id has no MAP resource" }
    [xml]$map = Get-Content (Join-Path $projectRoot ('res/' + $mapResource.Groups[1].Value)) -Raw
    $width = [int]$map.map.width * [int]$map.map.tilewidth
    if ($width -ne [int]$entry.Groups[2].Value) { throw "Stage $id registry width differs from TMX" }
    $spawns = @($map.map.objectgroup | Where-Object name -eq 'Spawns' | ForEach-Object { $_.object })
    if ($spawns.Count -ne 1 -or $spawns[0].name -ne 'PlayerSpawn') { throw "Stage $id needs exactly one PlayerSpawn" }
    $spawn = $spawns[0]
    if ([double]$spawn.x -lt 0 -or [double]$spawn.y -lt 0 -or [double]$spawn.x + 16 -gt $width -or [double]$spawn.y + 16 -gt 200) { throw "Stage $id spawn outside gameplay bounds" }
    if (!($spawn.properties.property | Where-Object { $_.name -eq 'exportPosition' -and $_.value -eq 'true' })) { throw "Stage $id spawn must exportPosition" }
    $cannons = @($map.map.objectgroup | Where-Object name -eq 'Cannons' | ForEach-Object { $_.object })
    if ($cannons.Count -gt 16) { throw "Stage $id exceeds STAGE_MAX_CANNONS" }
    $stages[$id] = @{ Map = $map; Width = $width; Spawn = $spawn }
}
if (!$stages.ContainsKey(1)) { throw 'Missing initial stage 1' }
foreach ($id in $stages.Keys) {
    $stage = $stages[$id]
    $exits = @($stage.Map.map.objectgroup | Where-Object name -eq 'StageExits' | ForEach-Object { $_.object })
    foreach ($exit in $exits) {
        $x = [double]$exit.x; $y = [double]$exit.y; $w = [double]$exit.width; $h = [double]$exit.height
        if ($null -ne $exit.point -or $null -ne $exit.polygon -or $null -ne $exit.polyline -or $null -ne $exit.ellipse -or $exit.gid -or [double]$exit.rotation -ne 0) { throw "Stage $id exit must be an unrotated rectangle" }
        if ($w -le 0 -or $h -le 0 -or $x -lt 0 -or $y -lt 0 -or $x + $w -gt $stage.Width -or $y + $h -gt 200) { throw "Stage $id exit outside gameplay bounds" }
        foreach ($value in @($x, $y, $w, $h)) { if ($value -ne [math]::Floor($value)) { throw "Stage $id exit geometry must use integer pixels" } }
        foreach ($propertyName in @('exportPosition', 'exportSize')) {
            if (!($exit.properties.property | Where-Object { $_.name -eq $propertyName -and $_.type -eq 'bool' -and $_.value -eq 'true' })) { throw "Stage $id exit requires $propertyName = true" }
        }
        $destination = @($exit.properties.property | Where-Object name -eq 'nextStage')
        if ($destination.Count -ne 1 -or $destination[0].type -ne 'int') { throw "Stage $id exit requires integer nextStage" }
        $next = [int]$destination[0].value
        if ($next -eq $id -or !$stages.ContainsKey($next)) { throw "Stage $id exit has invalid destination $next" }
        $spawn = $stage.Spawn
        if ([double]$spawn.x + 16 -gt $x -and [double]$spawn.x -lt $x + $w -and [double]$spawn.y + 16 -gt $y -and [double]$spawn.y -lt $y + $h) { throw "Stage $id spawn overlaps an exit" }
    }
    Write-Host "Stage ${id}: $($stage.Width) px, one spawn, $($exits.Count) exit(s): OK"
}

