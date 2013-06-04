#include "stdafx.h"
//#include "sdqt.h"
#include "Notepad.h"
#include "DataReadWorker.h"
#include "WorkerThread.h"

int main(int argv, char **args)
{
    QApplication app(argv, args);

    //QTextEdit textEdit;
    //QPushButton quitButton("Quit");

    //QObject::connect(&quitButton, SIGNAL(clicked()), qApp, SLOT(quit()));

    //QVBoxLayout layout;
  
    //layout.addWidget(&textEdit);
    //layout.addWidget(&quitButton);

    //QWidget window;
    //window.setLayout(&layout);

    //window.show();

	qDebug() << "main thread ID: " << app.thread()->currentThreadId();


	WorkerThread WT;

	WT.start();

	DataReadWorker *SDreader = new DataReadWorker();

	//SDreader->moveToThread(&WT);
	WT.lanchWorker(SDreader);

	

	Notepad myNotepad;
	
	 

	QObject::connect(&myNotepad, SIGNAL(startReadingSD(QString *, Notepad *)), SDreader, SLOT(readSectorsAll(QString *, Notepad *)), Qt::QueuedConnection);
	
	QObject::connect(SDreader, SIGNAL(initDataRead(unsigned long)), &myNotepad, SLOT(revInitDataRead(unsigned long)), Qt::QueuedConnection);
	
	QObject::connect(SDreader, SIGNAL(reportStatus(QString)), &myNotepad, SLOT(revStatus(QString)), Qt::QueuedConnection);

	QObject::connect(SDreader, SIGNAL(reportProgress(unsigned long)), &myNotepad, SLOT(revProgress(unsigned long)), Qt::QueuedConnection);

	QObject::connect(SDreader, SIGNAL(reportData(QString)), &myNotepad, SLOT(revData(QString)), Qt::QueuedConnection);

	//QMetaObject::invokeMethod(SDreader, "readSectorsAll", Qt::QueuedConnection);
	//QObject::connect(SDreader, SIGNAL(reportProgress(unsigned long)), &myNotepad, SLOT(revProgress(unsigned long)), Qt::QueuedConnection);

	//SDreader.start();

	myNotepad.show();

    app.exec();

	WT.quit();

	WT.wait();

	return 0;
}
