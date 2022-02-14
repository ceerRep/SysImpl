$vhdpath = $args[0]

$disk_number = (Get-DiskImage -ImagePath $vhdpath).Number

IF (!$disk_number) {
    Mount-DiskImage -ImagePath $vhdpath
}

$disk_number = (Get-DiskImage -ImagePath $vhdpath).Number

$target_folder = -join((Get-Partition -DiskNumber $disk_number).DriveLetter, ':\')

robocopy build\ $target_folder\ '*.elf' /s /ndl /xj

DisMount-DiskImage -ImagePath $vhdpath
