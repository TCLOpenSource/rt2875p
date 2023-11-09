ifeq ($(CONFIG_RTK_KDRV_DEMOD), y)

KBUILD_CPPFLAGS  += -I$(srctree)/drivers/rtk_kdriver/japan4k_dmx/$(CHIP_NAME)
KBUILD_CPPFLAGS  += -I$(srctree)/drivers/rtk_kdriver/demod/$(CHIP_NAME)
KBUILD_CPPFLAGS  += -I$(srctree)/drivers/rtk_kdriver/demod/$(CHIP_NAME)/demodcomm
KBUILD_CPPFLAGS  += -I$(srctree)/drivers/rtk_kdriver/demod/$(CHIP_NAME)/demodcore

KBUILD_CPPFLAGS  += -I$(srctree)/drivers/rtk_kdriver/common/include/rbus

#KBUILD_CPPFLAGS  += -I$(srctree)/drivers/rtk_kdriver/demod/CXD2856Family_refcode/
#KBUILD_CPPFLAGS  += -I$(srctree)/drivers/rtk_kdriver/demod/CXD2856Family_refcode/i2c
#KBUILD_CPPFLAGS  += -I$(srctree)/drivers/rtk_kdriver/demod/CXD2856Family_refcode/isdb_terr

rtkdemod-objs := DemodMgr.o \
                                     demod_debug.o \
                                     demod.o \
                                     demod_rtk_all.o \
                                     demod_rtk_r_atsc.o \
                                     demod_rtk_r_atsc3.o \
                                     demod_rtk_r_qam.o \
                                     demod_rtk_r_atv.o \
                                     demod_rtk_r_isdbt.o \
                                     demod_rtk_r_dvbc.o \
                                     demod_rtk_a_dvbtx.o \
                                     demod_rtk_a_dvbsx.o \
                                     demod_rtk_h_dtmb.o \
                                     demod_rtk_common.o \
#									 demod_rtk_s_isdbt.o \
                                     					
obj-y += rtkdemod.o 
obj-y +=demodcore/
obj-y +=demodcomm/
#obj-y +=CXD2856Family_refcode/


else ifeq ($(CONFIG_RTK_KDRV_DEMOD), m)
KBUILD_CPPFLAGS  += -I$(srctree)/drivers/rtk_kdriver/japan4k_dmx/$(CHIP_NAME)
KBUILD_CPPFLAGS  += -I$(srctree)/drivers/rtk_kdriver/demod/$(CHIP_NAME)
KBUILD_CPPFLAGS  += -I$(srctree)/drivers/rtk_kdriver/demod/$(CHIP_NAME)/demodcomm
KBUILD_CPPFLAGS  += -I$(srctree)/drivers/rtk_kdriver/demod/$(CHIP_NAME)/demodcore
KBUILD_CPPFLAGS  += -I$(srctree)/drivers/rtk_kdriver/common/include/rbus

