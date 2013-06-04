#pragma once

#include <QtGui>
#include <QAction>

#define SECTOR_SIZE 512

//#include "c:\qt\4.7.1\src\gui\widgets\qmainwindow.h"

class Notepad : public QWidget
         {
             Q_OBJECT

         public:
  		 QTextEdit *textEdit;
			 QProgressDialog *dataConvProgress;

             Notepad();

			 bool readingCancelled() { return cancelledRead; }
			 void SetCancelRead(bool b) { cancelledRead = b; }

         signals:
			 void startReadingSD(QString *dp, Notepad *np);

         private slots:
             void open();
             void save();
			 void read();
			 void readAll();
			 void erase();
			 
			 //for receiving status and progress report from worker thread
			 void revInitDataRead(unsigned long numSectors);
			 void revProgress(unsigned long finishedSectors); //currently this is only a dummy function since the progress bar is updated in the worker thread
			 void revStatus(QString str);
			 void revData(QString str);

         private:
			 bool isZeroSector(unsigned long long StartAdd, int SectorSize); //this is to support the fast format (not implemented yet)
			 void readSectors(QString &DrivePath);
			 void readSectorsAllInWorker(QString *DrivePath);
			 //void readSectorsAll(QString &DrivePath);
             //QTextEdit *textEdit;

			 bool cancelledRead;

			 QTextEdit *textEditL;
             QAction *openAction;
             QAction *saveAction;
             QAction *exitAction;
			 QAction *readAction;
			 QAction *readAllAction;

			 QAction *eraseAction;
			 QFileSystemModel *model;
			 QTreeView *tree;

			 //QProgressDialog *dataConvProgress;

             QMenu *fileMenu;
			 QMenuBar *menuBar;

			 QString selectedDrive;
			 
         };

