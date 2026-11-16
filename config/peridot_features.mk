# Peridot product feature switches.
# Keep defaults disabled so another ROM can inherit device.mk without
# automatically pulling ROM-specific apps and services.
PERIDOT_INCLUDE_XIAOMI_PARTS ?= false
PERIDOT_INCLUDE_CHARGING_DAEMON ?= false
PERIDOT_INCLUDE_LINEAGE_HEALTH ?= false
PERIDOT_ENABLE_LINEAGE_HEALTH_CHARGING_CONTROL ?= false
PERIDOT_ENABLE_LINEAGE_HEALTH_BYPASS ?= false
PERIDOT_INCLUDE_MIUI_CAMERA ?= false
PERIDOT_INCLUDE_GAMEBAR ?= false
PERIDOT_INCLUDE_GAMESPACE ?= false

# Infinity controls GameSpace from its common product configuration.
ifeq ($(PERIDOT_INCLUDE_GAMESPACE),true)
TARGET_EXCLUDES_GAMESPACE := false
else
TARGET_EXCLUDES_GAMESPACE := true
endif
