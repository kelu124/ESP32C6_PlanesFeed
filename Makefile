FQBN := esp32:esp32:esp32c6:CDCOnBoot=cdc,PartitionScheme=huge_app,FlashSize=4M
SKETCH := firmware/esp32_plane
PORT ?= /dev/ttyACM0
BUILD := build

# The `dfu` command sets this flag, and it survives a normal reset. See docs/design.md.
DOWNLOAD_FLAG_REG := 0x600B1034
DOWNLOAD_FLAG_BIT := 0x40000000

.PHONY: test firmware flash leave-dfu setup

test:
	mkdir -p $(BUILD)
	g++ -std=c++17 -Wall -Wextra -o $(BUILD)/test_logic test/test_logic.cpp $(SKETCH)/logic.cpp -lm
	$(BUILD)/test_logic

firmware:
	arduino-cli compile --fqbn $(FQBN) --warnings default --output-dir $(BUILD)/firmware $(SKETCH)

# Clears the flag and restarts the firmware. Does nothing if the board isn't in download mode.
leave-dfu:
	-esptool.py --port $(PORT) --connect-attempts 1 --before no-reset --after hard-reset write-mem $(DOWNLOAD_FLAG_REG) 0 $(DOWNLOAD_FLAG_BIT)

flash: firmware leave-dfu
	sleep 2
	arduino-cli upload --fqbn $(FQBN) --port $(PORT) --input-dir $(BUILD)/firmware $(SKETCH)

# Sends the Wi-Fi details and location from .env to the board over USB.
setup:
	python3 scripts/provision.py --port $(PORT)
