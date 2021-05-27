# 定義很多環境變數，可從這抓
include $(TOPDIR)/rules.mk

# Name, version and release number
# The name and version of your package are used to define the variable to point to the build directory of your package: $(PKG_BUILD_DIR)
PKG_NAME:=driver-unit-test
PKG_VERSION:=0.1
PKG_RELEASE:=$(PKG_VERSION)

# Source settings (i.e. where to find the source codes)
# This is a custom variable, used below
SOURCE_DIR:=src

# 需要包成ipkg，要include package
include $(INCLUDE_DIR)/package.mk

# Package definition; instructs on how and where our package will appear in the overall configuration menu ('make menuconfig')
# If you use C++, need to add DEPENDS+=libstdcpp
define Package/$(PKG_NAME)
# section沒有用到
  SECTION:=Driver
# 第2層選項的名字
  SUBMENU:=Driver
# 第1層選項的名字
  CATEGORY:=Liteon
# 說明
  TITLE:=Unit test tools for driver
# 記錄所有的相依性，在包ipkg時，需要把對應的lib全包在一起
  DEPENDS:=+libstdcpp
endef

# Package description; a more verbose description on what our package does
define Package/$(PKG_NAME)/description
  Unit test tools for driver
endef

# define external ld flag
PLATFORM_FLAGS:=
PLATFORM_LD_FLAGS:=-lstdc++ -lpthread -lm
BUILD_FLAGS:=
BUILD_OPTIONS:=

# 下make packages/Liteon/examples/app/prepare 做的事 
# Package preparation instructions; create the build directory and copy the source code.
# The last command is necessary to ensure our preparation instructions remain compatible with the patching system.
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	rsync -a $(SOURCE_DIR)/* $(PKG_BUILD_DIR)
	$(Build/Patch)
endef

# 下make packages/Liteon/examples/app/compile 做的事
# Package build instructions; invoke the target-specific compiler to first compile the source file, and then to link the file into the final executable
define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS) -Wall $(PLATFORM_FLAGS) $(BUILD_FLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS) $(PLATFORM_LD_FLAGS)" \
		$(BUILD_OPTIONS) all
endef

# 這會被最後的pack所呼叫，主要是把想要install的東西放到對應的位置
# (1)的位置會是stag_dir/build_dir/ipkg_dir
# Package install instructions; create a directory inside the package to hold our executable, and then copy the executable we built previously into the folder
define Package/$(PKG_NAME)/install
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CP="$(CP)" \
		LN="$(LN)" \
    	INSTALL_BIN="$(INSTALL_BIN)" \
    	INSTALL_DIR="$(INSTALL_DIR)" \
    	INSTALL_DATA="$(INSTALL_DATA)" \
		INSTALL_CONF="$(INSTALL_CONF)" \
    	PKG_BUILD_DIR="$(PKG_BUILD_DIR)" \
		PKG_VERSION="$(PKG_VERSION)" \
		$(BUILD_OPTIONS) \
		DES_DIR="$(1)" install
endef

# This command is always the last, it uses the definitions and variables we give above in order to get the job done
$(eval $(call BuildPackage,$(PKG_NAME)))
