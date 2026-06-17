// System Headers
#include "board.h"
#include "libgcc.h"
#include "hal_sys_uart.h"
#include "shell_init.h"
#include "shell_port.h"

// System Configuration
#include "generated/autoconf.h"

#define PSRAM_SCKL_FREQ_MHZ (CONFIG_CPU_FREQ_MHZ / CONFIG_PSRAM_NUM)