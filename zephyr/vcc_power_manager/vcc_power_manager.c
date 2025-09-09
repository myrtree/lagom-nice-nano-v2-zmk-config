/*
 * Custom ext-power driver for nice!nano v2 VCC control
 * Properly integrates P0.13 VCC control with ZMK ext-power system
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/pm/device.h>

LOG_MODULE_REGISTER(vcc_power_manager, CONFIG_ZMK_LOG_LEVEL);

#ifdef CONFIG_ZMK_VCC_POWER_MANAGER

struct vcc_power_config {
    struct gpio_dt_spec control_gpio;
};

struct vcc_power_data {
    bool enabled;
};

static int vcc_power_enable(const struct device *dev) {
    const struct vcc_power_config *config = dev->config;
    struct vcc_power_data *data = dev->data;
    
    if (!data->enabled) {
        LOG_INF("Enabling VCC power (P0.13 = HIGH)");
        gpio_pin_set_dt(&config->control_gpio, 1);
        data->enabled = true;
        k_msleep(100); // Allow components to power up
    }
    
    return 0;
}

static int vcc_power_disable(const struct device *dev) {
    const struct vcc_power_config *config = dev->config;
    struct vcc_power_data *data = dev->data;
    
    if (data->enabled) {
        LOG_INF("Disabling VCC power (P0.13 = LOW)");
        gpio_pin_set_dt(&config->control_gpio, 0);
        data->enabled = false;
    }
    
    return 0;
}

static int vcc_power_get(const struct device *dev) {
    struct vcc_power_data *data = dev->data;
    return data->enabled ? 1 : 0;
}

static const struct zmk_ext_power_driver_api vcc_power_api = {
    .enable = vcc_power_enable,
    .disable = vcc_power_disable,
    .get = vcc_power_get,
};

static int vcc_power_init(const struct device *dev) {
    const struct vcc_power_config *config = dev->config;
    struct vcc_power_data *data = dev->data;
    
    LOG_INF("Initializing VCC power manager for nice!nano v2");
    
    if (!device_is_ready(config->control_gpio.port)) {
        LOG_ERR("VCC control GPIO not ready");
        return -ENODEV;
    }
    
    int ret = gpio_pin_configure_dt(&config->control_gpio, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        LOG_ERR("Failed to configure VCC control GPIO: %d", ret);
        return ret;
    }
    
    // Initially enable VCC power
    gpio_pin_set_dt(&config->control_gpio, 1);
    data->enabled = true;
    
    LOG_INF("VCC power manager initialized - VCC enabled");
    
    return 0;
}

static struct vcc_power_data vcc_power_data_0;

static const struct vcc_power_config vcc_power_config_0 = {
    .control_gpio = GPIO_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), gpios, 0),
};

DEVICE_DT_DEFINE(DT_NODELABEL(vcc_power_ctrl), vcc_power_init, NULL,
                 &vcc_power_data_0, &vcc_power_config_0,
                 POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
                 &vcc_power_api);

#endif /* CONFIG_ZMK_VCC_POWER_MANAGER */
