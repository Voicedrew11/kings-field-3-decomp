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
void func_80018c60(int file_in, int entry_in)
{
    u16 file = file_in;
    u16 entry = entry_in;

    func_80019e48(file, entry, DAT_801c17a4, (CdCallback)func_80018c54);
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
 * Uploads the image records streamed into the read request's buffer: each
 * 16-byte header (RECT twice) is followed by its pixels; a record cut by the
 * end of the part is uploaded by whole rows and finished from the next part.
 */
ADDRESS(0x80018cd0, 0x28c)
void func_80018cd0(void)
{
    CdRequest *request;
    u16 *header;
    s32 done;
    CdlLOC *pos;

    EnterCriticalSection();
    request = DAT_801c12e4.read;
    if (request->kind != CD_REQUEST_IMAGE || request->state != 1) {
        goto leave;
    }
    done = 0;
    header = (u16 *)request->buffer;
    request->state = 0;
    ExitCriticalSection();

    for (;;) {
        s32 left = IMAGE_PART_BYTES - done;
        s16 height = request->u.image.rect.h;

        if (height != 0) {
            s32 width = request->u.image.rect.w;
            s32 bytes = (height * width) << 1;

            if (left < bytes) {
                s32 rows = (left / width) >> 1;

                if ((s16)rows != 0) {
                    request->u.image.rect.h = rows;
                    LoadImage(&request->u.image.rect, (u_long *)header);
                    request->u.image.rect.h = height - rows;
                    request->u.image.rect.y = rows + request->u.image.rect.y;
                }
                break;
            }
            request->u.image.rect.h = height;
            LoadImage(&request->u.image.rect, (u_long *)header);
            request->u.image.rect.h = 0;
            done += bytes;
            header = (u16 *)((u8 *)header + bytes);
            continue;
        }

        if ((u32)left < 16) {
            break;
        }
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
        setRECT(&request->u.image.rect, header[0], header[1], header[2], header[3]);
        header += 8;
    }

    pos = &request->pos;
    func_80019a74(pos, request->count, pos);
    func_80018c94(request);
    request->issued = 0;
    DrawSync(0);
    CdSeekL(pos);
    return;

leave:
    ExitCriticalSection();
}
