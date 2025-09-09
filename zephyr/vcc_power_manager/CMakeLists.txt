# VCC Power Manager module

zephyr_library()
zephyr_library_sources_ifdef(CONFIG_ZMK_VCC_POWER_MANAGER vcc_power_manager.c)
