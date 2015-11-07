TOP_DIR := $(shell cd $(CURDIR)/..;pwd)
OBJ_DIR := $(TOP_DIR)/obj
INCLUDE_PATH := $(addprefix $(TOP_DIR)/, $(INCLUDES))
CCINCLUDES := $(addprefix -I, $(INCLUDE_PATH))
OBJS := $(SRCS:.c=.o)
OBJS_FULL := $(addprefix $(OBJ_DIR)/, $(OBJS))

all : $(TOP_DIR)/obj/$(EXE_NAME)

$(TOP_DIR)/obj/$(EXE_NAME) : $(OBJS_FULL)
	@echo "Making EXE"
	@mkdir -p $(TOP_DIR)/obj/$(EXE_NAME)/
	@$(CC) -o $(TOP_DIR)/obj/$(EXE_NAME)/$(EXE_NAME) $(OBJS_FULL) $(EXT_LIB)

$(OBJ_DIR)/%.o: $(TOP_DIR)/%.c
	@echo "Compiling "$<
	@mkdir -p $(dir $@)
	@$(CC) -c $(CCINCLUDES) $(CCFLAGS) -o $@ $<

clean : 
	rm -r $(OBJ_DIR)