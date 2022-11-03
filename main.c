#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <memory.h>
#include <string.h>

// Source: http://www.fastgraph.com/help/bmp_header_format.html
#pragma pack(push, 1)
typedef struct
{
    struct BmpHeader
    {
        uint16_t signature;                         // Reserved, must be 0
        uint32_t size;                              // Size of BMP file in bytes
        uint16_t reserved_1;                        // Reserved, must be 0
        uint16_t reserved_2;                        // Reserved, must be 0
        uint32_t offset_to_start_of_image_data;     // Offset to start of image data in bytes
    } bmp_header;

    struct DibHeader
    {
        uint32_t bitmap_info_header;                // Must be 40
        int32_t width;                              // Image width
        int32_t height;                             // Image height
        uint16_t planes;                            // Number of planes in the image, must be 1
        uint16_t bits_per_pixel;                    // Bits per pixel (1, 4, 8 or 24)
        uint32_t compression_type;                  // 0 = None, 1 = RLE-8, 2 = RLE-4
        uint32_t image_data_size;                   // Including padding
        int32_t horizontal_ppm;                     // Horizontal resolution in pixels per meter
        int32_t vertical_ppm;                       // Vertical resolution in pixels per meter
        uint32_t colors_amount;                     // Number of colors in image, or zero
        uint32_t important_colors_amount;           // Number of important colors, or zero
    } dib_header;

    struct ColorHeader
    {
        uint32_t red_mask;                          // Red channel bit mask
        uint32_t green_mask;                        // Green channel bit mask
        uint32_t blue_mask;                         // Blue channel bit mask
        uint32_t alpha_mask;                        // Alpha channel bit mask
        uint32_t color_space_type;                  // Color space type
        uint32_t color_space_endpoints[9];          // Color space endpoints
        uint32_t gamma_red;                         // Gamma for red channel
        uint32_t gamma_green;                       // Gamma for green channel
        uint32_t gamma_blue;                        // Gamma for blue channel
        uint32_t intent;                            // Rendering intent
        uint32_t profile_data;                      // Profile data
        uint32_t profile_size;                      // Profile size
        uint32_t reserved;                          // Reserved
    } color_header;

    uint8_t *data;                                  // Image data
} BmpImage;
#pragma pack(pop, 1)

BmpImage* CreateBMPImage(int width, int height, int has_alpha)
{
    uint16_t bits_per_pixel = 24 + (has_alpha ? 8 : 0);
    uint32_t image_data_size = (width * height) * bits_per_pixel;

    BmpImage* image = (BmpImage*) malloc(sizeof(BmpImage));

    // Bitmap file header
    image->bmp_header.signature = 0x4D42;
    image->bmp_header.size = 54 + image_data_size;
    image->bmp_header.reserved_1 = 0;
    image->bmp_header.reserved_2 = 0;
    image->bmp_header.offset_to_start_of_image_data = sizeof(struct DibHeader) + sizeof(struct ColorHeader);

    // Bitmap v5 header (DIB header)
    image->dib_header.bitmap_info_header = 40;
    image->dib_header.width = width;
    image->dib_header.height = height;
    image->dib_header.planes = 1;
    image->dib_header.bits_per_pixel = bits_per_pixel;
    image->dib_header.compression_type = 0;
    image->dib_header.image_data_size = image_data_size;
    image->dib_header.horizontal_ppm = 0;
    image->dib_header.horizontal_ppm = 0;
    image->dib_header.colors_amount = 0;
    image->dib_header.important_colors_amount = 0;

    // Color header
    image->color_header.red_mask = 0x00FF0000;
    image->color_header.green_mask = 0x0000FF00;
    image->color_header.blue_mask = 0x000000FF;
    image->color_header.alpha_mask = 0xFF000000;
    image->color_header.color_space_type = 0x73524742;
    image->color_header.color_space_endpoints[0] = 0x57696E20;
    image->color_header.color_space_endpoints[1] = 0x4C696E65;
    image->color_header.color_space_endpoints[2] = 0x20726567;
    image->color_header.color_space_endpoints[3] = 0x69737472;
    image->color_header.color_space_endpoints[4] = 0x6174696F;
    image->color_header.color_space_endpoints[5] = 0x6E202863;
    image->color_header.color_space_endpoints[6] = 0x29206973;
    image->color_header.color_space_endpoints[7] = 0x20656E69;
    image->color_header.color_space_endpoints[8] = 0x74726174;
    image->color_header.gamma_red = 0;
    image->color_header.gamma_green = 0;
    image->color_header.gamma_blue = 0;
    image->color_header.intent = 0;
    image->color_header.profile_data = 0;
    image->color_header.profile_size = 0;
    image->color_header.reserved = 0;

    image->data = (uint8_t*) malloc(sizeof(uint8_t) * image_data_size);
    memset(image->data, 0xFF, image_data_size); // Set all pixels to white

    return image;
}

