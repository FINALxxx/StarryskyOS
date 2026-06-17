#include "fcc.h"
#include "stdio.h"
#include "string.h"
#include "generated/autoconf.h"

#ifdef CONFIG_COMPONENT_FLASH_FS
#include "ff.h"
#endif

#include "shell.h"

/* Parser API (lexer API is in fcc.h) */
int fcc_compile(Compiler *comp);

/* ===== Source buffer ===== */
static char src_buf[FCC_SRC_MAX];

/* ===== Singleton compiler ===== */
static Compiler compiler;

/* ===== Path construction (same pattern as editor) ===== */

static int fcc_make_path(char *fullpath, size_t size, const char *user_path) {
#ifdef CONFIG_COMPONENT_FLASH_FS
    Shell *shell = shellGetCurrent();
    const char *cwd = "/";

    if (shell != NULL) {
        cwd = shellGetPath(shell);
        if (cwd == NULL) cwd = "/";
    }

    if (user_path[0] == '/') {
        snprintf(fullpath, size, "0:%s", user_path);
    } else {
        size_t cwd_len = strlen(cwd);
        if (cwd_len > 0 && cwd[cwd_len - 1] == '/') {
            snprintf(fullpath, size, "0:%s%s", cwd, user_path);
        } else {
            snprintf(fullpath, size, "0:%s/%s", cwd, user_path);
        }
    }
    return 0;
#else
    return -1;
#endif
}

/* ===== File loading ===== */

static int fcc_load_file(const char *path) {
#ifdef CONFIG_COMPONENT_FLASH_FS
    char fullpath[512];
    fcc_make_path(fullpath, sizeof(fullpath), path);

    FIL file;
    FRESULT res = f_open(&file, fullpath, FA_OPEN_EXISTING | FA_READ);
    if (res != FR_OK) {
        printf("fcc: cannot open '%s' (code %d)\r\n", path, res);
        return -1;
    }

    if (f_size(&file) > FCC_SRC_MAX) {
        printf("fcc: source file too large (max %d bytes)\r\n", FCC_SRC_MAX);
        f_close(&file);
        return -1;
    }

    UINT bytes_read;
    res = f_read(&file, src_buf, FCC_SRC_MAX - 1, &bytes_read);
    f_close(&file);

    if (res != FR_OK) {
        printf("fcc: read error (code %d)\r\n", res);
        return -1;
    }

    src_buf[bytes_read] = '\0';
    return 0;
#else
    printf("fcc: filesystem not available\r\n");
    return -1;
#endif
}

/* ===== Shell command ===== */

int fccCmd(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: fcc <source.c>\r\n");
        return -1;
    }

    /* Load source */
    if (fcc_load_file(argv[1]) != 0) {
        return -1;
    }

    /* Initialize compiler */
    memset(&compiler, 0, sizeof(compiler));
    lexer_init(&compiler.lexer, src_buf);

    /* Compile */
    int result = fcc_compile(&compiler);
    if (result != 0) {
        printf("fcc: compilation failed\r\n");
        return -1;
    }

    /* Success — print code location */
    printf("fcc: OK  code=0x%08X  size=%u bytes\r\n",
           FCC_CODE_BASE, compiler.code_size);
    printf("     run: exec 0x%08X\r\n", FCC_CODE_BASE);

    return 0;
}
