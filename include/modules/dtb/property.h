#ifndef _SHAMPOOS_PROPERTY_H_
#define _SHAMPOOS_PROPERTY_H_
#include <common/types.h>
enum property_value_type {
	PROPERTY_VALUE_EMPTY,
	PROPERTY_VALUE_U32,
	PROPERTY_VALUE_U64,
	PROPERTY_VALUE_STRING,
	PROPERTY_VALUE_PROP_ENCODED_ARRAY,
	PROPERTY_VALUE_PHANDLE,
	PROPERTY_VALUE_STRINGLIST,
};
enum property_type_enum {
	PROPERTY_TYPE_NONE,
	PROPERTY_TYPE_COMPATIBLE,
	PROPERTY_TYPE_MODEL,
	PROPERTY_TYPE_PHANDLE,
	PROPERTY_TYPE_STATUS,
	PROPERTY_TYPE_ADDRESS_CELLS,
	PROPERTY_TYPE_SIZE_CELLS,
	PROPERTY_TYPE_REG,
	PROPERTY_TYPE_VIRTUAL_REG,
	PROPERTY_TYPE_RANGES,
	PROPERTY_TYPE_DMA_RANGES,
	PROPERTY_TYPE_DMA_COHERENT,
	PROPERTY_TYPE_DMA_NONCOHERENT,
	PROPERTY_TYPE_NAME,
	PROPERTY_TYPE_DEVICE_TYPE,
	PROPERTY_TYPE_INTERRUPT,
	PROPERTY_TYPE_INTERRUPT_PARENT,
	PROPERTY_TYPE_INTERRUPTS_EXTENDED,
	PROPERTY_TYPE_INTERRUPT_CELLS,
	PROPERTY_TYPE_INTERRUPT_CONTROLLER,
	PROPERTY_TYPE_INTERRUPT_MAP,
	PROPERTY_TYPE_INTERRUPT_MAP_MASK,
	PROPERTY_TYPE_SPECIFIER_MAP,
	PROPERTY_TYPE_SPECIFIER_MAP_MASK,
	PROPERTY_TYPE_SPECIFIER_MAP_PASS_THRU,
	PROPERTY_TYPE_SPECIFIER_CELLS,
	PROPERTY_TYPE_OTHER,
	PROPERTY_TYPE_NUM
};
struct property_type {
	char property_string[32];
	int property_type_enum;
	int value_enum[2];
};
enum property_type_enum get_property_type(const char *property_name);
#endif