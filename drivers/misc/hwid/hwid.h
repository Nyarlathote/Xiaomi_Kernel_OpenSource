@@ -0,0 +1,201 @@
// SPDX-License-Identifier: GPL-2.0
/*
 * HwId Module driver for mi driver acquire some hwid build info,
 * which is only used for xiaomi corporation internally.
 *
 * Copyright (C) 2020-2021 XiaoMi, Inc.
 * Copyright (C) 2022 The LineageOS Project
 */

/*****************************************************************************
* Included header files
*****************************************************************************/
#include <linux/export.h>
#include <linux/sysfs.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/kdev_t.h>
#include <linux/printk.h>
#include <linux/errno.h>
#include <linux/err.h>

#include "hwid.h"

/*****************************************************************************
* Global variable or extern global variabls
*****************************************************************************/
static uint hwid_value;
module_param(hwid_value, uint, 0444);
MODULE_PARM_DESC(hwid_value, "xiaomi hwid value correspondingly different build");

static uint project;
module_param(project, uint, 0444);
MODULE_PARM_DESC(project, "xiaomi project serial num predefine");

static uint build_adc;
module_param(build_adc, uint, 0444);
MODULE_PARM_DESC(build_adc, "xiaomi adc value of build resistance");

static uint project_adc;
module_param(project_adc, uint, 0444);
MODULE_PARM_DESC(project_adc, "xiaomi adc value of project resistance");

static struct kobject *hwid_kobj;
#define hwid_attr(_name) \
static struct kobj_attribute _name##_attr = {	\
	.attr	= {				\
		.name = __stringify(_name),	\
		.mode = 0444,			\
	},					\
	.show	= _name##_show,			\
	.store	= NULL,				\
}

/*****************************************************************************
* Global variable or extern global functions
*****************************************************************************/
static ssize_t hwid_project_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "0x%x\n", project);
}

static ssize_t hwid_value_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "0x%x\n", hwid_value);
}

static ssize_t hwid_project_adc_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", project_adc);
}

static ssize_t hwid_build_adc_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", build_adc);
}

const char *product_name_get(void)
{
	switch (project){
		case HARDWARE_PROJECT_L2: return "zeus";
		case HARDWARE_PROJECT_L3:  return "cupid";
		default: return "unknown";
	}
}
EXPORT_SYMBOL(product_name_get);

uint32_t get_hw_project_adc(void)
{
	return project_adc;
}
EXPORT_SYMBOL(get_hw_project_adc);

uint32_t get_hw_build_adc(void)
{
	return build_adc;
}
EXPORT_SYMBOL(get_hw_build_adc);

uint32_t get_hw_version_platform(void)
{
	return project;
}
EXPORT_SYMBOL(get_hw_version_platform);


uint32_t get_hw_id_value(void)
{
	return hwid_value;
}
EXPORT_SYMBOL(get_hw_id_value);

uint32_t get_hw_country_version(void)
{
	return (hwid_value & HW_COUNTRY_VERSION_MASK) >> HW_COUNTRY_VERSION_SHIFT;
}
EXPORT_SYMBOL(get_hw_country_version);

uint32_t get_hw_version_major(void)
{
	return (hwid_value & HW_MAJOR_VERSION_MASK) >> HW_MAJOR_VERSION_SHIFT;
}
EXPORT_SYMBOL(get_hw_version_major);

uint32_t get_hw_version_minor(void)
{
	return (hwid_value & HW_MINOR_VERSION_MASK) >> HW_MINOR_VERSION_SHIFT;
}
EXPORT_SYMBOL(get_hw_version_minor);

uint32_t get_hw_version_build(void)
{
	return (hwid_value & HW_BUILD_VERSION_MASK) >> HW_BUILD_VERSION_SHIFT;
}
EXPORT_SYMBOL(get_hw_version_build);

hwid_attr(hwid_project);
hwid_attr(hwid_value);
hwid_attr(hwid_project_adc);
hwid_attr(hwid_build_adc);

static struct attribute *hwid_attrs[] = {
	&hwid_project_attr.attr,
	&hwid_value_attr.attr,
	&hwid_project_adc_attr.attr,
	&hwid_build_adc_attr.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = hwid_attrs,
};

