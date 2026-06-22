#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define WIDTH 400
#define HEIGHT 400
#define NUM_FRAMES 100

double W[HEIGHT][WIDTH];
unsigned char IMG[HEIGHT][WIDTH];

#pragma pack(push,1)

typedef struct
{
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BMPFILEHEADER;

typedef struct
{
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BMPINFOHEADER;

#pragma pack(pop)

void saveBMP(const char *filename)
{
    FILE *fp = fopen(filename, "wb");
    if(fp == NULL)
    {
        printf("Cannot open %s for writing\n", filename);
        return;
    }

    int rowSize = ((WIDTH + 3) / 4) * 4;
    int imageSize = rowSize * HEIGHT;

    BMPFILEHEADER fh;
    BMPINFOHEADER ih;

    fh.bfType = 0x4D42;
    fh.bfOffBits = 14 + 40 + 256 * 4;
    fh.bfReserved1 = 0;
    fh.bfReserved2 = 0;
    fh.bfSize = fh.bfOffBits + imageSize;

    ih.biSize = 40;
    ih.biWidth = WIDTH;
    ih.biHeight = HEIGHT;
    ih.biPlanes = 1;
    ih.biBitCount = 8;
    ih.biCompression = 0;
    ih.biSizeImage = imageSize;
    ih.biXPelsPerMeter = 0;
    ih.biYPelsPerMeter = 0;
    ih.biClrUsed = 256;
    ih.biClrImportant = 256;

    fwrite(&fh, sizeof(fh), 1, fp);
    fwrite(&ih, sizeof(ih), 1, fp);

    for(int i = 0; i < 256; i++)
    {
        unsigned char color[4] = { i, i, i, 0 };
        fwrite(color, 4, 1, fp);
    }

    unsigned char *row = (unsigned char*)calloc(rowSize, 1);

    for(int y = HEIGHT - 1; y >= 0; y--)
    {
        for(int x = 0; x < WIDTH; x++)
            row[x] = IMG[y][x];

        fwrite(row, rowSize, 1, fp);
    }

    free(row);
    fclose(fp);
}

int main()
{
    for(int frame = 0; frame < NUM_FRAMES; frame++)
    {
        char txtname[64];
        char bmpname[64];
        FILE *fp;

        sprintf(txtname, "wavefront_%03d.txt", frame);
        sprintf(bmpname, "wavefront_%03d.bmp", frame);

        fp = fopen(txtname, "r");
        if(fp == NULL)
        {
            printf("Cannot open %s\n", txtname);
            continue;
        }

        double wmin, wmax;

        for(int y = 0; y < HEIGHT; y++)
        {
            for(int x = 0; x < WIDTH; x++)
            {
                if(fscanf(fp, "%lf", &W[y][x]) != 1)
                {
                    printf("Error reading %s\n", txtname);
                    fclose(fp);
                    return 1;
                }

                if(y == 0 && x == 0)
                {
                    wmin = W[y][x];
                    wmax = W[y][x];
                }

                if(W[y][x] < wmin) wmin = W[y][x];
                if(W[y][x] > wmax) wmax = W[y][x];
            }
        }

        fclose(fp);

        for(int y = 0; y < HEIGHT; y++)
        {
            for(int x = 0; x < WIDTH; x++)
            {
                if(wmax == wmin)
                    IMG[y][x] = 0;
                else
                    IMG[y][x] = (unsigned char)(
                        255.0 * (W[y][x] - wmin) / (wmax - wmin)
                    );
            }
        }

        saveBMP(bmpname);
        printf("%s generated\n", bmpname);
    }

    return 0;
}