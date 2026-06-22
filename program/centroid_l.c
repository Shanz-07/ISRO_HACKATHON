#include <stdio.h>
#include <stdlib.h>

#define WIDTH 400
#define HEIGHT 400
#define SUBAP 100
#define THRESHOLD 10
#define NUM_LENS 16
#define NUM_FRAMES 100
#define GRID 400

#define PIXEL_SIZE   5e-6
#define FOCAL_LENGTH 0.02

typedef struct
{
    double x;
    double y;
} Centroid;

/* Reference spot positions */
Centroid reference[NUM_LENS] =
{
    { 50.0,  50.0},
    {150.0,  50.0},
    {250.0,  50.0},
    {350.0,  50.0},

    { 50.0, 150.0},
    {150.0, 150.0},
    {250.0, 150.0},
    {350.0, 150.0},

    { 50.0, 250.0},
    {150.0, 250.0},
    {250.0, 250.0},
    {350.0, 250.0},

    { 50.0, 350.0},
    {150.0, 350.0},
    {250.0, 350.0},
    {350.0, 350.0}
};

/* Normalized centroids */
double X[16] =
{
   -0.75,-0.25, 0.25, 0.75,
   -0.75,-0.25, 0.25, 0.75,
   -0.75,-0.25, 0.25, 0.75,
   -0.75,-0.25, 0.25, 0.75
};

double Y[16] =
{
   -0.75,-0.75,-0.75,-0.75,
   -0.25,-0.25,-0.25,-0.25,
    0.25, 0.25, 0.25, 0.25,
    0.75, 0.75, 0.75, 0.75
};


int loadBMP(const char *filename, unsigned char image[HEIGHT][WIDTH])
{
    FILE *fp = fopen(filename, "rb");
    if(fp == NULL)
    {
        printf("Cannot open %s\n", filename);
        return 0;
    }

    unsigned char header[54];
    fread(header, 1, 54, fp);

    int width  = *(int*)&header[18];
    int height = *(int*)&header[22];
    int bits   = *(short*)&header[28];

    if(width != WIDTH || height != HEIGHT)
    {
        printf("BMP must be 400x400\n");
        fclose(fp);
        return 0;
    }

    if(bits != 8)
    {
        printf("BMP must be 8-bit grayscale\n");
        fclose(fp);
        return 0;
    }

    fseek(fp, 1078, SEEK_SET);

    for(int y = HEIGHT - 1; y >= 0; y--)
    {
        fread(image[y], 1, WIDTH, fp);
    }

    fclose(fp);
    return 1;
}



Centroid computeCentroid(
    unsigned char image[HEIGHT][WIDTH],
    int x0,
    int y0,
    int x1,
    int y1)
{
    Centroid c;
    double sumI  = 0.0;
    double sumXI = 0.0;
    double sumYI = 0.0;

    for(int y = y0; y < y1; y++)
    {
        for(int x = x0; x < x1; x++)
        {
            double I = image[y][x];
            if(I < THRESHOLD)
                continue;

            sumI  += I;
            sumXI += x * I;
            sumYI += y * I;
        }
    }

    if(sumI == 0.0)
    {
        c.x = -1.0;
        c.y = -1.0;
        return c;
    }

    c.x = sumXI / sumI;
    c.y = sumYI / sumI;

    return c;
}

void invert5x5(double A[5][5], double INV[5][5])
{
    double aug[5][10];

    for(int i = 0; i < 5; i++)
    {
        for(int j = 0; j < 5; j++)
        {
            aug[i][j] = A[i][j];
            aug[i][j + 5] = (i == j) ? 1.0 : 0.0;
        }
    }

    for(int i = 0; i < 5; i++)
    {
        double pivot = aug[i][i];

        if(pivot == 0.0)
        {
            printf("Singular Matrix\n");
            exit(1);
        }

        for(int j = 0; j < 10; j++)
            aug[i][j] /= pivot;

        for(int k = 0; k < 5; k++)
        {
            if(k == i)
                continue;

            double factor = aug[k][i];

            for(int j = 0; j < 10; j++)
                aug[k][j] -= factor * aug[i][j];
        }
    }

    for(int i = 0; i < 5; i++)
        for(int j = 0; j < 5; j++)
            INV[i][j] = aug[i][j + 5];
}

