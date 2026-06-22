#include <stdio.h>
#include <math.h>

#define N 100
#define DT 0.15

int main()
{
    FILE *fp = fopen("phi_series.txt","r");

    if(fp == NULL)
    {
        printf("Cannot open phi_series.txt\n");
        return 1;
    }

    double phi[N];

    for(int i=0;i<N;i++)
    {
        int frame;
        double p1,p2,p3,p4,p5;

        fscanf(fp,
               "%d %lf %lf %lf %lf %lf",
               &frame,
               &p1,
               &p2,
               &p3,
               &p4,
               &p5);

        phi[i] = p2;    /* Use Phi2 */
    }

    fclose(fp);

    double R0 = 0.0;

    for(int i=0;i<N;i++)
    {
        R0 += phi[i]*phi[i];
    }

    R0 /= N;

    printf("\nLag\tCorrelation\n\n");

    int tauLag = -1;

    for(int lag=0; lag<50; lag++)
    {
        double R = 0.0;

        for(int i=0;i<N-lag;i++)
        {
            R += phi[i]*phi[i+lag];
        }

        R /= (N-lag);

        double Rn = R/R0;

        printf("%2d\t%lf\n",
               lag,
               Rn);

        if(tauLag==-1 &&
           lag>0 &&
           Rn < 0.367879)
        {
            tauLag = lag;
        }
    }

    if(tauLag != -1)
    {
        double tau0 =
        tauLag * DT;

        printf("\nTau0 = %lf\n",
               tau0);

        printf("Lag  = %d frames\n",
               tauLag);
    }
    else
    {
        printf(
        "\nCorrelation never dropped below 1/e\n");
    }

    return 0;
}