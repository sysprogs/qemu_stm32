#include "hw/sysbus.h"
#include "hw/arm/stm32.h"
#include "sysemu/char.h"
#include "qemu/bitops.h"

typedef struct STM32DWTState
{
    SysBusDevice busdev;
    stm32_periph_t periph;
    MemoryRegion iomem;

	uint64_t IcountBase;
} STM32DWTState;

#define TYPE_STM32_DWT "stm32-dwt"
#define STM32_DWT(obj) OBJECT_CHECK(STM32DWTState, (obj), TYPE_STM32_DWT)

enum { ICOUNT_DIVIDER = 13 };

static uint64_t stm32_dwt_read(void *opaque, hwaddr offset, unsigned size)
{
  STM32DWTState *s = (STM32DWTState *)opaque;
  return cpu_get_icount() / ICOUNT_DIVIDER - s->IcountBase;
}

static void stm32_dwt_write(void *opaque, hwaddr offset,
                       uint64_t value, unsigned size)
{ 
    STM32DWTState *s = (STM32DWTState *)opaque;
	s->IcountBase = cpu_get_icount() / ICOUNT_DIVIDER - value;
}

static Property stm32_dac_properties[] = {
    DEFINE_PROP_PERIPH_T("periph", STM32DWTState, periph, STM32_PERIPH_UNDEFINED),
    DEFINE_PROP_END_OF_LIST()
};

static const MemoryRegionOps stm32_dac_ops = {
    .read = stm32_dwt_read,
    .write = stm32_dwt_write,
    .valid.min_access_size = 2,
    .valid.max_access_size = 4,
    .endianness = DEVICE_NATIVE_ENDIAN
};

static int stm32_dwt_init(SysBusDevice *dev)
{
    STM32DWTState *s = STM32_DWT(dev);
    memory_region_init_io(&s->iomem, OBJECT(s), &stm32_dac_ops, s,
                          "stm32-dwt", 0x4);
    sysbus_init_mmio(dev, &s->iomem);

    return 0;
}

static void stm32_dwt_class_init(ObjectClass *oc, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(oc);
    SysBusDeviceClass *k = SYS_BUS_DEVICE_CLASS(oc);

    k->init = stm32_dwt_init;
	dc->props = stm32_dac_properties;
}

static const TypeInfo stm32_dwt_type_info = {
    .name = TYPE_STM32_DWT,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(struct STM32DWTState),
    .class_init = stm32_dwt_class_init,
};

static void stm32_dwt_register_types(void)
{
    type_register_static(&stm32_dwt_type_info);
}

type_init(stm32_dwt_register_types)
