param (
    [string]$ModuleSource = ".\Lunar.psm1",
    [string]$DefaultDllSource = ".\build\liblcca.dll"
)

# 1. Prompt for the DLL path with a default fallback
$promptedDll = Read-Host "Enter the path to liblcca.dll [Default: $DefaultDllSource]"
if ([string]::IsNullOrWhiteSpace($promptedDll)) {
    $promptedDll = $DefaultDllSource
}

# 2. Safety check: Ensure both files actually exist before proceeding
if (-not (Test-Path $ModuleSource)) {
    Write-Error "Cannot find '$ModuleSource'. Please run this script from the directory containing your module."
    return
}

if (-not (Test-Path $promptedDll)) {
    Write-Error "Cannot find liblcca.dll at '$promptedDll'. Please verify the path and run the script again."
    return
}

# 3. Determine the 'Safe Zone' Current User module path
$pathSeparator = [System.IO.Path]::PathSeparator
$userModulePath = ($env:PSModulePath -split $pathSeparator)[0]
$targetDirectory = Join-Path -Path $userModulePath -ChildPath "Lunar"

# 4. Create the target directory if it doesn't exist
if (-not (Test-Path $targetDirectory)) {
    Write-Host "Creating module folder at: $targetDirectory"
    New-Item -Path $targetDirectory -ItemType Directory -Force | Out-Null
}

# 5. Copy the files into the Safe Zone
Write-Host "Copying files..."
Copy-Item -Path $ModuleSource -Destination (Join-Path $targetDirectory "Lunar.psm1") -Force
Copy-Item -Path $promptedDll -Destination (Join-Path $targetDirectory "liblcca.dll") -Force

# 6. Notify the user
Write-Host ""
Write-Host "Installation Complete! ✅" -ForegroundColor Green
Write-Host "Files safely installed to: $targetDirectory"
Write-Host "Please close this window and start a new PowerShell session so the module can be discovered." -ForegroundColor Yellow
