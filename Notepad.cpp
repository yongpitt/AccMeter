
#include "stdafx.h"
#include <string.h>
#include "NotePad.h"
#include "DataReadWorker.h"
//#using <mscorlib.dll>
//using namespace System;
//using namespace System::IO;
//using namespace System::Text;

Notepad::Notepad(void)
{            menuBar = new QMenuBar;

             fileMenu = new QMenu(tr("&File"), this);
           
			 openAction = new QAction(tr("&Open"), this);
             saveAction = new QAction(tr("&Save"), this);
             exitAction = new QAction(tr("&Exit"), this);
			 readAction = new QAction(tr("&Read Raw Data"), this);
			 readAllAction = new QAction(tr("&Read All Raw Data"), this);
			 eraseAction = new QAction(tr("&Erase SD card"), this);

             connect(openAction, SIGNAL(triggered()), this, SLOT(open()));
             connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));
             connect(exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
			 

           //  fileMenu = menuBar()->addMenu(tr("&File"));
             fileMenu->addAction(openAction);
             fileMenu->addAction(saveAction);
             fileMenu->addSeparator();
             fileMenu->addAction(exitAction);

			 menuBar->addMenu(fileMenu);

			 QHBoxLayout * mainlayout = new QHBoxLayout;
			 QVBoxLayout * mainAndMenulayout = new QVBoxLayout;

	         model = new QFileSystemModel;
             model->setRootPath(QDir::currentPath());
			 tree = new QTreeView();
             tree->setModel(model);
			 
			 tree->addAction(readAction);
			 tree->addAction(readAllAction);
			 tree->addAction(eraseAction);
             tree->setContextMenuPolicy(Qt::ActionsContextMenu);

			 connect(readAction, SIGNAL(triggered()), this, SLOT(read()));
			 connect(readAllAction, SIGNAL(triggered()), this, SLOT(readAll()));
			 connect(eraseAction, SIGNAL(triggered()), this, SLOT(erase()));

			 QGroupBox *horizontalGroupBox = new QGroupBox;
             textEdit = new QTextEdit;
			 textEditL = new QTextEdit;

			 textEdit->addAction(saveAction);
			 textEdit->setContextMenuPolicy(Qt::ActionsContextMenu);

			 mainlayout->addWidget(tree);
			 //  mainlayout->addWidget(textEditL);  // this is for test
			 mainlayout->addWidget(textEdit,1);
             //setCentralWidget(textEdit);  // this is only used when the class is inherited from the QMainWindow class
			 horizontalGroupBox->setLayout(mainlayout);

			 mainAndMenulayout->addWidget(menuBar);
			 mainAndMenulayout->addWidget(horizontalGroupBox);

			 setLayout(mainAndMenulayout);

             setWindowTitle(tr("SD Card Management Tool"));
			 this->resize(600,600);
}

void Notepad::read(void)
{
	QString selectedDrive = model->data(tree->currentIndex()).toString();
	//textEdit->append(selectedDrive);
	//QString dirPath = QString("\\\\.\\") + selectedDrive;
	readSectors(selectedDrive);
	return;
}

void Notepad::readAll(void)
{
	selectedDrive = model->data(tree->currentIndex()).toString();
	//textEdit->append(selectedDrive);
	//QString dirPath = QString("\\\\.\\") + selectedDrive;
	readSectorsAllInWorker(&selectedDrive);
	return;
}

void Notepad::open(void)
{
	 QString dirName = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
		 "", QFileDialog::ShowDirsOnly);

	 //QString dirPath = QString("\\\\.\\") + dirName;
	 readSectors(dirName);
	 return;
}

void Notepad::readSectorsAllInWorker(QString *DrivePath)
{
    // DataReadWorker	
	//SDreader.start();
	//SDreader->start();
	emit startReadingSD(DrivePath, this);
	//SDreader.wait();

	return;
}

void Notepad::revInitDataRead(unsigned long numSectors)
{
	dataConvProgress = new QProgressDialog("converting raw data...", "abort converting", 1, numSectors, this); 
    dataConvProgress->setWindowModality(Qt::WindowModal);
	return;
}

void Notepad::revStatus(QString str)
{
	textEdit->append(str);
    textEdit->repaint();
	return;
}

void Notepad::revProgress(unsigned long finishedSectors)
{
	dataConvProgress->setValue(finishedSectors);
	if (dataConvProgress->wasCanceled())
	   this->SetCancelRead(true);
    textEdit->repaint();
	return;
}

void Notepad::revData(QString str)
{
	textEdit->append(str);
    //textEdit->repaint();
	return;
}

