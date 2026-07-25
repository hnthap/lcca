# Lunar.Tests.ps1

Import-Module ".\Lunar.psm1" -Force

Describe "Lunar Module Sexagenary Cycle Math" {
    
    Context "Get-LunarYearBranch" {
        It "calculates the correct Chinese branch for known years" {
            Get-LunarYearBranch -Year 2024 -Language "Chinese" | Should -Be "辰"
            Get-LunarYearBranch -Year 2025 -Language "Chinese" | Should -Be "巳"
        }

        It "calculates the correct Vietnamese branch for known years" {
            Get-LunarYearBranch -Year 2024 -Language "Vietnamese" | Should -Be "Thìn"
            Get-LunarYearBranch -Year 2023 -Language "Vietnamese" | Should -Be "Mão"
        }
    }

    Context "Get-LunarYearStem" {
        It "calculates the correct Chinese stem for known years" {
            Get-LunarYearStem -Year 2024 -Language "Chinese" | Should -Be "甲"
            Get-LunarYearStem -Year 2023 -Language "Chinese" | Should -Be "癸"
        }

        It "calculates the correct Vietnamese stem for known years" {
            Get-LunarYearStem -Year 2024 -Language "Vietnamese" | Should -Be "Giáp"
        }
    }

    Context "Get-LunarYearRepresentation" {
        It "combines Stem and Branch correctly in Chinese" {
            Get-LunarYearRepresentation -Year 2024 -Language "Chinese" | Should -Be "甲辰年"
        }

        It "combines Stem and Branch correctly in Vietnamese" {
            Get-LunarYearRepresentation -Year 2024 -Language "Vietnamese" | Should -Be "năm Giáp Thìn"
        }
    }
}

Describe "Lunar Module Numeral Conversions" {
    
    Context "ConvertTo-ChineseNumeral" {
        It "converts single digits" {
            ConvertTo-ChineseNumeral -Value 5 | Should -Be "五"
        }
        It "handles 10 correctly" {
            ConvertTo-ChineseNumeral -Value 10 | Should -Be "十"
        }
        It "converts teens correctly" {
            ConvertTo-ChineseNumeral -Value 15 | Should -Be "十五"
        }
        It "converts higher double digits" {
            ConvertTo-ChineseNumeral -Value 23 | Should -Be "二十三"
        }
    }

    Context "ConvertTo-VietnameseNumeral" {
        It "converts single digits" {
            ConvertTo-VietnameseNumeral -Value 5 | Should -Be "năm"
        }
        It "handles exactly 10" {
            ConvertTo-VietnameseNumeral -Value 10 | Should -Be "mười"
        }
        It "handles special case 15" {
            ConvertTo-VietnameseNumeral -Value 15 | Should -Be "mười lăm"
        }
        It "handles higher tens with a 5" {
            ConvertTo-VietnameseNumeral -Value 25 | Should -Be "hai mươi lăm"
        }
        It "handles standard double digits" {
            ConvertTo-VietnameseNumeral -Value 23 | Should -Be "hai mươi ba"
        }
    }
}

Describe "Lunar Module Date Formatting" {

    Context "Get-LunarMonthRepresentation" {
        It "returns correct special month 1 in Chinese" {
            Get-LunarMonthRepresentation -Month 1 -Language "Chinese" | Should -Be "正月"
        }
        It "returns correct standard month in Chinese" {
            Get-LunarMonthRepresentation -Month 8 -Language "Chinese" | Should -Be "八月"
        }
        It "returns correct special months in Vietnamese" {
            Get-LunarMonthRepresentation -Month 1 -Language "Vietnamese" | Should -Be "tháng giêng"
            Get-LunarMonthRepresentation -Month 12 -Language "Vietnamese" | Should -Be "tháng chạp"
        }
    }

    Context "Get-LunarDayRepresentation" {
        It "formats 1st-10th days correctly in Chinese" {
            Get-LunarDayRepresentation -Day 5 -Language "Chinese" | Should -Be "初五日"
        }
        It "formats 20s correctly in Chinese" {
            Get-LunarDayRepresentation -Day 25 -Language "Chinese" | Should -Be "廿五日"
        }
        It "formats 1st-10th days correctly in Vietnamese" {
            Get-LunarDayRepresentation -Day 5 -Language "Vietnamese" | Should -Be "mùng năm"
        }
        It "formats the 15th correctly in Vietnamese" {
            Get-LunarDayRepresentation -Day 15 -Language "Vietnamese" | Should -Be "rằm"
        }
    }
}

Describe "Module Dependencies and API Structs" {

    It "successfully loaded the lcca_lunar_date struct into the session" {
        $structType = [lcca_lunar_date] -as [type]
        $structType | Should -Not -BeNullOrEmpty
    }

    It "successfully loaded the LunarAPI class into the session" {
        $apiType = [LunarAPI] -as [type]
        $apiType | Should -Not -BeNullOrEmpty
    }
}

Describe "Native liblcca.dll Interop" {

    It "converts a known Gregorian date to the correct lunar date" {
        # 2024-07-19 in +07:00 -> lunar day 14, month 6, not leap, 29-day month.
        # Exercises the native P/Invoke path (fails without a working DLL resolver on .NET Core).
        $lunar = Get-LunarDate -Date '2024-07-19'
        $lunar | Should -Not -BeNullOrEmpty
        $lunar.Year | Should -Be 2024
        $lunar.Month | Should -Be 6
        $lunar.Day | Should -Be 14
        $lunar.Leap | Should -Be 0
        $lunar.MonthSize | Should -Be 29
    }

    It "renders the known date correctly in Chinese" {
        $lunar = Get-LunarDate -Date '2024-07-19'
        Get-LunarDateRepresentation -Lunar $lunar -Language "Chinese" | Should -Be "甲辰年六月（小）十四日"
    }
}
