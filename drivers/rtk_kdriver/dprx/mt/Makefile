
##############################################
# Targets
##############################################
#SUBDIRS := mt-dprx_vfe/
SUBDIRS += mt-dprx_vfe_detect

##############################################
# Make
##############################################

.PHONY: all clean $(SUBDIRS)

all : clean
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i; done;


clean:
	@set -e; for i in $(SUBDIRS); do $(MAKE) clean -C $$i; done;

