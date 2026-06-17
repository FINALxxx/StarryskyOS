#ifndef __SHELL_INIT_H__
#define __SHELL_INIT_H__

#include "shell.h"
#include "shell_fs.h"
#include "hal_sys_uart.h"
// #include "hal_hp_uart.h"
#include "shell_port.h"
#include "generated/autoconf.h"

#ifdef CONFIG_COMPONENT_FLASH_FS
#include "ffinit.h"
#endif

void load_shell();

#endif
