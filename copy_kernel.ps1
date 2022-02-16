$vhdpath = $args[0]

$disk_number = (Get-DiskImage -ImagePath $vhdpath).Number

IF (!$disk_number) {
    Mount-DiskImage -ImagePath $vhdpath
}

$disk_number = (Get-DiskImage -ImagePath $vhdpath).Number

$target_folder = -join((Get-Partition -DiskNumber $disk_number).DriveLetter, ':\')
$target_user_folder = -join((Get-Partition -DiskNumber $disk_number).DriveLetter, ':\bin')

cp build\kernel.elf $target_folder\

get-childitem build\user\ -Attributes !Directory -filter *elf -recurse | copy-item -Destination $target_user_folder

DisMount-DiskImage -ImagePath $vhdpath
