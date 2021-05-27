#ifndef _CDEV_IOCTL_H
#define _CDEV_IOCTL_H

#include <linux/ioctl.h>
#include <asm/byteorder.h>

struct ioctl_cmd {
    unsigned int reg;
    unsigned int offset;
#ifdef __LP64__
    unsigned char *buf;      /* 64bit linux，long/pointer both are 8bytes */
#else
    /* 32bit linux */
#ifdef __LITTLE_ENDIAN_BITFIELD
    unsigned char *buf;      /* 32bit linux, long/pointer bother are 4bytes */
    unsigned int padding;
#elif __BIG_ENDIAN_BITFIELD
    unsigned int padding;
    unsigned char *buf;
#else
#error "Please fix <asm/byteorder.h>"
#endif

#endif
    unsigned int val;
    unsigned int val_padding;
};

#define IOC_MAGIC 'd'

#define IOCTL_SET_VAL _IOW(IOC_MAGIC, 1, struct ioctl_cmd)
#define IOCTL_GET_VAL _IOR(IOC_MAGIC, 2, struct ioctl_cmd)

#endif /* ifndef _CDEV_IOCTL_H */
