udo insmod /home/student/Desktop/message_slot.ko;
sudo mknod /dev/slot0 c 240 1;
while ~/Desktop/tester2.out /dev/slot0 ; do :;done
