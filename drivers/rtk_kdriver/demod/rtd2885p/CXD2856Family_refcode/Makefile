KBUILD_CPPFLAGS  += -I drivers/rtk_kdriver/demod
KBUILD_CPPFLAGS  += -I drivers/rtk_kdriver/demodcomm
KBUILD_CPPFLAGS  += -I drivers/rtk_kdriver/demod/CXD2856Family_refcode
KBUILD_CPPFLAGS  += -I drivers/rtk_kdriver/demod/CXD2856Family_refcode/i2c
KBUILD_CPPFLAGS  += -I drivers/rtk_kdriver/demod/CXD2856Family_refcode/lib
KBUILD_CPPFLAGS  += -I drivers/rtk_kdriver/demod/CXD2856Family_refcode/isdb_terr

CXD2856Family_refcode-objs := sony_demod.o \
sony_common.o \
sony_stopwatch_port.o \
					
obj-y += CXD2856Family_refcode.o 
obj-y += i2c/
obj-y += lib/
obj-y += isdb_terr/