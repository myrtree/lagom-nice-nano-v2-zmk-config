/*
 * VCC Power Manager for nice!nano v2
 * Automatically controls P0.13 to turn off 3.3V VCC when on battery
 * Works in conjunction with ZMK ext-power system
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/usb/usb_device.h>

LOG_MODULE_REGISTER(vcc_power_manager, CONFIG_ZMK_LOG_LEVEL);

#ifdef CONFIG_ZMK_VCC_POWER_MANAGER

static const struct gpio_dt_spec vcc_power_pin = GPIO_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), gpios, 0);
static bool usb_powered = true; // Assume USB powered at start
static bool vcc_enabled = true;

static void set_vcc_power(bool enable) {
    if (!device_is_ready(vcc_power_pin.port)) {
        LOG_ERR("VCC power control GPIO not ready");
        return;
    }
    
    if (enable && !vcc_enabled) {
        LOG_INF("Enabling VCC power (USB connected)");
        // P0.13 HIGH enables VCC power
        gpio_pin_set_dt(&vcc_power_pin, 1);
        vcc_enabled = true;
        k_msleep(100); // Give components time to power up
    } else if (!enable && vcc_enabled) {
        LOG_INF("Disabling VCC power (battery mode)");
        // P0.13 LOW disables VCC power  
        gpio_pin_set_dt(&vcc_power_pin, 0);
        vcc_enabled = false;
    }
}

static void usb_status_cb(enum usb_dc_status_code status, const uint8_t *param) {
    switch (status) {
        case USB_DC_CONNECTED:
        case USB_DC_CONFIGURED:
            if (!usb_powered) {
                LOG_DBG("USB connected");
                usb_powered = true;
                set_vcc_power(true);
            }
            break;
            
        case USB_DC_DISCONNECTED:
        case USB_DC_SUSPEND:
            if (usb_powered) {
                LOG_DBG("USB disconnected");
                usb_powered = false;
                set_vcc_power(false);
            }
            break;
            
        default:
            break;
    }
}

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
    gpio_pin_set_dt(&vcc_power_pin, 1);
    vcc_enabled = true;
    
    // Register USB status callback
    usb_register_status_callback(usb_status_cb);
    
    return 0;
}

SYS_INIT(vcc_power_manager_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

#endif /* CONFIG_ZMK_VCC_POWER_MANAGER */
