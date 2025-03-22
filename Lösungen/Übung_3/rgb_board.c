#include <linux/i2c.h>
#include <linux/kobject.h>

static struct kobject *my_kobj;
static struct i2c_client *my_client;

static ssize_t my_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
	if ( count < 3)
		return -EINVAL;

	u8 val = 0xff;

	val &= ~((buffer[0] - '0') << 1);
	val &= ~((buffer[1] - '0') << 2);
	val &= ~((buffer[2] - '0') << 3);

	i2c_smbus_write_byte(my_client, val);
	return count;
}

static ssize_t my_show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
	u8 val;
	val = i2c_smbus_read_byte(my_client);

	return sprintf(buffer, "Taster ist %sgedrückt\n", val & 1 ? "nicht " : "");
}

static struct kobj_attribute my_attr_led = __ATTR(led, 0220, NULL, my_store);
static struct kobj_attribute my_attr_button = __ATTR(taster, 0440, my_show, NULL);

/* Benenne alle kompatiblen Geräte */
static struct i2c_device_id my_ids[] = {
	{"rgb_brd"},
	{} /* leeres Element signalisiert das Ende der Liste */
};

MODULE_DEVICE_TABLE(i2c, my_ids);

/* Funktion wird aufgerufen, wenn ein kompatibles I2C Gerät hinzugefügt wird */
static int my_probe(struct i2c_client *client /*, const struct i2c_device_id *id*/)
{
	int status;
	my_client = client;

	printk("Hallo vom I2C Slave mit der Adresse: 0x%x\n", client->addr);

	my_kobj = kobject_create_and_add("rgb_led", my_kobj);
	if (!my_kobj) {
		printk("Fehler beim Anlegen des Kernel Objects\n");
		return -ENOMEM;
	}

	status = sysfs_create_file(my_kobj, &my_attr_led.attr);
	if (status) {
		printk("Fehler beim Anlegen von /sys/rgb_led/led\n");
		kobject_put(my_kobj);
		return status;
	}

	status = sysfs_create_file(my_kobj, &my_attr_button.attr);
	if (status) {
		printk("Fehler beim Anlegen von /sys/rgb_led/led\n");
		sysfs_remove_file(my_kobj, &my_attr_led.attr);
		kobject_put(my_kobj);
		return status;
	}
	printk("Hallo vom I2C Slave mit der Adresse: 0x%x\n", client->addr);

	return 0;
}

/* Funktion wird aufgerufen, wenn ein kompatibles I2C Gerät entfernt wird */
static void my_remove(struct i2c_client *client)
{
	i2c_smbus_write_byte(client, 0xff);
	sysfs_remove_file(my_kobj, &my_attr_led.attr);
	sysfs_remove_file(my_kobj, &my_attr_button.attr);
	kobject_put(my_kobj);
	printk("Bye, bye, I2C\n");
}

/* Fasse kompatible Geräte, Probe- und Remove-Funktionen in Treiber zusammen */
static struct i2c_driver my_driver = {
	.probe = my_probe,
	.remove = my_remove,
	.id_table = my_ids,
	.driver = {
		.name = "my-i2c-driver",
	}
};

/* Registriere Treiber */
module_i2c_driver(my_driver);

/* Informationen über Treiber */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes Roith");
MODULE_DESCRIPTION("Ein Hello World I2C Teiber");

