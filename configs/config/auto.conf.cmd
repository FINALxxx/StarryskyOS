deps_config := \
	/home/finalx/.local/ecos-sdk/board/StarrySkyC2/driver.kconfig \
	/home/finalx/.local/ecos-sdk/board/StarrySkyC2/board.kconfig \
	/home/finalx/.local/ecos-sdk/tools/kconfig/Kconfig

include/config/auto.conf: \
	$(deps_config)

ifneq "$(BoardExport)" "/home/finalx/.local/ecos-sdk/board/StarrySkyC2/board.kconfig"
include/config/auto.conf: FORCE
endif
ifneq "$(DriverExport)" "/home/finalx/.local/ecos-sdk/board/StarrySkyC2/driver.kconfig"
include/config/auto.conf: FORCE
endif

$(deps_config): ;