void DestroyBmpImage(BmpImage* image)
{
    if(image != NULL)
    {
        free(image->data);
        free(image);
    }
}

void DrawCircleOnBMPImage(BmpImage* image)
{
    if(image != NULL)
    {
        int32_t width = image->dib_header.width;
        int32_t height = image->dib_header.height;

        int32_t center_x = width / 2;
        int32_t center_y = height / 2;

        int32_t radius = ((min(width, height) - 1) / 2);
        double pi = 3.14159265359f;

        for(double rad = 0; rad < 2 * pi; rad += 0.001)
        {
            int32_t x = (int32_t)(center_x + radius * cos(rad));
            int32_t y = (int32_t)(center_y + radius * sin(rad));

            uint8_t* pixel = image->data + (y * width + x) * 4;
            pixel[3] = 0x00; // B
            pixel[2] = 0x00; // G
            pixel[1] = 0xCC; // R
            pixel[0] = 0xFF; // A
        }
    }
}

void RemoveFile(const char* path)
{
    if(path != NULL)
    {
        FILE* file;
        fopen_s(&file, path, "r");
        if(file != NULL)
        {
            fclose(file);
            remove(path);
        }
    }
}

void WriteImage(BmpImage* image, const char* path)
{
    if(image != NULL && path != NULL)
    {
        RemoveFile(path);

        FILE* output_file;
        fopen_s(&output_file, path, "w+");
        if(!output_file)
        {
            return;
        }

        fwrite(&image->bmp_header, sizeof(struct BmpHeader), 1, output_file);
        fwrite(&image->dib_header, sizeof(struct DibHeader), 1, output_file);
        fwrite(&image->color_header, sizeof(struct ColorHeader), 1, output_file);
        fwrite(image->data, image->dib_header.image_data_size, 1, output_file);
        fclose(output_file);
    }
}

//---
typedef struct
{
    int width;
    int height;
} CommandLineArgs;

CommandLineArgs GetCommandLineArgs(int argc, char** argv)
{
    CommandLineArgs args;
    args.height = 480;
    args.width = 640;

    if(argc > 1)
    {
        for(int i = 1; i < argc; i ++)
        {
            if(strcmp(argv[i - 1], "-w") == 0 || strcmp(argv[i - 1], "--width") == 0)
            {
                int w = strtol(argv[i], NULL, 10);
                args.width = w ? w : 640;
            }
            else if(strcmp(argv[i - 1], "-h") == 0 || strcmp(argv[i - 1], "--height") == 0)
            {
                int h = strtol(argv[i], NULL, 10);
                args.height = h ? h : 480;
            }
        }
    }

    return args;
}

int main(int argc, char** argv)
{
    CommandLineArgs args = GetCommandLineArgs(argc, argv);
    BmpImage* image = CreateBMPImage(args.width, args.height, 1);
    DrawCircleOnBMPImage(image);
    WriteImage(image, u8"./test.bmp");
    DestroyBmpImage(image);
    return 0;
}
