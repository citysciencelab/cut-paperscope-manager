/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INCLUDES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	// Qt
	#include <QApplication>
	#include <QFile>

	// app
	#include "MainWindow.h"



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	MAIN
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	int main(int argc, char **argv) {
	
		QApplication app(argc, argv);
		
		app.setApplicationName("PaperScope Manager");
		app.setOrganizationName("HafenCity Universitaet Hamburg");
		app.setOrganizationDomain("hcu-hamburg.de");

		// load custom ui styling
		QFile file(":/css/stylesheet.qss");
		file.open(QFile::ReadOnly);
		app.setStyleSheet(QLatin1String(file.readAll()));

		MainWindow mainWindow;

		return app.exec();
	}

