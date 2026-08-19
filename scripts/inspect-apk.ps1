[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$ApkPath,

    [string]$OutputDir = '.\analysis-output'
)

$ErrorActionPreference = 'Stop'

$apk = (Resolve-Path -LiteralPath $ApkPath).Path
$output = [System.IO.Path]::GetFullPath((Join-Path (Get-Location) $OutputDir))
New-Item -ItemType Directory -Force -Path $output | Out-Null

Add-Type -AssemblyName System.IO.Compression.FileSystem

function Read-ZipEntryText {
    param([System.IO.Compression.ZipArchiveEntry]$Entry)

    $stream = $Entry.Open()
    try {
        $reader = [System.IO.StreamReader]::new($stream)
        try { return $reader.ReadToEnd() }
        finally { $reader.Dispose() }
    }
    finally { $stream.Dispose() }
}

$hash = (Get-FileHash -LiteralPath $apk -Algorithm SHA256).Hash
$size = (Get-Item -LiteralPath $apk).Length
$archive = [System.IO.Compression.ZipFile]::OpenRead($apk)

try {
    $entries = @($archive.Entries | ForEach-Object FullName | Sort-Object)
    $airEntry = $archive.Entries | Where-Object FullName -eq 'assets/META-INF/AIR/application.xml' | Select-Object -First 1
    $airText = if ($airEntry) { Read-ZipEntryText $airEntry } else { '' }
    $usesSdk = [regex]::Match($airText, '<uses-sdk[^>]+>')
    $libs = @($entries | Where-Object { $_ -match '^lib/.+\.so$' })
    $anes = @($entries | Where-Object { $_ -match '/META-INF/ANE/extension\.xml$' })

    $lines = [System.Collections.Generic.List[string]]::new()
    [void]$lines.Add('# APK inspection report')
    [void]$lines.Add('')
    [void]$lines.Add("- File: ``$apk``")
    [void]$lines.Add("- Size: $size bytes")
    [void]$lines.Add("- SHA-256: ``$hash``")
    [void]$lines.Add("- ZIP entries: $($entries.Count)")
    [void]$lines.Add('')
    [void]$lines.Add('## AIR descriptor summary')
    [void]$lines.Add('')

    foreach ($pattern in @('<application xmlns="[^"]+"', '<id>[^<]+', '<versionNumber>[^<]+', '<content>[^<]+', '<minSdkVersion>[^<]+', '<targetSdkVersion>[^<]+')) {
        $match = [regex]::Match($airText, $pattern)
        if ($match.Success) { [void]$lines.Add("- ``$($match.Value)``") }
    }
    if ($usesSdk.Success) { [void]$lines.Add("- ``$($usesSdk.Value)``") }

    [void]$lines.Add('')
    [void]$lines.Add('## Native libraries')
    [void]$lines.Add('')
    if ($libs.Count -eq 0) { [void]$lines.Add('- None found') }
    else { $libs | ForEach-Object { [void]$lines.Add("- ``$_``") } }

    [void]$lines.Add('')
    [void]$lines.Add('## ANE descriptors')
    [void]$lines.Add('')
    if ($anes.Count -eq 0) { [void]$lines.Add('- None found') }
    else { $anes | ForEach-Object { [void]$lines.Add("- ``$_``") } }

    [void]$lines.Add('')
    [void]$lines.Add('## Selected archive entries')
    [void]$lines.Add('')
    $entries | Where-Object { $_ -match '(^AndroidManifest\.xml$|TTPixel\.swf$|application\.xml$|\.so$|extension\.xml$|catalog\.xml$)' } | ForEach-Object { [void]$lines.Add("- ``$_``") }

    $report = Join-Path $output 'apk-inspection-report.md'
    Set-Content -LiteralPath $report -Value $lines -Encoding UTF8
    Write-Output "Report: $report"
}
finally {
    $archive.Dispose()
}
