#include <QtCore/QFlags>

#include <QtGui/QMainWindow>
#include <QtGui/QWidget>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QToolBar>
#include <QtGui/QPushButton>
#include <QtGui/QAction>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QAbstractSpinBox>
#include <QtGui/QDoubleSpinBox>

enum Drawable {
	DrawNone             = 0x00,
	DrawFunction         = 0x01,
	DrawPolynoms         = 0x02,
	DrawPolynomsResidual = 0x04,
	DrawSplines          = 0x08,
	DrawSplinesResidual  = 0x10
};

Q_DECLARE_FLAGS(DrawableFlags, Drawable)

class DrawArea: public QWidget {

Q_OBJECT

public:
	double a;
	double b;
	double *chebPoints;
	double *chebValues;
	double *chebAlphas;
	double *splinesRightCol;
	double *splinesValues;
	DrawArea():
		chebAllocSize(0),
		splinesAllocSize(0)
	{}
	~DrawArea();
	virtual void paintEvent(QPaintEvent *event);

public slots:
	void requestUpdate();

private:
	quint32 chebAllocSize;
	quint32 splinesAllocSize;
	bool updateRequested;

	void drawAxes();
	void drawFunction();
	void drawPolynom(bool residual = false);
	void drawSplines(bool residual = false);
	void drawAbstractFunction(QColor color, double f(DrawArea *, double),
	                          int textPos = 0);
};

double origFunction(DrawArea *area, double point);
double polynom(DrawArea *area, double point);
double polynomResidual(DrawArea *area, double point);

class BinarySpinBox: public QAbstractSpinBox {

Q_OBJECT

private:
	quint32 _value;

public:
	BinarySpinBox():
		_value(128)
	{
		setMinimumWidth(95);
		lineEdit()->setText(text());
		setReadOnly(true);
	}
	quint32 value() const {
		return _value;
	}
	QAbstractSpinBox::StepEnabled stepEnabled() const;
	QString text() const {
		return QString::number(_value);
	}
	void stepBy(int steps);
};

class MyMainWindow: public QMainWindow {

Q_OBJECT

public:
	MyMainWindow();
	quint32 getNumberOfPoints() const {
		return pointsSpinBox.value();
	}
	double getA() const {
		return leftSpinBox.value();
	}
	double getB() const {
		return rightSpinBox.value();
	}
	DrawableFlags drawableFlags() const;

private:
	QToolBar toolBar;
	QLabel leftLabel;
	QLabel rightLabel;
	QDoubleSpinBox leftSpinBox;
	QDoubleSpinBox rightSpinBox;
	QLabel pointsLabel;
	BinarySpinBox pointsSpinBox;
	QAction buildAction;
	QAction functionAction;
	QAction polynomsAction;
	QAction polynomsResidualAction;
	QAction splinesAction;
	QAction splinesResidualAction;
	DrawArea drawArea;
};
