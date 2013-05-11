#include <Qt>
#include <QString>
#include <QtAlgorithms>
#include <QPoint>
#include <QRect>
#include <qmath.h>

#include <QWidget>
#include <QPainter>

#include "window.hpp"
#include "function.hpp"
#include "math.hpp"

#define ARROW_SIZE    10
#define SPINBOX_RANGE 0x400000
#define DRAW_POINTS   1000
#define CHECK_POINTS  100

#define CONVERT_X(x) (w * (x - a) / (b - a))
#define CONVERT_Y(y) (h * (sup - y) / (sup - inf))

#define WINDOW qobject_cast<MyMainWindow *>

MyMainWindow::MyMainWindow():
	toolBar("Toolbar", this),
	leftLabel("From:"),
	rightLabel("To:"),
	pointsLabel("Points:"),
	buildAction("Draw", this),
	functionAction("Original", this),
	polynomsAction("Polynoms", this),
	polynomsResidualAction("Residual", this),
	splinesAction("Splines", this),
	splinesResidualAction("Residual", this)
{
	// prepare widgets
	leftLabel.setMargin(5);
	rightLabel.setMargin(5);
	pointsLabel.setMargin(5);
	leftSpinBox.setRange(-100, 100);
	connect(&leftSpinBox, SIGNAL(valueChanged(double)),
		&drawArea, SLOT(requestUpdate()));
	connect(&rightSpinBox, SIGNAL(valueChanged(double)),
		&drawArea, SLOT(requestUpdate()));
	rightSpinBox.setRange(-100, 100);
	leftSpinBox.setValue(-10);
	rightSpinBox.setValue(10);
	// prepare actions
	functionAction.setCheckable(true);
	polynomsAction.setCheckable(true);
	polynomsAction.setChecked(true);
	polynomsResidualAction.setCheckable(true);
	splinesAction.setCheckable(true);
	splinesAction.setChecked(true);
	splinesResidualAction.setCheckable(true);
	// prepare toolbar
	toolBar.setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	toolBar.addWidget(&leftLabel);
	toolBar.addWidget(&leftSpinBox);
	toolBar.addWidget(&rightLabel);
	toolBar.addWidget(&rightSpinBox);
	toolBar.addWidget(&pointsLabel);
	toolBar.addWidget(&pointsSpinBox);
	toolBar.addSeparator();
	toolBar.addAction(&functionAction);
	toolBar.addAction(&polynomsAction);
	toolBar.addAction(&polynomsResidualAction);
	toolBar.addAction(&splinesAction);
	toolBar.addAction(&splinesResidualAction);
	toolBar.addSeparator();
	toolBar.addAction(&buildAction);
	connect(&buildAction, SIGNAL(triggered()), &drawArea, SLOT(repaint()));
#ifndef NO_AUTO_REPAINT
	connect(&functionAction, SIGNAL(triggered(bool)),
		&drawArea, SLOT(repaint()));
	connect(&splinesAction, SIGNAL(triggered(bool)),
		&drawArea, SLOT(repaint()));
	connect(&splinesResidualAction, SIGNAL(triggered(bool)),
		&drawArea, SLOT(repaint()));
	connect(&polynomsAction, SIGNAL(triggered(bool)),
		&drawArea, SLOT(repaint()));
	connect(&polynomsResidualAction, SIGNAL(triggered(bool)),
		&drawArea, SLOT(repaint()));
#endif
	// prepare window
	resize(1000, 800);
	setCentralWidget(&drawArea);
	addToolBar(&toolBar);
}

void DrawArea::requestUpdate() {
	updateRequested = true;
}

DrawableFlags MyMainWindow::drawableFlags() const {
	DrawableFlags result = DrawNone;
	if (functionAction.isChecked())
		result |= DrawFunction;
	if (polynomsAction.isChecked())
		result |= DrawPolynoms;
	if (polynomsResidualAction.isChecked())
		result |= DrawPolynomsResidual;
	if (splinesAction.isChecked())
		result |= DrawSplines;
	if (splinesResidualAction.isChecked())
		result |= DrawSplinesResidual;
	return result;
}

double origFunction(DrawArea *, double point) {
	return function(point);
}

double polynom(DrawArea *area, double point) {
	MyMainWindow *parentWindow = WINDOW(area->parentWidget());
	quint32 size = parentWindow->getNumberOfPoints();
	return getPolynomValue(size, area->chebAlphas, point, area->a, area->b);
}

double polynomResidual(DrawArea *area, double point) {
	return polynom(area, point) - function(point);
}

double splines(DrawArea *area, double point) {
	MyMainWindow *parentWindow = WINDOW(area->parentWidget());
	quint32 size = parentWindow->getNumberOfPoints();
	return getSplinesValue(size, area->splinesRightCol, point, area->a,
		area->b, area->splinesValues);
}

double splinesResidual(DrawArea *area, double point) {
	return splines(area, point) - function(point);
}

void DrawArea::paintEvent(QPaintEvent *event) {
	QWidget::paintEvent(event);
	drawAxes();
	MyMainWindow *parentWindow = WINDOW(parentWidget());
	a = parentWindow->getA();
	b = parentWindow->getB();
	if (a > b)
		qSwap(a, b);
	DrawableFlags flags = parentWindow->drawableFlags();
	if (flags & DrawFunction)
		drawAbstractFunction(Qt::darkGray, origFunction);
	if (flags & DrawPolynoms)
		drawPolynom();
	if (flags & DrawPolynomsResidual)
		drawPolynom(true);
	if (flags & DrawSplines)
		drawSplines();
	if (flags & DrawSplinesResidual)
		drawSplines(true);
	updateRequested = false;
}