void saveWavefrontTxt(const char *filename, double W[GRID][GRID])
{
    FILE *fp = fopen(filename, "w");
    if(fp == NULL)
    {
        printf("Cannot write %s\n", filename);
        return;
    }

    for(int y = 0; y < GRID; y++)
    {
        for(int x = 0; x < GRID; x++)
        {
            fprintf(fp, "%lf ", W[y][x]);
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
}

int main()
{
    FILE *phi_fp = fopen("phi_series.txt", "w");
    if(phi_fp == NULL)
    {
        printf("Cannot write phi_series.txt\n");
        return 1;
    }
    double A[32][5];
    double ATA[5][5];
    double INV[5][5];

    /* Build A once */
    for(int i = 0; i < 16; i++)
    {
        double x = X[i];
        double y = Y[i];
        int r = 2 * i;

        A[r][0] = 1.0;
        A[r][1] = 0.0;
        A[r][2] = 4.0 * x;
        A[r][3] = 2.0 * y;
        A[r][4] = 2.0 * x;

        A[r + 1][0] = 0.0;
        A[r + 1][1] = 1.0;
        A[r + 1][2] = 4.0 * y;
        A[r + 1][3] = 2.0 * x;
        A[r + 1][4] = -2.0 * y;
    }

    /* Build ATA once */
    for(int i = 0; i < 5; i++)
    {
        for(int j = 0; j < 5; j++)
        {
            ATA[i][j] = 0.0;
            for(int k = 0; k < 32; k++)
                ATA[i][j] += A[k][i] * A[k][j];
        }
    }

    invert5x5(ATA, INV);

    for(int frame = 0; frame < NUM_FRAMES; frame++)
    {
        char filename[64];
        char outname[64];

        unsigned char image[HEIGHT][WIDTH];
        double S[32];
        double ATS[5];
        double PHI[5];
        double W[GRID][GRID];

        sprintf(filename, "frame_%03d.bmp", frame);

        if(!loadBMP(filename, image))
            continue;

        /* Compute centroids and slope vector S */
        int k = 0;
        for(int row = 0; row < 4; row++)
        {
            for(int col = 0; col < 4; col++)
            {
                int x0 = col * SUBAP;
                int x1 = x0 + SUBAP;
                int y0 = row * SUBAP;
                int y1 = y0 + SUBAP;

                Centroid current = computeCentroid(image, x0, y0, x1, y1);

                double dx = current.x - reference[k].x;
                double dy = current.y - reference[k].y;

                double sx = dx * PIXEL_SIZE / FOCAL_LENGTH;
                double sy = dy * PIXEL_SIZE / FOCAL_LENGTH;

                S[2 * k]     = sx;
                S[2 * k + 1] = sy;

                k++;
            }
        }

        /* ATS = A^T S */
        for(int i = 0; i < 5; i++)
        {
            ATS[i] = 0.0;
            for(int r = 0; r < 32; r++)
                ATS[i] += A[r][i] * S[r];
        }

        /* PHI = INV * ATS */
        for(int i = 0; i < 5; i++)
        {
            PHI[i] = 0.0;
            for(int j = 0; j < 5; j++)
                PHI[i] += INV[i][j] * ATS[j];
        }
        fprintf(phi_fp, "%03d ", frame);
        for(int i = 0; i < 5; i++)
        {
            fprintf(phi_fp, "%lf ", PHI[i]);
        }
        fprintf(phi_fp, "\n");
        

        /* Reconstruct wavefront W(x,y) */
        for(int iy = 0; iy < GRID; iy++)
        {
            for(int ix = 0; ix < GRID; ix++)
            {
                double x = -1.0 + 2.0 * ix / (GRID - 1);
                double y = -1.0 + 2.0 * iy / (GRID - 1);

                double z2 = x;
                double z3 = y;
                double z4 = 2.0 * (x * x + y * y) - 1.0;
                double z5 = 2.0 * x * y;
                double z6 = x * x - y * y;

                W[iy][ix] =
                    PHI[0] * z2 +
                    PHI[1] * z3 +
                    PHI[2] * z4 +
                    PHI[3] * z5 +
                    PHI[4] * z6;
            }
        }

        sprintf(outname, "wavefront_%03d.txt", frame);
        saveWavefrontTxt(outname, W);
        
        printf("Processed %s -> %s\n", filename, outname);
    }
    fclose(phi_fp);

    return 0;
}