/*****************************************************************************
*  Name: hwid_module_init
*****************************************************************************/
static int __init hwid_module_init(void)
{
	int ret = -ENOMEM;

	hwid_kobj = kobject_create_and_add("hwid", NULL);
	if (!hwid_kobj) {
		pr_err("hwid: hwid module init failed\n");
		goto fail;
	}

	ret = sysfs_create_group(hwid_kobj, &attr_group);
	if (ret) {
		pr_err("hwid: sysfs register failed\n");
		goto sys_fail;
	}

sys_fail:
	kobject_del(hwid_kobj);
fail:
	return ret;
}

/*****************************************************************************
*  Name: hwid_module_exit
*****************************************************************************/
static void __exit hwid_module_exit(void)
{
	if (hwid_kobj) {
		sysfs_remove_group(hwid_kobj, &attr_group);
		kobject_del(hwid_kobj);
	}
	pr_info("hwid: hwid module exit success\n");
}

subsys_initcall(hwid_module_init);
module_exit(hwid_module_exit);

MODULE_AUTHOR("weixiaotian1@xiaomi.com");
MODULE_DESCRIPTION("Hwid Module Driver for Xiaomi Corporation");
MODULE_LICENSE("GPL v2");
 79 changes: 79 additions & 0 deletions79  
drivers/misc/hwid/hwid.h
@@ -0,0 +1,79 @@
#ifndef __HWID_H__
#define __HWID_H__

#define MAX_PRODUCT_SIZE            16
#define MAX_REVIRSION_SIZE          8
#define MAX_COUNTRY_SIZE            8

#define HARDWARE_PROJECT_UNKNOWN    0
#define HARDWARE_PROJECT_L2         1
#define HARDWARE_PROJECT_L3         2

#define HW_MAJOR_VERSION_SHIFT      16
#define HW_MINOR_VERSION_SHIFT      0
#define HW_COUNTRY_VERSION_SHIFT    20
#define HW_BUILD_VERSION_SHIFT      16
#define HW_MAJOR_VERSION_MASK       0xFFFF0000
#define HW_MINOR_VERSION_MASK       0x0000FFFF
#define HW_COUNTRY_VERSION_MASK     0xFFF00000
#define HW_BUILD_VERSION_MASK       0x000F0000

#define SMEM_ID_VENDOR1	135
#define	ADCDEV_MAJOR	0
#define	ADCDEV_MINOR	200
#define	XIAOMI_ADC_MODULE	"xiaomi_adc_module"
#define	XIAOMI_ADC_DEVICE	"xiaomi_adc_device"
#define	XIAOMI_ADC_CLASS	"xiaomi_adc_class"

typedef enum {
	CountryCN = 0x00,
	CountryGlobal = 0x01,
	CountryIndia = 0x02,
	CountryJapan = 0x03,
	INVALID = 0x04,
	CountryIDMax = 0x7FFFFFFF
} CountryType;

typedef enum {
	CHIPINFO_ID_UNKNOWN = 0x00,
	CHIPINFO_ID_MAX = 0x7FFFFFFF
}ChipInfoIdType;

typedef enum {
	PROJECT_ID_UNKNOWN = 0x00,
	PROJECT_ID_MAX = 0x7FFFFFFF
}ProjectInfoType;

/**
  Stores the target project and the hw version.
 */
struct project_info
{
	ChipInfoIdType chiptype;                        /* Chip identification type */
	uint32_t pro_r1;                                /* resistance of the project*/
	uint32_t pr_min_adc;                            /* min adc value from project resistance*/
	uint32_t pr_max_adc;                            /* max adc value from project resistance*/
	ProjectInfoType project;                        /* Project type of the mi predefine*/
	char productname[MAX_PRODUCT_SIZE];             /* product name*/
	uint32_t hw_r1;                                 /* resistance of the hwid*/
	uint32_t hr_min_adc;                            /* min adc value from hwid resistance*/
	uint32_t hr_max_adc;                            /* max adc value from hwid resistance*/
	char hw_level[MAX_REVIRSION_SIZE];              /* hardware reversion*/
	char hw_country[MAX_COUNTRY_SIZE];              /* hardware country*/
	uint32_t hw_id;                                 /* hardware id*/
	uint8_t  ddr_id;                                /* ddr id */
	uint32_t reserved1;                             /* reserved field1 for further*/
	uint32_t reserved2;                             /* reserved field2 for further*/
};

const char *product_name_get(void);
uint32_t get_hw_version_platform(void);
uint32_t get_hw_country_version(void);
uint32_t get_hw_version_major(void);
uint32_t get_hw_version_minor(void);
uint32_t get_hw_version_build(void);
uint32_t get_hw_project_adc(void);
uint32_t get_hw_build_adc(void);
uint32_t get_hw_id_value(void);

#endif
