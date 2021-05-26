#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

export TARGET	:=	10SecondRun
NITRODATA		:=	nitrofiles

include $(DEVKITARM)/ds_rules

.PHONY: clean makearm7 makearm9

all:	$(TARGET).nds

makearm7:
	$(MAKE) -C arm7
	cp arm7/$(TARGET).elf $(TARGET).arm7.elf

makearm9:
	$(MAKE) -C arm9
	cp arm9/$(TARGET).elf $(TARGET).arm9.elf

$(TARGET).nds:	makearm7 makearm9
	ndstool	-u 00030004 -g HJUA KJ "10SECRUN" -c $(TARGET).nds -7 $(TARGET).arm7.elf -9 $(TARGET).arm9.elf -d $(NITRODATA) \
	-b icon.bmp "10 Second Run;GO Series;Gamebridge, Rocket Robz"
	python2 patch_ndsheader_dsiware.py $(TARGET).nds --twlTouch

clean:
	@echo clean ...
	@rm -fr data
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).nds
	@rm -fr $(TARGET).arm7.elf
	@rm -fr $(TARGET).arm9.elf
	@$(MAKE) -C arm9 clean
	@$(MAKE) -C arm7 clean

data:
	@mkdir -p data