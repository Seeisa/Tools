function Get-CRC32 {
    [CmdletBinding()]
    param (
    [Parameter(Position = 0, ValueFromPipeline = $true,Mandatory = $true,HelpMessage = ' Array of Bytes to use for CRC calculation')]
    [ValidateNotNullOrEmpty()]
    [byte[]]$InputObject
    )

    Begin {
        function New-CrcTable {
            [uint32]$c = $null
            $script:crcTable = New-Object 'System.Uint32[]' 256
            for ($i = 0; $i -lt 256; $i++) {
                $c = $i
                for ($j = 0; $j -lt 8; $j++) {
                    if ($c -band 1) {
                        $c = (($c -shr 1) -bxor 0xEDB88320L)
                    }
                    else {
                        $c = ($c -shr 1)
                    }
                }

                $script:crcTable[$i] = $c
            }
        }

        function Update-Crc ([byte[]]$buffer, [int]$length) {
            [uint32]$crc = 0xffffffffL
            if (-not($script:crcTable)) {
                New-CrcTable
            }
    
            for ($n = 0; $n -lt $length; $n++) {
                $crc = (($crc -shr 8) -band 0x00FFFFFFL) -bxor ($script:crcTable[($crc -bxor $buffer[$n]) -band 0xFF])
            }

            return $crc
        }
    }

    End {
        $inputLength = $InputObject.Length
        return (Update-Crc -buffer $InputObject -length $inputLength)
    }
}