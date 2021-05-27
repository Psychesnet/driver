#ifndef _CDEV_IOCTL_H
#define _CDEV_IOCTL_H

#include <linux/ioctl.h>

struct ioctl_cmd {
    unsigned int reg;
    unsigned int offset;
    unsigned int val;
};

#define IOC_MAGIC 'd'

#define IOCTL_SET_VAL _IOW(IOC_MAGIC, 1, struct ioctl_cmd)
#define IOCTL_GET_VAL _IOR(IOC_MAGIC, 2, struct ioctl_cmd)

#endif /* ifndef _CDEV_IOCTL_H */
