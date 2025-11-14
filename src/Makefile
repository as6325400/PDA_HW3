CXX = g++
CXXFLAGS = -O3 -MMD -MP -g
TARGET = ../bin/main
VERIFY = ../verifier/verify
BUILD_DIR = ../build

ARGS = ../testcase/public1.lef ../testcase/public1.def ../output/public.out.def

SRC = $(wildcard core/*.cpp io/*.cpp placer/*.cpp) main.cpp
OBJ = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRC))
DEPS = $(OBJ:.o=.d)

.PHONY: all
all: $(TARGET) 

-include $(DEPS)


$(TARGET): $(OBJ)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: run clean verify
run: $(TARGET)
	@$(TARGET) $(ARGS)

verify: run
	@echo "[VERIFY] $(VERIFY) $(ARGS)"
	@$(VERIFY) $(ARGS)

clean:
	rm -f $(OBJ) $(DEPS) $(TARGET)