void DrawArea::drawAxes() {
	QPainter painter(this);
	QRect rectangle = rect();
	painter.setPen(Qt::darkGray);
	int x1 = rectangle.topLeft().x();
	int x2 = rectangle.topRight().x();
	int y1 = rectangle.topLeft().y();
	int y2 = rectangle.bottomLeft().y();
	painter.drawLine((x1+x2)/2, y1, (x1+x2)/2, y2);
	painter.drawLine(x1, (y1+y2)/2, x2, (y1+y2)/2);
#ifdef ENABLE_AXES
	painter.drawLine(x2 - ARROW_SIZE, (y1+y2)/2 + ARROW_SIZE/2, x2, (y1+y2)/2);
	painter.drawLine(x2 - ARROW_SIZE, (y1+y2)/2 - ARROW_SIZE/2, x2, (y1+y2)/2);
	painter.drawLine((x1+x2)/2 + ARROW_SIZE/2, y1 + ARROW_SIZE, (x1+x2)/2, y1);
	painter.drawLine((x1+x2)/2 - ARROW_SIZE/2, y1 + ARROW_SIZE, (x1+x2)/2, y1);
#endif
}

DrawArea::~DrawArea() {
	if (chebAllocSize) {
		delete[] chebPoints;
		delete[] chebValues;
		delete[] chebAlphas;
	}
	if (splinesAllocSize) {
		delete[] splinesRightCol;
		delete[] splinesValues;
	}
}

void DrawArea::drawAbstractFunction(QColor color, double f(DrawArea *, double),
                                    int textPos)
{
	QPainter painter(this);
	painter.setPen(color);
	double step = (b - a) / CHECK_POINTS, point, value;
	double inf, sup;
	inf = f(this, a + step);
	sup = inf;
	for (point = a + 2 * step; point < b - step; point += step) {
		value = f(this, point);
		if (value < inf)
			inf = value;
		if (value > sup)
			sup = value;
	}
#ifdef ENABLE_AXES
	sup = qMax(qAbs(inf), qAbs(sup));
	inf = -sup;
#endif
	double w = width(), h = height();
	QPoint pointf, oldpointf;
	for (point = a; point < b - step/2; point += step) {
		value = f(this, point);
		pointf = QPoint(CONVERT_X(point), CONVERT_Y(value));
		if (point > a + step)
			painter.drawLine(oldpointf, pointf);
		oldpointf = pointf;
	}
	if (textPos) {
		QString text = QLatin1String("Residual: %1");
		double residual = qMax(qAbs(inf), qAbs(sup));
		painter.drawText(w - 200, h - textPos, text.arg(residual));
	}
}

void DrawArea::drawPolynom(bool residual) {
	MyMainWindow *parentWindow = WINDOW(parentWidget());
	quint32 size = parentWindow->getNumberOfPoints();
	if (chebAllocSize != size || updateRequested) {
		if (chebAllocSize) {
			delete[] chebAlphas;
			delete[] chebPoints;
			delete[] chebValues;
		}
		chebAllocSize = size;
		chebAlphas = new double[size];
		chebPoints = new double[size];
		chebValues = new double[size];
		for (quint32 i = 0; i < size; ++i) {
			chebPoints[i] = (a + b) + (b - a) * qCos((M_PI * (i + .5)) / size);
			chebPoints[i] /= 2;
			chebValues[i] = function(chebPoints[i]);
		}
		getAlphas(size, chebAlphas, chebPoints, chebValues, a, b);
	}
	if (residual)
		drawAbstractFunction(Qt::red, polynomResidual, 30);
	else
		drawAbstractFunction(Qt::darkBlue, polynom);
}

void DrawArea::drawSplines(bool residual) {
	MyMainWindow *parentWindow = WINDOW(parentWidget());
	quint32 size = parentWindow->getNumberOfPoints();
	if (splinesAllocSize != size || updateRequested) {
		if (splinesAllocSize) {
			delete[] splinesValues;
			delete[] splinesRightCol;
		}
		splinesAllocSize = size;
		splinesRightCol = new double[size+1];
		splinesValues   = new double[size+1];
		double *botDiag = new double[size];
		double *midDiag = new double[size+1];
		double *topDiag = new double[size];
		for (quint32 i = 0; i <= size; ++i)
			splinesValues[i] = function(POINT(i));
		fillMatrix(size, botDiag, midDiag, topDiag, splinesRightCol,
			a, b, splinesValues);
		getDeltas(size, botDiag, midDiag, topDiag, splinesRightCol);
		delete[] botDiag;
		delete[] midDiag;
		delete[] topDiag;
	}
	if (residual)
		drawAbstractFunction(Qt::darkYellow, splinesResidual, 12);
	else
		drawAbstractFunction(Qt::darkMagenta, splines);
}

QAbstractSpinBox::StepEnabled BinarySpinBox::stepEnabled() const {
	QAbstractSpinBox::StepEnabled result = QAbstractSpinBox::StepNone;
	if (_value < SPINBOX_RANGE)
		result |= QAbstractSpinBox::StepUpEnabled;
	if (_value > 1)
		result |= QAbstractSpinBox::StepDownEnabled;
	return result;
}

void BinarySpinBox::stepBy(int steps) {
	while (steps < 0 && _value > 1) {
		_value /= 2;
		++steps;
	}
	while (steps > 0 && _value < SPINBOX_RANGE) {
		_value *= 2;
		--steps;
	}
	lineEdit()->setText(text());
}
