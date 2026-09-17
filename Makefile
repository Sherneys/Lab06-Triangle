BUILD := build
PARTS := info triangle cost stripes earlyz index probe

.PHONY: all clean $(PARTS)

all:
	cmake -S . -B $(BUILD) -DCMAKE_BUILD_TYPE=Release
	cmake --build $(BUILD)

$(PARTS):
	cmake -S . -B $(BUILD) -DCMAKE_BUILD_TYPE=Release
	cmake --build $(BUILD) --target $@
	@echo "running ./$(BUILD)/$@"
	@./$(BUILD)/$@ $(ARGS)

clean:
	rm -rf $(BUILD)
