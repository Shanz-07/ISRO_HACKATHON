#include <stdio.h>
#include <math.h>

#define GRID 400
#define NUM_FRAMES 100
#define SHIFT 10

double W[GRID][GRID];

int main()
{
    double D_total = 0.0;
    int frame_count = 0;

    for(int frame=0; frame<NUM_FRAMES; frame++)
    {
        char filename[64];

        sprintf(filename,
                "wavefront_%03d.txt",
                frame);

        FILE *fp = fopen(filename,"r");

        if(fp == NULL)
        {
            printf("Cannot open %s\n",
                   filename);
            continue;
        }

        for(int y=0;y<GRID;y++)
        {
            for(int x=0;x<GRID;x++)
            {
                fscanf(fp,"%lf",
                       &W[y][x]);
            }
        }

        fclose(fp);

        double D = 0.0;
        long count = 0;

        for(int y=0;y<GRID;y++)
        {
            for(int x=0;
                x<GRID-SHIFT;
                x++)
            {
                double diff =
                    W[y][x+SHIFT]
                    -
                    W[y][x];

                D += diff*diff;

                count++;
            }
        }

        D /= count;

        printf(
            "Frame %03d  D(r)=%.10e\n",
            frame,
            D
        );

        D_total += D;
        frame_count++;
    }

    double Davg =
        D_total /
        frame_count;

    double r0 =
        SHIFT /
        pow(Davg/6.88,
            3.0/5.0);

    printf("\n");
    printf("Average Structure Function = %.10e\n",
           Davg);

    printf("Estimated r0 = %.6f pixels\n",
           r0);

    return 0;
}