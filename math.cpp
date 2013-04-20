#include "math.hpp"

#define EPS .001
#define DDIFF(i, j) ((values[j] - values[i]) / DISTANCE)

void getAlphas(quint32 size, double *alphas, double *points,
               double *values, double a, double b)
{
	double *tcap = new double[size];
	double *tcapprev = new double[size];
	double *tcapnew = new double[size];
	quint32 i, j;
	alphas[0] = 0;
	for (i = 0; i < size; ++i) {
		tcap[i] = 1;
		alphas[0] += values[i];
	}
	alphas[0] /= size;
	for (j = 1; j < size; ++j) {
		alphas[j] = 0;
		for (i = 0; i < size; ++i) {
			tcapnew[i] = (2 * points[i] - b - a) / (b - a);
			if (j > 1)
				tcapnew[i] = 2 * tcapnew[i] * tcap[i] - tcapprev[i];
			tcapprev[i] = tcap[i];
			tcap[i] = tcapnew[i];
			alphas[j] += values[i] * tcap[i];
		}
		alphas[j] /= size;
		alphas[j] *= 2;
	}
	delete[] tcap;
	delete[] tcapprev;
	delete[] tcapnew;
}

double getPolynomValue(quint32 size, double *alphas, double point,
                       double a, double b)
{
	double tcap, tcapprev, tcapnew;
	double value;
	tcapprev = 1;
	tcap = (2 * point - b - a) / (b - a);
	value = alphas[0] * tcapprev + alphas[1] * tcap;
	for (quint32 i = 2; i < size; ++i) {
		tcapnew = 2 * (2 * point - b - a) / (b - a);
		tcapnew *= tcap;
		tcapnew -= tcapprev;
		tcapprev = tcap;
		tcap = tcapnew;
		value += alphas[i] * tcap;
	}
	return value;
}

void fillMatrix(quint32 size, double *botDiag, double *midDiag,
                double *topDiag, double *rightCol, double a, double b,
                double *values)
{
	midDiag[0] = 2;
	topDiag[0] = 1;
	rightCol[0] = 3 * DDIFF(0, 1);
	for (quint32 i = 1; i < size; ++i) {
		botDiag[i-1] = DISTANCE;
		midDiag[i] = 4 * DISTANCE;
		topDiag[i] = DISTANCE;
		rightCol[i] = 3 * DDIFF(i-1, i) * DISTANCE
		            + 3 * DDIFF(i, i+1) * DISTANCE;
	}
	botDiag[size-1] = 1;
	midDiag[size] = 2;
	rightCol[size] = 3 * DDIFF(size-1, size);
}

void getDeltas(quint32 size, double *botDiag, double *midDiag,
               double *topDiag, double *rightCol)
{
	quint32 s;
	for (s = 0; s < size; ++s) {
		topDiag[s] /= midDiag[s];
		rightCol[s] /= midDiag[s];
		midDiag[s+1] -= botDiag[s] * topDiag[s];
		rightCol[s+1] -= botDiag[s] * rightCol[s];
	}
	rightCol[size] /= midDiag[size];
	for (s = size; s >= 1; --s)
		rightCol[s-1] -= topDiag[s-1] * rightCol[s];
}

double getSplinesValue(quint32 size, double *rightCol, double point,
                       double a, double b, double *values)
{
	// number of nearest point to the left
	quint32 i = (point - a) / DISTANCE;
	double c1, c2, c3, c4;
	c1 = values[i];
	c2 = rightCol[i];
	c3 = (3 * DDIFF(i, i+1) - 2 * rightCol[i] - rightCol[i+1]) / DISTANCE;
	c4 = (rightCol[i] + rightCol[i+1] - 2 * DDIFF(i, i+1)) / (DISTANCE * DISTANCE);
	double d = point - POINT(i);
	return c1 + c2*d + c3*d*d + c4*d*d*d;
}
