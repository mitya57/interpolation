#include <QtGlobal>

#define DISTANCE    ((b - a) / size)
#define POINT(i)    (a + DISTANCE * i)

void getAlphas(quint32 size, double *alphas, double *points,
               double *values, double a, double b);

double getPolynomValue(quint32 size, double *alphas, double point,
                       double a, double b);

void fillMatrix(quint32 size, double *botDiag, double *midDiag,
                double *topDiag, double *rightCol, double a, double b,
                double *values);

void getDeltas(quint32 size, double *botDiag, double *midDiag,
               double *topDiag, double *rightCol);

double getSplinesValue(quint32 size, double *rightCol, double point,
                       double a, double b, double *values);
