#include <graphics/drawing.h>
#include <graphics/color.h>
#include <graphics/text.h>
#include <graphics/init.h>
#include <graphics/lcdc.h>
#include <sh4a/input/keypad.h>
#include <stdio.h>
#include <string.h>
#include "libc/memmgr.h"

#define SW 528
#define SH 320

extern const unsigned char save_payload[];
extern const unsigned int save_payload_len;
extern const char save_target_path[];
extern const char save_backup_path[];
extern const char save_target_name[];

static void line(int y, const char *text, unsigned short color)
{
    set_pen(color);
    render_text(12, y, (char *)text);
}

static void clear_screen(void)
{
    set_pen(create_rgb16(0, 0, 0));
    draw_rect(0, 0, SW, SH);
}

static void release_key(int key)
{
    while (get_key_state(key))
        keypad_read();
}

static void draw_confirm(void)
{
    char detail[96];
    clear_screen();
    line(18, "Gnuboy Save Importer", create_rgb16(0, 255, 255));
    line(60, "Target save:", create_rgb16(255, 255, 0));
    line(88, save_target_name, create_rgb16(255, 255, 255));
    sprintf(detail, "Payload: %u bytes", save_payload_len);
    line(126, detail, create_rgb16(255, 255, 255));
    line(174, "Existing save will be backed up.", create_rgb16(180, 180, 180));
    line(242, "ENTER: write   BACK: cancel", create_rgb16(0, 255, 0));
    lcdc_copy_vram();
}

static void draw_result(const char *status, const char *detail,
                        unsigned short color)
{
    clear_screen();
    line(18, "Gnuboy Save Importer", create_rgb16(0, 255, 255));
    line(76, status, color);
    line(116, detail, create_rgb16(255, 255, 255));
    line(270, "BACK: exit", create_rgb16(0, 255, 0));
    lcdc_copy_vram();
}

static int backup_current_save(void)
{
    unsigned char buffer[256];
    unsigned int count;
    FILE *src = fopen(save_target_path, "rb");
    FILE *dst;

    if (!src)
        return 0; /* A new save needs no backup. */
    dst = fopen(save_backup_path, "wb");
    if (!dst) {
        fclose(src);
        return -1;
    }
    while ((count = fread(buffer, 1, sizeof(buffer), src)) != 0) {
        if (fwrite(buffer, 1, count, dst) != count) {
            fclose(src);
            fclose(dst);
            return -1;
        }
    }
    fclose(src);
    return fclose(dst) == 0 ? 1 : -1;
}

static int write_and_verify(void)
{
    unsigned char verify[256];
    unsigned int offset;
    FILE *f;
    int backup = backup_current_save();

    if (backup < 0)
        return 1;
    f = fopen(save_target_path, "wb");
    if (!f)
        return 2;
    if (fwrite(save_payload, 1, save_payload_len, f) != save_payload_len) {
        fclose(f);
        return 3;
    }
    if (fclose(f) != 0)
        return 4;

    f = fopen(save_target_path, "rb");
    if (!f)
        return 5;
    for (offset = 0; offset < save_payload_len; offset += sizeof(verify)) {
        unsigned int remaining = save_payload_len - offset;
        unsigned int chunk = remaining < sizeof(verify) ? remaining : sizeof(verify);
        if (fread(verify, 1, chunk, f) != chunk ||
            memcmp(verify, save_payload + offset, chunk) != 0) {
            fclose(f);
            return 6;
        }
    }
    fclose(f);
    return 0;
}

int main(void *ptr)
{
    int rc;
    char detail[96];
    if (ptr && *(long *)ptr == 1)
        return -1;
    memmgr_init();
    graphics_init(SW, SH, (void *)0xAC200000);
    draw_confirm();
    for (;;) {
        keypad_read();
        if (get_key_state(KEY_POWER) || get_key_state(KEY_BACK))
            return -2;
        if (get_key_state(KEY_ENTER)) {
            release_key(KEY_ENTER);
            draw_result("Writing save data...", "Do not power off.",
                        create_rgb16(255, 255, 0));
            rc = write_and_verify();
            if (rc == 0) {
                sprintf(detail, "%u bytes verified.", save_payload_len);
                draw_result("SAVE IMPORT: SUCCESS", detail,
                            create_rgb16(0, 255, 0));
            } else {
                sprintf(detail, "Error code: %d", rc);
                draw_result("SAVE IMPORT: FAILED", detail,
                            create_rgb16(255, 0, 0));
            }
        }
    }
}
