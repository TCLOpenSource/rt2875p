# Makefile for the Realtek scaler VIP drivers.
#

mkfile_path:=$(shell pwd)/$(lastword $(MAKEFILE_LIST))
mkfile_path:=$(shell dirname $(mkfile_path))

ST2094OBJ:=ST2094.o.static

ifeq ("", "$(wildcard $(mkfile_path)/$(ST2094OBJ))")
ST2094OBJ=ST2094.o
endif


obj-y := ultrazoom.o di_ma.o scalerColor.o dcc.o color.o film.o icm.o intra.o nr.o custom_sharpness.o xc.o peaking.o gibi_od.o pq_misc_table.o pq_adaptive.o localdimming.o vip_panel_default.o viptable.o scalerColor_tv006.o localcontrast.o memc_mux.o pq_3dlut_table.o pq_rpc.o still_logo_detect.o tc_hdr.o tc_math.o tc_OEOTF_convert.o pcid.o valc.o fcic.o hdr_table.o module_test_table.o $(ST2094OBJ) ai_pq.o srnn.o srnn_table.o frequency_detection.o pdf.o SIP.o pqmask.o dtm.o GSR2.o GSR_adapter.o  localdimmingInterface_table.o localdimmingInterface.o vip_ring_buffer.o APLSatGain.o HDR10_vivid.o


obj-$(CONFIG_RTK_PQMASK_DRV) += scalerPQMaskCtrl.o scalerPQMaskColorLib.o

KBUILD_CFLAGS += -Werror
