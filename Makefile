ifndef CROSS_COMPILE
#CROSS_COMPILE	:= arm-none-linux-gnueabihf-
CROSS_COMPILE	:= arm-linux-gnueabihf-
endif
#arm-linux-gnueabihf-

CC			:=	${CROSS_COMPILE}gcc
CPP			:=	${CROSS_COMPILE}cpp
AS			:=	${CROSS_COMPILE}gcc
AR			:=	${CROSS_COMPILE}ar
LINKER			:=	${CROSS_COMPILE}ld
OC			:=	${CROSS_COMPILE}objcopy
OD			:=	${CROSS_COMPILE}objdump
NM			:=	${CROSS_COMPILE}nm
PP			:=	${CROSS_COMPILE}gcc -E

BUILD_DIR :=  $(PWD)/build
LIB_DIR := -L$(PWD)/src/lib
INCLUDES		+=	-I./src				\
                    -I./src/include  \
					-I./src/simubams  \
					-I./src/ems	\
					-I./src/lcd	\
					-I./src/plc							

MAKE_DEP = -Wp,-MD,$(DEP) -MT $$@ -MP
#_SOURCES_ += ./src/main.c
_SOURCES_ += $(wildcard ./src/*.c)
_SOURCES_ += $(wildcard ./src/simubams/*.c)
_SOURCES_ += $(wildcard ./src/ems/*.c)
_SOURCES_ += $(wildcard ./src/lcd/*.c)
_SOURCES_ += $(wildcard ./src/plc/*.c)
$(eval OBJS1 := $(patsubst %.c,%.o,$(notdir $(_SOURCES_)))) 
$(info OBJS1 = $(OBJS1))
$(eval OBJS = $(addprefix $(BUILD_DIR)/,$(OBJS1)))
$(info OBJS= $(OBJS))


protest_emu: $(OBJS)
	@echo "55555 xxxxx"

	$(CC)   $(OBJS) $(LIB_DIR) -o protest_emu -lcommon -lpthread -lm -lrt -ldl
#	-cp testpro_emu  /mnt/hgfs/share_E/zgEMU/Program/protest_emu;
	-cp protest_emu  /mnt/hgfs/Tcu_lib/protest_emu
# $(CC)   $(OBJS) $(LIB_DIR) -o bms_simu1 -lcommon -lpthread -lrt -ldl
# -cp bms_simu1  /mnt/hgfs/share_E/zgEMU/Program/bms_simu1;
	-@echo ' '
#-------------------------------------------------------------------
#$(CC)   $(OBJS)  -o bms_simu -ldl -rdynamic
define MAKE_C

$(eval OBJ = $(addprefix $(1)/,$(patsubst %.c,%.o,$(notdir $(2)))))
$(info xxxxxxxxxOBJ= $(OBJ))
$(info yyyyyyyyysrc= $(2))
$(eval DEP := $(patsubst %.o,%.d,$(OBJ)))
$(OBJ): $(2)
	-mkdir -p $(BUILD_DIR)
	@echo "  CC      $$<"
	$(CC) -Wall  -c $(INCLUDES)  $(MAKE_DEP)   $$< -o $$@ 
endef
#-ldl

$(eval $(foreach obj,$(_SOURCES_),$(call MAKE_C,$(BUILD_DIR),$(obj))))
#-------------------------------------------------------------------
.PHONY: clean
clean:
	@echo "clean"
	-rm protest_emu
# 	-rm bms_simu1
	-rm -rf 	$(BUILD_DIR)