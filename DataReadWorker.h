#pragma once

#include <QtGui>
#include <QAction>
#include "NotePad.h"


class DataReadWorker : public QObject 
      {
         Q_OBJECT

       public:
		   DataReadWorker()
		   {
			   drivePath = NULL;
			   notepad = NULL;
			   
		   };

		   DataReadWorker(QString *dp, Notepad *np)
		   {
			   drivePath = dp;
			   notepad = np;
			 //  dataConvProgress = dcp;
			 //  textEdit = tt;
		   };

	   private slots:

		   void readSectorsAll(QString *dp, Notepad *np);

       signals:

		   void initDataRead(unsigned long numSectors);
		   void reportStatus(QString str);
		   void reportProgress(unsigned long finishedSectors);
		   void reportData(QString str);

	   private:
		   QString *drivePath;
		   Notepad *notepad;
		   //QProgressDialog *dataConvProgress;
		   //QTextEdit *textEdit;
	 
	   //protected:
	//	   void run();

};
