#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUM_MODES   5
#define NUM_ACT     16
#define NUM_FRAMES  100

/* Same normalized actuator/lenslet coordinates used earlier */
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

void buildInfluenceMatrix(double H[NUM_ACT][NUM_ACT])
{
    for(int i = 0; i < NUM_ACT; i++)
        for(int j = 0; j < NUM_ACT; j++)
            H[i][j] = 0.0;

    for(int r = 0; r < 4; r++)
    {
        for(int c = 0; c < 4; c++)
        {
            int id = r * 4 + c;

            H[id][id] = 1.0;

            if(c > 0) H[id][id - 1] = 0.2;
            if(c < 3) H[id][id + 1] = 0.2;
            if(r > 0) H[id][id - 4] = 0.2;
            if(r < 3) H[id][id + 4] = 0.2;
        }
    }
}

void computePhaseAtActuators(const double phi[NUM_MODES], double phase[NUM_ACT])
{
    for(int i = 0; i < NUM_ACT; i++)
    {
        double x = X[i];
        double y = Y[i];

        double z2 = x;
        double z3 = y;
        double z4 = 2.0 * (x * x + y * y) - 1.0;
        double z5 = 2.0 * x * y;
        double z6 = x * x - y * y;

        phase[i] =
            phi[0] * z2 +
            phi[1] * z3 +
            phi[2] * z4 +
            phi[3] * z5 +
            phi[4] * z6;
    }
}

/* Solve H * a = phase using Gauss-Jordan elimination */
int solve16(double H[NUM_ACT][NUM_ACT], double phase[NUM_ACT], double actuator[NUM_ACT])
{
    double aug[NUM_ACT][NUM_ACT + 1];

    for(int i = 0; i < NUM_ACT; i++)
    {
        for(int j = 0; j < NUM_ACT; j++)
            aug[i][j] = H[i][j];

        aug[i][NUM_ACT] = phase[i];
    }

    for(int i = 0; i < NUM_ACT; i++)
    {
        int pivotRow = i;
        double maxVal = fabs(aug[i][i]);

        for(int r = i + 1; r < NUM_ACT; r++)
        {
            double v = fabs(aug[r][i]);
            if(v > maxVal)
            {
                maxVal = v;
                pivotRow = r;
            }
        }

        if(maxVal < 1e-12)
        {
            printf("Singular DM matrix\n");
            return 0;
        }

        if(pivotRow != i)
        {
            for(int j = 0; j < NUM_ACT + 1; j++)
            {
                double tmp = aug[i][j];
                aug[i][j] = aug[pivotRow][j];
                aug[pivotRow][j] = tmp;
            }
        }

        double pivot = aug[i][i];
        for(int j = i; j < NUM_ACT + 1; j++)
            aug[i][j] /= pivot;

        for(int r = 0; r < NUM_ACT; r++)
        {
            if(r == i)
                continue;

            double factor = aug[r][i];

            for(int j = i; j < NUM_ACT + 1; j++)
                aug[r][j] -= factor * aug[i][j];
        }
    }

    for(int i = 0; i < NUM_ACT; i++)
        actuator[i] = aug[i][NUM_ACT];

    return 1;
}

int main(void)
{
    FILE *fp = fopen("phi_series.txt", "r");
    if(fp == NULL)
    {
        printf("Cannot open phi_series.txt\n");
        return 1;
    }

    FILE *out = fopen("dm_series.txt", "w");
    if(out == NULL)
    {
        printf("Cannot write dm_series.txt\n");
        fclose(fp);
        return 1;
    }

    double H[NUM_ACT][NUM_ACT];
    buildInfluenceMatrix(H);

    printf("DM influence matrix built.\n");
    fprintf(out, "# frame a1 a2 a3 ... a16\n");

    for(int frame = 0; frame < NUM_FRAMES; frame++)
    {
        int idx;
        double phi[NUM_MODES];

        if(fscanf(fp, "%d %lf %lf %lf %lf %lf",
                  &idx, &phi[0], &phi[1], &phi[2], &phi[3], &phi[4]) != 6)
        {
            printf("Error reading phi_series.txt at frame %d\n", frame);
            break;
        }

        double phase[NUM_ACT];
        double actuator[NUM_ACT];

        computePhaseAtActuators(phi, phase);

        if(!solve16(H, phase, actuator))
        {
            printf("Failed to solve DM system at frame %d\n", frame);
            break;
        }

        fprintf(out, "%03d ", idx);
        for(int i = 0; i < NUM_ACT; i++)
        {
            fprintf(out, "%lf ", actuator[i]);
        }
        fprintf(out, "\n");

        printf("Processed frame %03d\n", idx);
    }

    fclose(fp);
    fclose(out);

    printf("DM actuator commands saved to dm_series.txt\n");
    return 0;
}