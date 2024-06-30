#include <string.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "shell.h"
#include "syslog.h"
#include "io.h"
#include "spi.h"
#include "boot_flash.h"
#include "aiva_malloc.h"

typedef enum {
    MEM_POOL_DEFAULT = 0,
    MEM_POOL_SMALL,
} mem_pool_type_t;

static void cmds_dump_help(void)
{
    Shell *shell = shellGetCurrent();
    shellPrint(shell,   "Usage: mem_info [ARGS] \r\n"
                        "      - mem_info : show memory usage info,  ARGS: [mem_pool_type] \r\n"
                        "                   mem_pool_type: 0-default 1-small_pool\r\n"
               );
}


int mem_info(int argc, char **argv)
{
    mem_pool_type_t mpt = MEM_POOL_DEFAULT;

    Shell *shell = shellGetCurrent();

    
    if (strncmp("mem_info", argv[0], 8) == 0) {
        if (argc != 2) {
            cmds_dump_help();
            return -1;
        }
        int type = strtol(argv[1], NULL, 0);
        if (type == 0 || type == 1) {
            mpt = (mem_pool_type_t)type;
            if (mpt == MEM_POOL_DEFAULT) {
                aiva_default_print_memory_info(&g_norm_mem_pool);
            } else {
                aiva_tlsf_print_memory_info();
            }
        }
        else {
            shellPrint(shell, "Unknown option: %s\n", argv[1]);
            cmds_dump_help();
            return -1;
        }
    }

    return 0;
}


/* _attr, _name, _func, _desc */
SHELL_EXPORT_CMD(
SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
mem_info, mem_info, show memory usage info);
