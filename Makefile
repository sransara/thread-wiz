# Tools
CLANG ?= clang
CC ?= gcc
BPFTOOL ?= bpftool

# Directories
APP_SRC_DIR := src
BPF_SRC_DIR := bpf
BUILD_DIR := build

VENDOR_LIBBPF_DIR := vendor/libbpf
VENDOR_LIBBPF_SRC_DIR := $(VENDOR_LIBBPF_DIR)/src

APP_NAME := pthread-wiz

BPF_FLAGS := -g -O2 -target bpf
INCLUDES := -I$(BUILD_DIR)
CPP_FLAGS := -g -Wall -Werror

ifeq ($(V),1)
	Q =
	msg =
else
	Q = @
	msg = @printf '%-8s %s %s\n' "$(1)" "$(2)" "$(if $(3), $(3))";
endif

.PHONY: none
none:

# Build libbpf
$(BUILD_DIR)/libbpf.a: $(wildcard $(VENDOR_LIBBPF_SRC_DIR)/*.[ch] $(VENDOR_LIBBPF_SRC_DIR)/Makefile)
	$(call msg,LIB,$@)
	$(Q)$(MAKE) -C $(VENDOR_LIBBPF_SRC_DIR) BUILD_STATIC_ONLY=1 \
		OBJDIR=$(abspath ${BUILD_DIR})/libbpf DESTDIR=$(abspath ${BUILD_DIR}) \
		INCLUDEDIR= LIBDIR= UAPIDIR= \
		install

# Build BPF code
$(BUILD_DIR)/%.bpf.o: $(BPF_SRC_DIR)/%.bpf.c $(BUILD_DIR)/libbpf.a
	$(call msg,BPF,$@)
	$(Q)$(CLANG) $(BPF_FLAGS) $(INCLUDES) \
		-c $(filter %.c,$^) -o $(patsubst %.bpf.o,%.tmp.bpf.o,$@)
	$(Q)$(BPFTOOL) gen object $@ $(patsubst %.bpf.o,%.tmp.bpf.o,$@)

# Generate BPF skeletons
$(BUILD_DIR)/%.skel.h: $(BUILD_DIR)/%.bpf.o
	$(call msg,GEN-SKEL,$@)
	$(Q)$(BPFTOOL) gen skeleton $< > $@

# Build user space code
$(BUILD_DIR)/%.o: $(APP_SRC_DIR)/%.c $(BUILD_DIR)/%.skel.h
	$(call msg,CC,$@)
	$(Q)$(CC) $(CPP_FLAGS) $(INCLUDES) -c $(filter %.c,$^) -o $@

# Build user space application binary
$(BUILD_DIR)/%.exe: $(BUILD_DIR)/%.o $(BUILD_DIR)/libbpf.a
	$(call msg,BINARY,$@)
	$(Q)$(CC) $(CPP_FLAGS) $(INCLUDES) $^ -lelf -lz -o $@

.PHONY: bpf
bpf: $(BUILD_DIR)/main.skel.h

.PHONY: app
app: $(BUILD_DIR)/main.exe
	$(call msg,APP,Linking to executable)
	$(Q)ln -sf main.exe $(BUILD_DIR)/$(APP_NAME)

.PHONY: clean
clean:
	$(call msg,CLEAN)
	$(Q)rm -rf $(BUILD_DIR)
