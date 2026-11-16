# Optional peridot product features

`device.mk` contains the reusable peridot hardware configuration. ROM products
choose optional applications and ROM-facing services before inheriting it.

Include `config/peridot_features.mk` before the ROM common product makefile and
`device.mk`, then set only the features required by that product.

```make
PERIDOT_INCLUDE_XIAOMI_PARTS := true
PERIDOT_INCLUDE_CHARGING_DAEMON := true
PERIDOT_INCLUDE_LINEAGE_HEALTH := true
PERIDOT_ENABLE_LINEAGE_HEALTH_CHARGING_CONTROL := true
PERIDOT_ENABLE_LINEAGE_HEALTH_BYPASS := true
PERIDOT_INCLUDE_MIUI_CAMERA := true
PERIDOT_INCLUDE_GAMEBAR := false
PERIDOT_INCLUDE_GAMESPACE := false
include device/xiaomi/peridot/config/peridot_features.mk
```

All switches default to `false`. The shipped `infinity_peridot` and
`lineage_peridot` products explicitly preserve their previous package sets.

## Switch ownership

- `PERIDOT_INCLUDE_XIAOMI_PARTS`: installs the privileged XiaomiParts APK.
- `PERIDOT_INCLUDE_CHARGING_DAEMON`: installs `peridot-charging`.
- `PERIDOT_INCLUDE_LINEAGE_HEALTH`: installs the Lineage Health service and
  exposes the three fast-charge values.
- `PERIDOT_ENABLE_LINEAGE_HEALTH_CHARGING_CONTROL`: enables Lineage Health
  toggle/limit paths.
- `PERIDOT_ENABLE_LINEAGE_HEALTH_BYPASS`: advertises bypass through Lineage
  Health when charging control is enabled.
- `PERIDOT_INCLUDE_MIUI_CAMERA`: inherits the MiuiCamera product fragment when
  its repository exists.
- `PERIDOT_INCLUDE_GAMEBAR`: inherits the external GameBar product fragment.
- `PERIDOT_INCLUDE_GAMESPACE`: controls Infinity's existing
  `TARGET_EXCLUDES_GAMESPACE` switch.

The kernel charging and bypass sysfs backend remains part of the peridot
hardware core. These product switches prevent ROM-specific frontends and
services from being forced on every ROM; they do not alter the tested kernel
ABI.
