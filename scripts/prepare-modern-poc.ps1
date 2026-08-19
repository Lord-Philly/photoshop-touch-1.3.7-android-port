[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$DecodedProject,

    [Parameter(Mandatory = $true)]
    [string]$OutputProject,

    [int]$TargetSdk = 35,
    [int]$VersionCode = 1003008,
    [string]$VersionName = '1.3.7-modern-poc'
)

$ErrorActionPreference = 'Stop'

$source = (Resolve-Path -LiteralPath $DecodedProject).Path
$destination = [System.IO.Path]::GetFullPath((Join-Path (Get-Location) $OutputProject))
if (Test-Path -LiteralPath $destination) {
    throw "OutputProject already exists: $destination. Use a new directory to keep runs reproducible."
}

Copy-Item -LiteralPath $source -Destination $destination -Recurse

$androidNamespace = 'http://schemas.android.com/apk/res/android'
$manifestPath = Join-Path $destination 'AndroidManifest.xml'
$manifestXml = [System.Xml.XmlDocument]::new()
$manifestXml.PreserveWhitespace = $true
$manifestXml.Load($manifestPath)
$ns = [System.Xml.XmlNamespaceManager]::new($manifestXml.NameTable)
$ns.AddNamespace('android', $androidNamespace)

$manifest = $manifestXml.SelectSingleNode('/manifest')
$usesSdk = $manifestXml.SelectSingleNode('/manifest/uses-sdk')
$application = $manifestXml.SelectSingleNode('/manifest/application')
$entryActivity = $manifestXml.SelectSingleNode('/manifest/application/activity[contains(@android:name, "AppEntry")]', $ns)
$legacyReceiver = $manifestXml.SelectSingleNode('/manifest/application/receiver[contains(@android:name, "C2DMReceiver")]', $ns)

if (-not $manifest -or -not $application -or -not $entryActivity) {
    throw 'Expected AIR manifest nodes were not found.'
}

$null = $manifest.SetAttribute('versionCode', $androidNamespace, $VersionCode.ToString())
$null = $manifest.SetAttribute('versionName', $androidNamespace, $VersionName)
if ($usesSdk) {
    $null = $usesSdk.SetAttribute('targetSdkVersion', $androidNamespace, $TargetSdk.ToString())
}
$null = $entryActivity.SetAttribute('exported', $androidNamespace, 'true')

# The legacy receiver has an intent filter and must declare exported explicitly
# for target SDK 31+. It retains the original permission gate for this PoC.
if ($legacyReceiver) {
    $null = $legacyReceiver.SetAttribute('exported', $androidNamespace, 'true')
}

# This flag is useful on older Android versions; Android 11+ may ignore it for
# target SDK 30+, so storage behavior still requires a real device test.
$null = $application.SetAttribute('requestLegacyExternalStorage', $androidNamespace, 'true')
$manifestXml.Save($manifestPath)

$apktoolYmlPath = Join-Path $destination 'apktool.yml'
if (Test-Path -LiteralPath $apktoolYmlPath) {
    $apktoolYml = [System.IO.File]::ReadAllText($apktoolYmlPath)
    $apktoolYml = $apktoolYml -replace '(?m)^  targetSdkVersion:\s*\d+', "  targetSdkVersion: $TargetSdk"
    $apktoolYml = $apktoolYml -replace '(?m)^  versionCode:\s*\d+', "  versionCode: $VersionCode"
    $apktoolYml = $apktoolYml -replace '(?m)^  versionName:\s*.*$', "  versionName: $VersionName"
    if ($apktoolYml -notmatch '(?m)^- resources\.arsc$') {
        $apktoolYml = $apktoolYml -replace '(?m)^doNotCompress:\r?\n', "doNotCompress:`r`n- resources.arsc`r`n"
    }
    [System.IO.File]::WriteAllText($apktoolYmlPath, $apktoolYml, [System.Text.UTF8Encoding]::new($false))
}

$airPath = Join-Path $destination 'assets\META-INF\AIR\application.xml'
if (Test-Path -LiteralPath $airPath) {
    $airText = [System.IO.File]::ReadAllText($airPath)
    $airText = $airText -replace '<versionNumber>1\.3\.7</versionNumber>', '<versionNumber>1.3.8</versionNumber>'
    $airText = $airText -replace '<versionLabel>1\.3\.7</versionLabel>', "<versionLabel>$VersionName</versionLabel>"
    $targetSdkAttribute = 'android:targetSdkVersion="' + $TargetSdk + '"'
    $airText = $airText -replace 'android:targetSdkVersion="12"', $targetSdkAttribute
    [System.IO.File]::WriteAllText($airPath, $airText, [System.Text.UTF8Encoding]::new($false))
}

Write-Output "Prepared modern manifest PoC at: $destination"
Write-Output "targetSdkVersion: $TargetSdk"
Write-Output "versionCode: $VersionCode"
Write-Output "versionName: $VersionName"
