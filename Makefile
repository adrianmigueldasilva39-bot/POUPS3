.SUFFIXES:

ifeq ($(strip $(PSL1GHT)),)
$(error "Please set PSL1GHT in your environment")
endif

include $(PSL1GHT)/ppu_rules

TARGET		:=	pou_ps3
BUILD		:=	build
SOURCES		:=	source
INCLUDES	:=	include

TITLE		:=	Pou PS3
APPID		:=	POUPS3001
CONTENTID	:=	UP0001-POUPS3001_00-0000000000000000

CFLAGS		:=	-g -O2 -Wall -mcpu=cell $(MACHDEP) $(INCLUDE)
CXXFLAGS	:=	$(CFLAGS)
LDFLAGS		:=	$(MACHDEP) -Wl,-Map,$(notdir $@).map

LIBS		:=	-lSDL_image -lpng -ljpeg -lz -lSDL -laudio -lio -lsysutil -lsysmodule -lgcm_sys -lreality -lm

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export BUILDDIR	:=	$(CURDIR)/$(BUILD)
export DEPSDIR	:=	$(BUILDDIR)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
OBJS		:=	$(CFILES:.c=.o)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(BUILDDIR)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean pkg

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@rm -rf $(BUILD) $(OUTPUT).elf $(OUTPUT).self $(OUTPUT).pkg pkg

pkg: $(BUILD)
	@make_self_npdrm $(OUTPUT).elf EBOOT.BIN $(CONTENTID)
	@mkdir -p pkg/USRDIR
	@cp EBOOT.BIN pkg/USRDIR/
	@cp corpo.png pkg/USRDIR/
	@cp comida.png pkg/USRDIR/
	@sfo.py --title "$(TITLE)" --appid "$(APPID)" -f $(PSL1GHT)/bin/sfo.xml pkg/PARAM.SFO
	@pkg.py --contentid $(CONTENTID) pkg/ $(OUTPUT).pkg

$(OUTPUT).elf: $(OBJS)
