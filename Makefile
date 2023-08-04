BUILD_DIR=build
include $(N64_INST)/include/n64.mk

OBJS = $(BUILD_DIR)/rdptest.o

all: rdptest.z64

rdptest.z64: N64_ROM_TITLE="RDP Test"

$(BUILD_DIR)/rdptest.elf: $(OBJS)

clean:
	rm -rf $(BUILD_DIR) rdptest.z64

-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: all clean
