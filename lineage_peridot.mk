#
# Copyright (C) 2024 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit_only.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# Peridot optional feature profile. Preserve the existing Lineage product.
PERIDOT_INCLUDE_XIAOMI_PARTS := true
PERIDOT_INCLUDE_CHARGING_DAEMON := true
PERIDOT_INCLUDE_LINEAGE_HEALTH := true
PERIDOT_ENABLE_LINEAGE_HEALTH_CHARGING_CONTROL := true
PERIDOT_ENABLE_LINEAGE_HEALTH_BYPASS := true
PERIDOT_INCLUDE_MIUI_CAMERA := true
PERIDOT_INCLUDE_GAMEBAR := false
PERIDOT_INCLUDE_GAMESPACE := false
include device/xiaomi/peridot/config/peridot_features.mk

# Inherit some common Lineage stuff.
$(call inherit-product, vendor/lineage/config/common_full_phone.mk)

# Inherit from peridot device
$(call inherit-product, device/xiaomi/peridot/device.mk)

PRODUCT_NAME := lineage_peridot
PRODUCT_DEVICE := peridot
PRODUCT_MANUFACTURER := Xiaomi
PRODUCT_BRAND := POCO
PRODUCT_MODEL := 24069PC21G

PRODUCT_SYSTEM_NAME := peridot_global
PRODUCT_SYSTEM_DEVICE := peridot

PRODUCT_BUILD_PROP_OVERRIDES += \
    BuildDesc="peridot_global-user 16 BP2A.250605.031.A3 OS3.0.6.0.WNPMIXM release-keys" \
    BuildFingerprint=POCO/peridot_global/peridot:16/BP2A.250605.031.A3/OS3.0.6.0.WNPMIXM:user/release-keys \
    DeviceName=$(PRODUCT_SYSTEM_DEVICE) \
    DeviceProduct=$(PRODUCT_SYSTEM_NAME)

PRODUCT_GMS_CLIENTID_BASE := android-xiaomi
