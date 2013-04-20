#include <QtGui/QApplication>
#include "window.hpp"

int main(int argc, char **argv) {
	QApplication app(argc, argv);
	MyMainWindow window;
	window.show();
	return app.exec();
}
