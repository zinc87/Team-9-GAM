$path = "c:\Users\CheEe.Fang\Desktop\csd3401f25_team_alcosoft\Algin\Algin\Assets\Model\MiniTissuePacket.agstaticmesh"
$bytes = [System.IO.File]::ReadAllBytes($path)
$str = [System.Text.Encoding]::ASCII.GetString($bytes)
$matches = [regex]::Matches($str, '[a-zA-Z0-9_\-\. ]{4,}')
$count = 0
foreach ($m in $matches) {
    if (-not $m.Value.Contains("0000")) {
        Write-Host "FOUND: " $m.Value
        $count++
        if ($count -ge 30) { break }
    }
}
Write-Host "Done"
