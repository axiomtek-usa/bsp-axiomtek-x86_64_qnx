ROOT_DIR := $(notdir $(CURDIR))
ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

BUILD_TEMPLATE=../../../Templates_build_sdp80x
INSTALL=../../install
BOARD=x86_64-generic-uefi
HOST_MKIFS := mkifs

.PHONY: all clean

# Note that the hypervisor-enabled boot image variant, does not build by default.
# Once you have QNX Hypervisor installed, you can build it by running 'make hyp'.
all: $(BOARD).efi

clean:
	$(RM_HOST) *.efi *.sym 

$(BOARD).efi: $(BOARD).build
	$(HOST_MKIFS) -r $(INSTALL) $(MKIFSFLAGS) $^ $@

-include $(BUILD_TEMPLATE)/template.mk