void Notepad::readSectors(QString &DrivePath)
{
	 HANDLE hDevice;               // handle to the drive to be examined
	 PCWSTR driveName;

	 GET_LENGTH_INFORMATION numberBytes;
	 
	 DWORD bytesReturned;
	 DWORD bytesRead;

	 char *readStr;
	 unsigned char *buffer;

	 short int x,y,z;

	 int zeroBytes = 0;

	 unsigned long maxBufferSize = 512*65535;

	 unsigned long long numBytes = 0;
	 unsigned long long SDi = 0;
	 unsigned long numSectors = 0;

	 QString dirPath = QString("\\\\.\\") + DrivePath;
	 
	 std::string dirStr = std::string(dirPath.toAscii().data());
	 const char *dirChar = dirStr.c_str();
	 
     WCHAR    tmpStr[6];
	 
     MultiByteToWideChar(0,0,dirChar, -1, tmpStr, 7);
	 tmpStr[6] = '\0';

     LPCWSTR mydir = tmpStr;
	 //LPCWSTR test =	TEXT("\\\\.\\H:");
     textEdit->append("open the device...\n");
	 textEdit->repaint();
	 hDevice = CreateFile(tmpStr,  // drive to open   hDevice = CreateFile(TEXT("\\\\.\\g:"),
		            GENERIC_READ,
                                    // read access to the drive
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,             // default security attributes
                    OPEN_EXISTING,    // disposition
                    FILE_FLAG_NO_BUFFERING,                // file attributes
                    NULL);            // do not copy file attributes
	 if (hDevice == INVALID_HANDLE_VALUE) // cannot open the drive
     {
         QMessageBox::critical(this, tr("Error"),
                     tr("Could not open file!"));
                 return;
     }

	if(!DeviceIoControl(hDevice, IOCTL_DISK_GET_LENGTH_INFO, NULL, 0, &numberBytes, sizeof(GET_LENGTH_INFORMATION), &bytesReturned, (LPOVERLAPPED) NULL)) 
	{
         QMessageBox::critical(this, tr("Error"),
                     tr("Could not read configuration of the disk!"));
                 return;
    }

	numBytes = unsigned long long(numberBytes.Length.QuadPart);

	numSectors = unsigned long(numBytes >> BYTES_PER_SECTOR_IN_BITS); 

	buffer = new unsigned char[maxBufferSize];
	
	textEdit->append(QString("reading raw data from the selected device: ") + DrivePath + QString(" ...\n"));
	textEdit->repaint();
	if (!ReadFile (hDevice, buffer, maxBufferSize, &bytesRead, NULL) )
    {
        QMessageBox::critical(this, tr("Error"),
                    tr("Fail to read file!"));
                return;
    } 

	textEdit->append("converting the raw data...\n");
	textEdit->repaint();
	QByteArray data = QByteArray::fromRawData((const char *)buffer, 512*numSectors);
	
	SDi = 0;

	unsigned long validBytes = maxBufferSize/2;
	while(validBytes > 3)
	{
	  if(buffer[validBytes] == 0 && buffer[validBytes-1] == 0 && buffer[validBytes-2] == 0 && buffer[validBytes+1] == 0 && buffer[validBytes+2] == 0 )
		  validBytes = validBytes/2;
	  else
	  {
		  validBytes = 2*validBytes;
		  break;
	  }
	}
	QProgressDialog dataConvProgress("converting raw data...", "abort converting", 0, validBytes, this); 
	dataConvProgress.setWindowModality(Qt::WindowModal);


	while(SDi <= numBytes-6)
	{

		x = (short int) ((buffer[SDi+1]<<8) | buffer[SDi]);
		y = (short int) ((buffer[SDi+3]<<8) | buffer[SDi+2]);
		z = (short int) ((buffer[SDi+5]<<8) | buffer[SDi+4]);
		SDi += 6;
		
		//skip the last two bytes in a sector
		if(SDi%512 == 510)
			SDi += 2;
		
		if(SDi % 5120 == 0)
		{
		   dataConvProgress.setValue(SDi);
		   if (dataConvProgress.wasCanceled())
			break;
		}
		//remove bad data
		
		if(x<-4096 || x>4096 || y<-4096 || y>4096 || z<-4096 || z>4096 )
		{
			x = y = z = -1;
		}
		

		textEdit->append(QString::number(x) + QString(" ") + QString::number(y) + QString(" ") + QString::number(z) + QString(" "));

		if(x == 0 && y==0 && z == 0)
		{
           zeroBytes += 3;
		}
		else
		   zeroBytes = 0;

		//if zeroBytes > 64, probably there are all zeros thereafter, stop reading data
		if(zeroBytes > 64)
			break;
	}

	dataConvProgress.setValue(validBytes);

	CloseHandle(hDevice);

	delete[] buffer;

	return;
}

