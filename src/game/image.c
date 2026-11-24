/*
 * GAME.EXE: VRAM image streaming (WIP module). func_80018c60 queues an
 * archive entry as an image request; func_80018cd0 runs once a frame,
 * uploads what the last 16-sector part holds and asks for the next part,
 * or restarts the read when a part's RECT header is damaged. It sees the CD
 * state without the owning module's volatile counter (game/cd.h).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/cd.h>
#include <psyq/kernel.h>
#include <game/cd.h>
#include <game/image.h>

/* One part: 16 sectors. */
#define IMAGE_PART_SECTORS 16
#define IMAGE_PART_BYTES 0x8000

/* RECT.x of the header that ends the image. */
#define IMAGE_END 0xffff

void func_80018c54(CdRequest *request);
void func_80018c94(CdRequest *request);

ADDRESS(0x80018c54, 0xc)
void func_80018c54(CdRequest *request)
{
    request->state = 1;
}

ADDRESS(0x80018c60, 0x34)
void func_80018c60(file, entry)
u16 file;
u16 entry;
{
    func_80019e48(file, entry, DAT_801c17a4, func_80018c54);
}

ADDRESS(0x80018c94, 0x3c)
void func_80018c94(CdRequest *request)
{
    if (request->u.image.remaining > IMAGE_PART_SECTORS) {
        request->count = IMAGE_PART_SECTORS;
        request->u.image.remaining -= IMAGE_PART_SECTORS;
    } else {
        request->count = request->u.image.remaining;
        request->u.image.remaining = 0;
    }
}

/*
 * Not exact: retail loads the RECT height into a temporary that the multiply
 * uses and copies it to the saved height; GCC here loads the saved height
 * directly, so the bytes and rows registers differ.
 */
ADDRESS(0x80018cd0, 0x28c)
void func_80018cd0(void)
{
    CdRequest *request;
    u8 *data;
    u16 *header;
    s32 done;
    s32 left;
    s32 height;
    s32 bytes;
    s16 rows;
    CdlLOC *pos;
    RECT unused; /* the frame keeps 8 unused bytes here */

    EnterCriticalSection();
    request = DAT_801c12e4.read;
    if (request->kind == CD_REQUEST_IMAGE && request->state == 1) {
        done = 0;
        data = request->buffer;
        request->state = 0;
        ExitCriticalSection();
        for (;;) {
            left = IMAGE_PART_BYTES - done;
            if (request->u.image.rect.h != 0) {
                height = request->u.image.rect.h;
                bytes = request->u.image.rect.h * request->u.image.rect.w * 2;
                if (left < bytes) {
                    rows = (left / request->u.image.rect.w) >> 1;
                    if (rows != 0) {
                        request->u.image.rect.h = rows;
                        LoadImage(&request->u.image.rect, (u_long *)data);
                        request->u.image.rect.h = height - rows;
                        request->u.image.rect.y += rows;
                    }
                    break;
                }
                request->u.image.rect.h = height;
                LoadImage(&request->u.image.rect, (u_long *)data);
                done += bytes;
                data += bytes;
                request->u.image.rect.h = 0;
                continue;
            }
            if ((u32)left < 16) {
                break;
            }
            header = (u16 *)data;
            if (header[0] != header[4] || header[1] != header[5] || header[2] != header[6]
                || header[3] != header[7] || header[2] == 0 || header[3] == 0) {
                request->u.image.rect.h = 0;
                request->state = 0;
                request->u.image.remaining = request->u.image.sectors;
                request->pos = request->retry_pos;
                func_80018c94(request);
                request->issued = 0;
                CdSeekL(&request->pos);
                return;
            }
            done += 16;
            if (header[0] == IMAGE_END) {
                DrawSync(0);
                request->count = 0;
                func_8001967c(request);
                return;
            }
            request->u.image.rect.x = header[0];
            request->u.image.rect.y = header[1];
            request->u.image.rect.w = header[2];
            request->u.image.rect.h = header[3];
            data += 16;
        }
        pos = &request->pos;
        func_80019a74(pos, request->count, pos);
        func_80018c94(request);
        request->issued = 0;
        DrawSync(0);
        CdSeekL(pos);
    } else {
        ExitCriticalSection();
    }
}
