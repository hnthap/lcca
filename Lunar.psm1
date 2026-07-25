if (-not ("LunarAPI" -as [type])) {
    Add-Type -TypeDefinition @"
    using System;
    using System.Runtime.InteropServices;

    [StructLayout(LayoutKind.Sequential)]
    public struct lcca_gregorian_date {
        public double TimeZone;
        public int Year;
        public sbyte Month;
        public sbyte Day;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct lcca_lunar_date {
        public double TimeZone;
        public int K;
        public int Year;
        public sbyte Month;
        public sbyte MonthSize;
        public sbyte Day;
        public sbyte Leap; 
    }
    public class LunarAPI {
        [DllImport("liblcca.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern lcca_lunar_date lcca_convert_gregorian_to_lunar(lcca_gregorian_date gregorian);

        public static lcca_lunar_date GetLunar(double tz, int year, int month, int day) {
            lcca_gregorian_date gDate = new lcca_gregorian_date();

            gDate.TimeZone = tz;
            gDate.Year = year;
            gDate.Month = (sbyte)month;
            gDate.Day = (sbyte)day;

            return lcca_convert_gregorian_to_lunar(gDate);
        }

        // On .NET Core (PowerShell 7+), this type lives in an in-memory
        // assembly with no on-disk Location, so the default P/Invoke probing
        // resolves liblcca.dll against a null base path and throws
        // "Value cannot be null. (Parameter 'path1')". Registering an explicit
        // resolver that loads the DLL from a caller-supplied directory (the
        // module folder) fixes native-library discovery. On .NET Framework
        // (Windows PowerShell 5.1) NativeLibrary is unavailable, so callers
        // fall back to the current directory / PATH as before.
        public static void RegisterDllResolver(string directory) {
            // Installed layout: liblcca.dll sits next to the module. In-repo
            // layout (running from source, e.g. tests): it lives under build/.
            string[] candidates = {
                System.IO.Path.Combine(directory, "liblcca.dll"),
                System.IO.Path.Combine(directory, "build", "liblcca.dll")
            };
            NativeLibrary.SetDllImportResolver(
                typeof(LunarAPI).Assembly,
                (libraryName, assembly, searchPath) => {
                    if (libraryName == "liblcca.dll") {
                        foreach (string candidate in candidates) {
                            if (System.IO.File.Exists(candidate)) {
                                return NativeLibrary.Load(candidate);
                            }
                        }
                    }
                    return IntPtr.Zero;
                });
        }
    }
"@
}

# Point native P/Invoke at the liblcca.dll that ships alongside this module.
# NativeLibrary only exists on .NET Core (PowerShell 6+); on Windows PowerShell
# 5.1 the original current-directory / PATH resolution is used instead.
if ("System.Runtime.InteropServices.NativeLibrary" -as [type]) {
    [LunarAPI]::RegisterDllResolver($PSScriptRoot)
}

function Get-LunarDate {
    [CmdletBinding()]
    param (
        [Parameter(Mandatory = $false, HelpMessage = "The date (Gregorian) to convert")]
        [datetime]$Date = (Get-Date)
    )

    $offsetHours = [TimeZoneInfo]::Local.GetUtcOffset($Date).TotalHours

    try {
        $lunar = [LunarAPI]::GetLunar($offsetHours, $Date.Year, $Date.Month, $Date.Day)
        return $lunar
    } catch [System.DllNotFoundException] {
        Write-Error "Dependency missing: 'liblcca.dll' could not be found. Please ensure it is located in the current working directory or added to your system's PATH."
    } catch {
        Write-Error "An unexpected error occurred while calling the Lunar API: $_"
    }

    return $lunar
}

$ChineseDigits = @(
    "",
    "一",
    "二",
    "三",
    "四",
    "五",
    "六",
    "七",
    "八",
    "九"
)

$ChineseBranches = @(
    "申",
    "酉",
    "戌",
    "亥",
    "子",
    "丑",
    "寅",
    "卯",
    "辰",
    "巳",
    "午",
    "未"
)

$ChineseStems = @(
    "庚",
    "辛",
    "壬",
    "癸",
    "甲",
    "乙",
    "丙",
    "丁",
    "戊",
    "己"
)

$VietnameseDigits = @(
    "",
    "một",
    "hai",
    "ba",
    "bốn",
    "năm",
    "sáu",
    "bảy",
    "tám",
    "chín"
)

$VietnameseBranches = @(
    "Thân",
    "Dậu",
    "Tuất",
    "Hợi",
    "Tí",
    "Sửu",
    "Dần",
    "Mão",
    "Thìn",
    "Tị",
    "Ngọ",
    "Mùi"
)

$VietnameseStems = @(
    "Canh",
    "Tân",
    "Nhâm",
    "Quý",
    "Giáp",
    "Ất",
    "Bính",
    "Đinh",
    "Mậu",
    "Kỉ"
)

function Get-LunarYearBranch {
    [CmdletBinding()]
    param (
        [Parameter(Mandatory = $true, HelpMessage = "Approximate Gregorian year of the lunar year")]
        [int]$Year,

        [Parameter(Mandatory = $true, HelpMessage = "Language")]
        [ValidateSet("Chinese", "Vietnamese")]
        [string]$Language
    )
    $branchNumber = (($Year % 12) + 12) % 12
    switch ($Language) {
        "Chinese" { return $script:ChineseBranches[$branchNumber] }
        "Vietnamese" { return $script:VietnameseBranches[$branchNumber] }
        default { return $script:ChineseBranches[$branchNumber] }
    }
}

function Get-LunarYearStem {
    [CmdletBinding()]
    param (
        [Parameter(Mandatory = $true, HelpMessage = "Approximate Gregorian year of the lunar year")]
        [int]$Year,

        [Parameter(Mandatory = $true, HelpMessage = "Language")]
        [ValidateSet("Chinese", "Vietnamese")]
        [string]$Language
    )
    $stemNumber = (($Year % 10) + 10) % 10
    switch ($Language) {
        "Vietnamese" { return $script:VietnameseStems[$stemNumber] }
        default {
            # Chinese
            return $script:ChineseStems[$stemNumber]
        }
    }
}

function Get-LunarYearRepresentation {
    [CmdletBinding()]
    param (
        [Parameter(Mandatory = $true, HelpMessage = "Approximate Gregorian year of the lunar year")]
        [int]$Year,

        [Parameter(Mandatory = $true, HelpMessage = "Language")]
        [ValidateSet("Chinese", "Vietnamese")]
        [string]$Language
    )
    $stem = Get-LunarYearStem -Year $Year -Language $Language
    $branch = Get-LunarYearBranch -Year $Year -Language $Language
    switch ($Language) {
        "Vietnamese" {
            return "năm " + $stem + " " + $branch
        }
        default {
            # Chinese
            return $stem + $branch + "年"
        }
    }
}

function ConvertTo-ChineseNumeral {
    [CmdletBinding()]
    param (
        [int]$Value
    )
    if (($Value -ge 100) -or ($Value -le 0)) {
        return "$Value"
    }
    $units = $Value % 10
    $tens = [Math]::Floor($Value / 10.0)
    if ($tens -eq 0) {
        return $script:ChineseDigits[$units]
    }
    if ($tens -eq 1) {
        return "十" + $script:ChineseDigits[$units]
    }
    return $script:ChineseDigits[$tens] + "十" + $script:ChineseDigits[$units]
}

function ConvertTo-VietnameseNumeral {
    [CmdletBinding()]
    param (
        [int]$Value
    )
    if (($Value -ge 100) -or ($Value -le 0)) {
        return "$Value"
    }
    if ($Value -eq 10) {
        return "mười"
    }
    if ($Value -eq 15) {
        return "mười lăm"
    }
    $units = $Value % 10
    $tens = [Math]::Floor($Value / 10.0)
    if ($tens -eq 0) {
        return $script:VietnameseDigits[$units]
    }
    if ($tens -eq 1) {
        return "mười " + $script:VietnameseDigits[$units]
    }
    if ($units -eq 0) {
        return $script:VietnameseDigits[$tens] + " mươi"
    }
    if ($units -eq 5) {
        return $script:VietnameseDigits[$tens] + " mươi lăm"
    }
    return $script:VietnameseDigits[$tens] + " mươi " + $script:VietnameseDigits[$units]
}

function Get-LunarMonthRepresentation {
    [CmdletBinding()]
    param (
        [Parameter(Mandatory = $true, HelpMessage = "Lunar month, from 1 to 12")]
        [int]$Month,

        [Parameter(Mandatory = $true, HelpMessage = "Language")]
        [ValidateSet("Chinese", "Vietnamese")]
        [string]$Language
    )
    switch ($Language) {
        "Vietnamese" {
            switch ($Month) {
                1 { return "tháng giêng"}
                4 { return "tháng tư" }
                11 { return "tháng một" }
                12 { return "tháng chạp" }
                default {
                    $result = ConvertTo-VietnameseNumeral -Value $Month
                    return "tháng " + $result
                }
            }
        }
        default {
            # Chinese
            $result = ConvertTo-ChineseNumeral -Value $Month
            if ($result -eq "一") {
                return "正月"
            }
            return $result + "月"
        }
    }
}

function Get-LunarDayRepresentation {
    [CmdletBinding()]
    param (
        [Parameter(Mandatory = $true, HelpMessage = "Lunar day of month, from 1 to 29 or 30")]
        [int]$Day,

        [Parameter(Mandatory = $true, HelpMessage = "Language")]
        [ValidateSet("Chinese", "Vietnamese")]
        [string]$Language
    )
    switch ($Language) {
        "Vietnamese" {
            switch ($Day) {
                15 {
                    return "rằm"
                }
                { $_ -le 10 } {
                    return "mùng " + (ConvertTo-VietnameseNumeral -Value $Day)
                }
                default {
                    return "ngày " + (ConvertTo-VietnameseNumeral -Value $Day)
                }
            }
        }
        default {
            # Chinese
            $result = (ConvertTo-ChineseNumeral -Value $Day)
            if (($result.Length -gt 2) -and ($result.Substring(0, 2) -eq "二十")) {
                $result = "廿" + $result.Substring(2)
            }
            elseif ($result.Length -eq 1) {
                $result = "初" + $result
            }
            return $result + "日"
        }
    }
}

function Get-LunarDateRepresentation {
    [CmdletBinding()]
    param (
        [Parameter(Mandatory = $false, HelpMessage = "Lunar date")]
        $Lunar = (Get-LunarDate),

        [Parameter(Mandatory = $true, HelpMessage = "Language")]
        [ValidateSet("Chinese", "Vietnamese")]
        [string]$Language
    )
    
    if (($null -ne $Lunar) -and ($Lunar.GetType().Name -ne 'lcca_lunar_date')) {
        throw "Invalid type: `$Lunar must be of type 'lcca_lunar_date'. Ensure liblcca.dll is loading correctly."
    }

    $year = (Get-LunarYearRepresentation -Year $Lunar.Year -Language $Language)
    $month = (Get-LunarMonthRepresentation -Month $Lunar.Month -Language $Language)
    $day = (Get-LunarDayRepresentation -Day $Lunar.Day -Language $Language)

    switch ($Language) {
        "Vietnamese" {
            $leap = if ($Lunar.Leap) { " nhuận" } else { "" }
            $monthSize = if ($Lunar.MonthSize -eq 29) { "(thiếu)" } else { "đủ" }
            return $day + " " + $month + $leap + " " + $monthSize + " " + $year
        }
        default {
            # Chinese
            $leap = if ($Lunar.Leap) { "閏" } else { "" }
            $monthSize = if ($Lunar.MonthSize -eq 29) { "（小）" } else { "（大）" }
            return $year + $leap + $month + $monthSize + $day
        }
    }
}
