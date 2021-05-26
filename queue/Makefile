include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=queue
PKG_VERSION:=0.0.1
PKG_RELEASE:=$(PKG_VERSION)

SOURCE_DIR:=src

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/target.mk

define KernelPackage/$(PKG_NAME)
  SECTION:=Driver
  SUBMENU:=Driver
  CATEGORY:=Liteon
  DEPENDS:=
  TITLE:=Queue exmaple via link list at kernel part
  FILES:=$(PKG_BUILD_DIR)/queue.ko
  AUTOLOAD:=$(call AutoProbe,queue)
endef

define KernelPackage/$(PKG_NAME)/description
  Queue exmaple via link list at kernel part
endef

include $(INCLUDE_DIR)/kernel-defaults.mk

export AMBARELLA_DRV_CFLAGS := -I$(PLATFORM_DIR)/$(SUBTARGET)
export AMBARELLA_DRV_AFLAGS :=
export AMBARELLA_DRV_LDFLAGS :=

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	rsync -a $(SOURCE_DIR)/* $(PKG_BUILD_DIR)
endef

MAKEOPTS= -C $(PKG_BUILD_DIR) \
		ARCH="$(LINUX_KARCH)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		KERNEL_SOURCE="$(LINUX_DIR)" \
		SUB_BOARD_DIR="$(PLATFORM_DIR)/$(SUBTARGET)" \
		PKG_BUILD_DIR="$(PKG_BUILD_DIR)" \
		STAGING_DIR="$(STAGING_DIR)" \
		LDOPTS=" "

define Build/Compile
	$(MAKE) $(MAKEOPTS)
endef

define KernelPackage/$(PKG_NAME)/install
endef

$(eval $(call KernelPackage,$(PKG_NAME)))
