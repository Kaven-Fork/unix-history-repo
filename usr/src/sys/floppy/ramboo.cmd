!
! BOOTSTRAP ON UP, GOING MULTI USER
!
SET DEF HEX
SET DEF LONG
SET REL:0
HALT
UNJAM
INIT
LOAD BOOT
D R10 9		! DEVICE CHOICE 9=RA
D R11 0		! 0= AUTOBOOT
START 2
