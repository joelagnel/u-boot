commit 1ed8e42811a9d5e485a1983c95fad57169a745aa
Author: Joel A Fernandes <agnel.joel@gmail.com>
Date:   Thu Aug 11 23:14:29 2011 -0500

    OMAP3: Add function to get state of a GPIO output
    
    Read directly from OMAP24XX_GPIO_DATAOUT to get the output state of the GPIO pin
    
    Signed-off-by: Joel A Fernandes <agnel.joel@gmail.com>
    Signed-off-by: Jason Kridner <jkridner@beagleboard.org>

diff --git a/arch/arm/cpu/armv7/omap3/gpio.c b/arch/arm/cpu/armv7/omap3/gpio.c
index aeb6066..c8bb551 100644
--- a/arch/arm/cpu/armv7/omap3/gpio.c
+++ b/arch/arm/cpu/armv7/omap3/gpio.c
@@ -160,6 +160,26 @@ int omap_get_gpio_datain(int gpio)
 			& (1 << get_gpio_index(gpio))) != 0;
 }
 
+int omap_get_gpio_dataout(int gpio)
+{
+	struct gpio_bank *bank;
+	void *reg;
+
+	if (check_gpio(gpio) < 0)
+		return -EINVAL;
+	bank = get_gpio_bank(gpio);
+	reg = bank->base;
+	switch (bank->method) {
+	case METHOD_GPIO_24XX:
+		reg += OMAP24XX_GPIO_DATAOUT;
+		break;
+	default:
+		return -EINVAL;
+	}
+	return (__raw_readl(reg)
+			& (1 << get_gpio_index(gpio))) != 0;
+}
+
 static void _reset_gpio(struct gpio_bank *bank, int gpio)
 {
 	_set_gpio_direction(bank, get_gpio_index(gpio), 1);
diff --git a/arch/arm/include/asm/arch-omap3/gpio.h b/arch/arm/include/asm/arch-omap3/gpio.h
index 30f633c..81615bc 100644
--- a/arch/arm/include/asm/arch-omap3/gpio.h
+++ b/arch/arm/include/asm/arch-omap3/gpio.h
@@ -82,5 +82,7 @@ void omap_set_gpio_direction(int gpio, int is_input);
 void omap_set_gpio_dataout(int gpio, int enable);
 /* Get the value of a gpio input */
 int omap_get_gpio_datain(int gpio);
+/* Get the value of a gpio output */
+int omap_get_gpio_dataout(int gpio);
 
 #endif /* _GPIO_H_ */
