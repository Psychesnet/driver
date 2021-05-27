TARGETS := test
SRCS := test.cpp

.PHONY: all install
all: $(TARGETS)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< | sed -re 's,^.*:,$*.o $*.d:,g' > $@
%.d: %.cpp
	$(CXX) -MM $(CFLAGS) $< | sed -re 's,^.*:,$*.o $*.d:,g' > $@
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
%.o: %.cpp
	$(CXX) -c $(CFLAGS) $< -o $@

DEPENDS := $(SRCS:%.c=%.d)
DEPENDS := $(DEPENDS:%.cpp=%.d)
ifneq ($(MAKECMDGOALS),clean)
-include $(DEPENDS)
endif

audiod: $(SRCS:%.cpp=%.o)
	$(CXX) $^ $(LDFLAGS) -o $@

install: $(TARGETS)
	$(INSTALL_DIR) $(DES_DIR)/usr/bin
	$(foreach file,$(TARGETS),\
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(file) $(DES_DIR)/usr/bin;)
