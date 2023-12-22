#include <pd_api.h>
#include "../lib/stb-playdate-fixes/stb_image.h"

#define MARGIN 10
const char instructions[] = "Press left to load gif, up to load jpg,\nor right to load png.";

// Using '.gif' or '.png' extension will compile to a '.pdi'
const char file_path_gif[] = "example.gif_";
const char file_path_jpg[] = "example.jpg";
const char file_path_png[] = "example.png_";

// Stores 8-bit image data in a Playdate bitmap. Can act as a threshold filter.
LCDBitmap *pack_bitmap(PlaydateAPI *pd, const unsigned char *input, int w, int h)
{
    LCDBitmap *bitmap = pd->graphics->newBitmap(w, h, kColorWhite);
    int rowbytes;
    uint8_t *output;
    pd->graphics->getBitmapData(bitmap, NULL, NULL, &rowbytes, NULL, &output);

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            // Set pixel black if value is half or less
            if (input[y * w + x] < 128)
            {
                output[y * rowbytes + (x / 8)] &= ~(1 << (7 - (x % 8)));
            }
        }
    }

    return bitmap;
}

int button_loop(void *userdata)
{
    PlaydateAPI *playdate = userdata;

    PDButtons released;
    const char *file_path = NULL;
    playdate->system->getButtonState(NULL, &released, NULL);
    // Determine file path from button pressed
    if (released & kButtonLeft)
    {
        file_path = file_path_gif;
    }
    else if (released & kButtonUp)
    {
        file_path = file_path_jpg;
    }
    else if (released & kButtonRight)
    {
        file_path = file_path_png;
    }
    else
    {
        return 0;
    }

    // Load file from memory
    // Clear screen, start time
    playdate->graphics->clear(kColorWhite);
    playdate->system->resetElapsedTime();

    // Get file size and other info
    FileStat file_stat;
    if (playdate->file->stat(file_path, &file_stat))
    {
        playdate->system->error("%s", playdate->file->geterr());
        return 0;
    }

    // Open file
    SDFile *file = playdate->file->open(file_path, kFileRead);
    if (file == NULL)
    {
        playdate->system->error("%s", playdate->file->geterr());
        return 0;
    }

    // Allocate buffer for file
    unsigned char *file_buffer = playdate->system->realloc(NULL, file_stat.size);
    if (file_buffer == NULL)
    {
        playdate->system->error("File buffer allocation failed");
        return 0;
    }

    // Read file into buffer
    int bytes_read = playdate->file->read(file, file_buffer, file_stat.size);
    if (bytes_read != file_stat.size)
    {
        playdate->system->error("File read expected %i bytes, got %i instead", file_stat.size, bytes_read);
        return 0;
    }

    // Close file
    playdate->file->close(file);

    // Open image
    int x, y, channels;
    unsigned char *image = stbi_load_from_memory(file_buffer, bytes_read, &x, &y, &channels, 1);
    if (image == NULL)
    {
        playdate->system->error("Error reading image data: %s", stbi_failure_reason());
        return 0;
    }

    // Convert to Playdate bitmap
    LCDBitmap *bitmap = pack_bitmap(playdate, image, x, y);

    // Free image
    stbi_image_free(image);

    // Free buffer
    playdate->system->realloc(file_buffer, 0);

    // Get time
    float elapsed_time = playdate->system->getElapsedTime();

    // Draw bitmap to screen
    playdate->graphics->drawBitmap(bitmap, 0, 0, kBitmapUnflipped);

    // Free bitmap
    playdate->graphics->freeBitmap(bitmap);

    // Draw time to screen
    char *time;
    int time_len = playdate->system->formatString(&time, "Took %f seconds.", (double) elapsed_time);
    playdate->graphics->pushContext(NULL);
    playdate->graphics->setDrawMode(kDrawModeFillWhite);
    playdate->graphics->drawText(time, time_len, kUTF8Encoding, MARGIN, MARGIN);
    playdate->graphics->popContext();
    playdate->system->realloc(time, 0);

    return 1;
}

// Does any initialization tasks that must run during the update loop.
int init_loop(void *userdata)
{
    PlaydateAPI *playdate = userdata;

    playdate->graphics->drawText(instructions, strlen(instructions), kUTF8Encoding, MARGIN, MARGIN);
    playdate->system->setUpdateCallback(button_loop, playdate);
    return 1;
}

#ifdef _WINDLL
__declspec(dllexport)
#endif

int eventHandler(PlaydateAPI *playdate, PDSystemEvent event, uint32_t arg)
{
    if (event == kEventInit)
    {
        playdate->system->setUpdateCallback(init_loop, playdate);
    }

    return 0;
}
