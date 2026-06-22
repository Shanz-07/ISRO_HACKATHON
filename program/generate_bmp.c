#include <stdio.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define WIDTH 400
#define HEIGHT 400

unsigned char image[HEIGHT][WIDTH];

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

void drawSpot(int cx,int cy,int radius)
{
    for(int y=cy-radius;y<=cy+radius;y++)
    {
        for(int x=cx-radius;x<=cx+radius;x++)
        {
            if(x<0 || x>=WIDTH || y<0 || y>=HEIGHT)
                continue;

            int dx=x-cx;
            int dy=y-cy;

            if(dx*dx+dy*dy <= radius*radius)
                image[y][x]=255;
        }
    }
}

void saveBMP(const char *filename)
{
    FILE *fp=fopen(filename,"wb");

    int rowSize=((WIDTH+3)/4)*4;
    int imageSize=rowSize*HEIGHT;

    BMPFILEHEADER fh;
    BMPINFOHEADER ih;

    fh.bfType=0x4D42;
    fh.bfOffBits=14+40+256*4;
    fh.bfReserved1=0;
    fh.bfReserved2=0;
    fh.bfSize=fh.bfOffBits+imageSize;

    ih.biSize=40;
    ih.biWidth=WIDTH;
    ih.biHeight=HEIGHT;
    ih.biPlanes=1;
    ih.biBitCount=8;
    ih.biCompression=0;
    ih.biSizeImage=imageSize;
    ih.biXPelsPerMeter=0;
    ih.biYPelsPerMeter=0;
    ih.biClrUsed=256;
    ih.biClrImportant=256;

    fwrite(&fh,sizeof(fh),1,fp);
    fwrite(&ih,sizeof(ih),1,fp);

    for(int i=0;i<256;i++)
    {
        unsigned char color[4]={i,i,i,0};
        fwrite(color,4,1,fp);
    }

    unsigned char *row=(unsigned char*)calloc(rowSize,1);

    for(int y=HEIGHT-1;y>=0;y--)
    {
        memcpy(row,image[y],WIDTH);
        fwrite(row,rowSize,1,fp);
    }

    free(row);
    fclose(fp);
}


#include <math.h>

int main()
{
    for(int frame=0; frame<100; frame++)
    {
        memset(image,0,sizeof(image));

        double t = frame * 0.15;

        double global_dx = 8.0*sin(t);
        double global_dy = 6.0*cos(t);

        for(int r=0;r<4;r++)
        {
            for(int c=0;c<4;c++)
            {
                int cx = 50 + c*100;
                int cy = 50 + r*100;

                double local_dx =
                    2.0*sin(t + r*0.4 + c*0.3);

                double local_dy =
                    2.0*cos(t + r*0.3 + c*0.4);

                int x =
                    (int)(cx + global_dx + local_dx);

                int y =
                    (int)(cy + global_dy + local_dy);

                drawSpot(x,y,4);
            }
        }

        char filename[64];

        sprintf(filename,
                "frame_%03d.bmp",
                frame);

        saveBMP(filename);

        printf("%s generated\n",
               filename);
    }

    return 0;
}