void Notepad::save(void)
{
	 QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "",
             tr("Text Files (*.txt);;C++ Files (*.cpp *.h)"));

         if (fileName != "") {
             QFile file(fileName);
             if (!file.open(QIODevice::WriteOnly)) {
                 // error message
             } else {
                 QTextStream stream(&file);
                 stream << textEdit->toPlainText();
                 stream.flush();
                 file.close();
             }
         }
    return;
}




void Notepad::erase()
{
	 

	 HANDLE hDevice;               // handle to the drive to be examined
	 PCWSTR driveName;
//	 System::IO::FileStream fs;
	 GET_LENGTH_INFORMATION numberBytes;
	 
	 DWORD bytesReturned;
	 DWORD bytesWritten;

	 OVERLAPPED overlap;

	 char *readStr;
	 unsigned char *buffer;

	 short int x,y,z;

	 int zeroBytes = 0;

	 //erase 512000 bytes once
	 unsigned long BufferSize = 5120000;

	 unsigned long long numBytes = 0;
	 unsigned long long currSector = 0;
	 unsigned long numSectors = 0;
	 
	 QMessageBox userConfirm;
	 userConfirm.setText("This is going to erase data on a selected drive.");
	 userConfirm.setInformativeText("Are you sure you want to perform the erase operation?");
	 userConfirm.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	 userConfirm.setDefaultButton(QMessageBox::No);
	 int ret = userConfirm.exec();

	 switch (ret) 
	 {
	   case QMessageBox::Yes:
		   // Yes was clicked
		   break;
	   case QMessageBox::No:
		   // No was clicked
		   return;
	   default:
		   // should never be reached
		   break;
     }

	 QString selectedDrive = model->data(tree->currentIndex()).toString();

	 QString dirPath = QString("\\\\.\\") + selectedDrive;
	 
	 std::string dirStr = std::string(dirPath.toAscii().data());
	 const char *dirChar = dirStr.c_str();
	 
     WCHAR    tmpStr[6];
	 
     MultiByteToWideChar(0,0,dirChar, -1, tmpStr, 7);
	 tmpStr[6] = '\0';

     LPCWSTR mydir = tmpStr;
	 //LPCWSTR test =	TEXT("\\\\.\\H:");
     textEdit->append("open the device...\n");
	 textEdit->repaint();
	 
	 hDevice = CreateFile(tmpStr,  // drive to open   hDevice = CreateFile(TEXT("\\\\.\\g:"),
		            GENERIC_READ | GENERIC_WRITE,
                                    // read access to the drive
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,             // default security attributes
                    OPEN_EXISTING,    // disposition
                    FILE_FLAG_NO_BUFFERING,                // file attributes
                    NULL);  
	 
	if (hDevice == INVALID_HANDLE_VALUE) // cannot open the drive
    {
         QMessageBox::critical(this, tr("Error"),
                     tr("Could not open the device!"));
                 return;
    }
	 
	if(!DeviceIoControl(hDevice, IOCTL_DISK_GET_LENGTH_INFO, NULL, 0, &numberBytes, sizeof(GET_LENGTH_INFORMATION), &bytesReturned, (LPOVERLAPPED) NULL)) 
	{
         QMessageBox::critical(this, tr("Error"),
                     tr("Could not read configuration of the drive!"));
                 return;
    }

	numBytes = unsigned long long(numberBytes.Length.QuadPart);

	numSectors = unsigned long(numBytes >> BYTES_PER_SECTOR_IN_BITS); 
	
	buffer = new unsigned char[BufferSize];

	//initialize the buffer
	for(int i = 0; i<BufferSize; i++)
	{
		buffer[i] = 0;
	}
	
	textEdit->append(QString("erasing the selected device: ") + selectedDrive + QString(" ...\n"));
	textEdit->repaint();

	//erase the selected drive

	QProgressDialog dataConvProgress("erasing the selected drive...", "abort erasing", 0, numSectors/10000, this); 
	dataConvProgress.setWindowModality(Qt::WindowModal);
	
	for(currSector = 0; currSector < numSectors/10000; currSector++)
	{
		
		if (!WriteFile (hDevice, buffer, BufferSize, &bytesWritten, NULL) )
		{
			QMessageBox::critical(this, tr("Error"),
						tr("Fail to write to the device!"));
					return;
		}

		
		   dataConvProgress.setValue(currSector);
		   if (dataConvProgress.wasCanceled())
			break;
	
		
		
	}

	dataConvProgress.setValue(numSectors/10000);

	textEdit->append(QString("erase finished. \n"));
	textEdit->repaint();

	CloseHandle(hDevice);

	delete[] buffer;

	return;
}

bool Notepad::isZeroSector(unsigned long long StartAdd, int SectorSize)
{
	return false;
}