CLEAN_FILES = *.o *.ko *~ core .depend .*.cmd *.mod.c *.bak .tmp_versions Module.symvers \
							demodcore/*.o demodcore/*.ko demodcore/*~ core demodcore/.depend demodcore/.*.cmd \
							demodcore/*.mod.c demodcore/*.bak demodcore/.tmp_versions demodcore/Module.symvers

rtkdemod-objs := DemodMgr.o \
                                     demod_debug.o \
                                     demod.o \
                                     demod_rtk_all.o \
                                     demod_rtk_r_atsc.o \
                                     demod_rtk_r_atsc3.o \
                                     demod_rtk_r_qam.o \
                                     demod_rtk_r_atv.o \
                                     demod_rtk_r_isdbt.o \
                                     demod_rtk_r_dvbc.o \
                                     demod_rtk_a_dvbtx.o \
                                     demod_rtk_a_dvbsx.o \
                                     demod_rtk_h_dtmb.o \
                                     demod_rtk_common.o \
                                     demodcore/foundation.o \
                                     demodcore/math_mpi.o \
                                     demodcore/isdbt_demod_rtk_r.o \
                                     demodcore/isdbt_demod_rtk_r_base.o \
                                     demodcore/atsc_demod_rtk_r.o \
                                     demodcore/atsc_demod_rtk_r_base.o \
                                     demodcore/atsc3_demod_rtk_r.o \
                                     demodcore/atsc3_demod_rtk_r_base.o \
                                     demodcore/qam_demod_rtk_r.o \
                                     demodcore/qam_demod_rtk_r_base.o \
                                     demodcore/dvbtx_demod_rtk_a.o \
                                     demodcore/dvbtx_demod_rtk_a_base.o \
                                     demodcore/dvbtx_rom_data.o \
                                     demodcore/dvbsx_demod_rtk_a.o \
                                     demodcore/dvbsx_demod_rtk_a_base.o \
                                     demodcore/dvbsx_blindscan_demod_rtk_a.o \
                                     demodcore/dvbsx_rom_data.o \
                                     demodcore/dtmb_demod_base.o \
                                     demodcore/dtmb_demod_rtk.o \
                                     demodcore/dtmb_demod_rtk_user.o \
                                     demodcore/diseqc_rtk.o \
                                     demodcore/ifd_new.o \
                                     demodcore/ifd_agc.o \
                                     demodcore/ifd_cr.o \
                                     demodcore/ifd_video.o \
                                     demodcore/ifd_table.o \
                                     demodcore/ifd_analog.o \
                                     demodcomm/tv_osal.o \
                                     demodcomm/comm.o \
                                     demodcomm/tuner.o \
                                     demodcomm/tv_fe_aux.o

obj-m += rtkdemod.o


else ifeq ($(CONFIG_RTK_KDRV_KHAL_DEMOD), y)

KBUILD_CPPFLAGS  += -I$(srctree)/drivers/rtk_kdriver/japan4k_dmx/$(CHIP_NAME)
KBUILD_CPPFLAGS  += -I$(srctree)/drivers/rtk_kdriver/demod/$(CHIP_NAME)
KBUILD_CPPFLAGS  += -I$(srctree)/drivers/rtk_kdriver/demod/$(CHIP_NAME)/demodcomm
KBUILD_CPPFLAGS  += -I$(srctree)/drivers/rtk_kdriver/demod/$(CHIP_NAME)/demodcore
KBUILD_CPPFLAGS  += -I$(srctree)/drivers/rtk_kdriver/common/include/rbus

CLEAN_FILES = *.o *.ko *~ core .depend .*.cmd *.mod.c *.bak .tmp_versions Module.symvers \
							demodcore/*.o demodcore/*.ko demodcore/*~ core demodcore/.depend demodcore/.*.cmd \
							demodcore/*.mod.c demodcore/*.bak demodcore/.tmp_versions demodcore/Module.symvers

demod_khal-objs := DemodMgr.o \
                                     demod_debug.o \
                                     demod.o \
                                     demod_rtk_all.o \
                                     demod_rtk_r_atsc.o \
                                     demod_rtk_r_atsc3.o \
                                     demod_rtk_r_qam.o \
                                     demod_rtk_r_isdbt.o \
                                     demod_rtk_r_dvbc.o \
                                     demod_rtk_r_atv.o \
                                     demod_rtk_a_dvbtx.o \
                                     demod_rtk_a_dvbsx.o \
                                     demod_rtk_h_dtmb.o \
                                     demod_rtk_common.o \
                                     khal_demod_dvb.o \
                                     khal_demod_isdbt.o \
                                     khal_demod_dtmb.o \
                                     khal_demod_vqi.o \
								 khal_demod_all.o \
								 khal_demod_atsc3.o \
								 khal_demod_atv.o \
                                     rtk_demod_khal_transform.o \
                                     demodcore/foundation.o \
                                     demodcore/math_mpi.o \
                                     demodcore/isdbt_demod_rtk_r.o \
                                     demodcore/isdbt_demod_rtk_r_base.o \
                                     demodcore/atsc_demod_rtk_r.o \
                                     demodcore/atsc_demod_rtk_r_base.o \
                                     demodcore/atsc3_demod_rtk_r.o \
                                     demodcore/atsc3_demod_rtk_r_base.o \
                                     demodcore/qam_demod_rtk_r.o \
                                     demodcore/qam_demod_rtk_r_base.o \
                                     demodcore/dvbtx_demod_rtk_a.o \
                                     demodcore/dvbtx_demod_rtk_a_base.o \
                                     demodcore/dvbtx_rom_data.o \
                                     demodcore/dvbsx_demod_rtk_a.o \
                                     demodcore/dvbsx_demod_rtk_a_base.o \
                                     demodcore/dvbsx_blindscan_demod_rtk_a.o \
                                     demodcore/dvbsx_rom_data.o \
                                     demodcore/dtmb_demod_base.o \
                                     demodcore/dtmb_demod_rtk.o \
                                     demodcore/dtmb_demod_rtk_user.o \
                                     demodcore/diseqc_rtk.o \
                                     demodcore/ifd_new.o \
                                     demodcore/ifd_agc.o \
                                     demodcore/ifd_cr.o \
                                     demodcore/ifd_video.o \
                                     demodcore/ifd_table.o \
                                     demodcore/ifd_analog.o \
                                     demodcomm/tv_osal.o \
                                     demodcomm/comm.o \
                                     demodcomm/tuner.o \
                                     demodcomm/tv_fe_aux.o
					         
obj-m += demod_khal.o  

endif
