all: rsp-ruination.z64
.PHONY: all

BUILD_DIR = build
include $(N64_INST)/include/n64.mk

CFLAGS+=-DN64_BIG_ENDIAN

OBJS = $(BUILD_DIR)/rsp-ruination.o  $(BUILD_DIR)/rsp_state.o  $(BUILD_DIR)/rsp_vector_instructions.o  $(BUILD_DIR)/rsp_test_ucode.o

rsp-ruination.z64: N64_ROM_TITLE = "RSP Ruination"

$(BUILD_DIR)/rsp-ruination.elf: $(OBJS)

clean:
	rm -rf $(BUILD_DIR) *.z64
.PHONY: clean

-include $(wildcard $(BUILD_DIR)/*.d))
