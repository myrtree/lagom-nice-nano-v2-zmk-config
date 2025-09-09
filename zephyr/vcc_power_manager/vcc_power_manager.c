/*
 * VCC Power Manager for nice!nano v2
 * Controls P0.13 to manage 3.3V VCC power
 * Manual control via ext-power toggle
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

LOG_MODULE_REGISTER(vcc_power_manager, CONFIG_ZMK_LOG_LEVEL);

#ifdef CONFIG_ZMK_VCC_POWER_MANAGER

static const struct gpio_dt_spec vcc_power_pin = GPIO_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), gpios, 0);

static int vcc_power_manager_init(void) {
    LOG_INF("Initializing VCC power manager for nice!nano v2");
    
    if (!device_is_ready(vcc_power_pin.port)) {
        LOG_ERR("VCC power control GPIO not ready");
        return -ENODEV;
    }
    
    // Configure P0.13 as output, initially HIGH (VCC enabled)
    int ret = gpio_pin_configure_dt(&vcc_power_pin, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        LOG_ERR("Failed to configure VCC power GPIO: %d", ret);
        return ret;
    }
    
    // Initially enable VCC power (P0.13 = HIGH)
    // This ensures display and other VCC components work at startup
    gpio_pin_set_dt(&vcc_power_pin, 1);
    
    LOG_INF("VCC power manager initialized - P0.13 set to HIGH");
    LOG_INF("Use ext-power toggle (End key in Fn layer) to control VCC power");
    
    return 0;
}

SYS_INIT(vcc_power_manager_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

#endif /* CONFIG_ZMK_VCC_POWER_MANAGER */
