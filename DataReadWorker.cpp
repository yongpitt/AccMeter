
#include "stdafx.h"
#include "NotePad.h"
#include "DataReadWorker.h"

//void DataReadWorker::run()
//{
  //readSectorsAll(*drivePath, *notepad);
//}

void DataReadWorker::readSectorsAll(QString *dp, Notepad *np)
{
	 
	 bool readSectorsDone = false;
	 HANDLE hDevice;               // handle to the drive to be examined
	 PCWSTR driveName;

	 GET_LENGTH_INFORMATION numberBytes;
	 
	 DWORD bytesReturned;
	 DWORD bytesRead;

	 char *readStr;
	 unsigned char *buffer;

	 short int x,y,z;

	 int zeroBytes = 0;

	 unsigned long BufferSize = 512;

	 unsigned long long numBytes = 0;
	 unsigned long long SDi = 0;
	 unsigned long numSectors = 0;
	 unsigned long validSectors = 1;
	 unsigned long testOffset = 0;
	 QString dirPath = QString("\\\\.\\") + *dp;
	 QString dataStr;

	 std::string dirStr = std::string(dirPath.toAscii().data());
	 const char *dirChar = dirStr.c_str();
	 
     WCHAR    tmpStr[6];
	 
     MultiByteToWideChar(0,0,dirChar, -1, tmpStr, 7);
	 tmpStr[6] = '\0';

     LPCWSTR mydir = tmpStr;

	 qDebug() << "work thread ID: " << this->thread()->currentThreadId();

	 //LPCWSTR test =	TEXT("\\\\.\\H:");

	 //the next two lines have been removed to the Notepad::revStatus() slot. Instead, reportStatus signal is placed here since 
	 //the worker thread can not maintain the widgets in the main GUI thread
	 //np->textEdit->append("open the device...\n");
	 //np->textEdit->repaint();

	 emit reportStatus("open the device...\n");

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
		 
         QMessageBox::critical(np, tr("Error"),
                     tr("Could not open file!"));
                 return;
     }

	if(!DeviceIoControl(hDevice, IOCTL_DISK_GET_LENGTH_INFO, NULL, 0, &numberBytes, sizeof(GET_LENGTH_INFORMATION), &bytesReturned, (LPOVERLAPPED) NULL)) 
	{
         QMessageBox::critical(np, tr("Error"),
                     tr("Could not read configuration of the disk!"));
                 return;
    }

	numBytes = unsigned long long(numberBytes.Length.QuadPart);

	numSectors = unsigned long(numBytes >> BYTES_PER_SECTOR_IN_BITS); 

	buffer = new unsigned char[BufferSize];
	
	//the next two lines have been removed to the Notepad::revStatus() slot. Instead, reportStatus signal is placed here since 
	//the worker thread can not maintain the widgets in the main GUI thread
	//np->textEdit->append(QString("reading raw data from the selected device: ") + *dp + QString(" ...\n"));
	//np->textEdit->repaint();
	emit reportStatus(QString("Reading raw data from the selected device: ") + *dp + QString(" ...\n"));
	
	while(validSectors < numSectors)
	{

		SetFilePointer (hDevice, validSectors*512, NULL, FILE_BEGIN); 
		if (!ReadFile (hDevice, buffer, BufferSize, &bytesRead, NULL) )
        {
           QMessageBox::critical(np, tr("Error"),
                    tr("Fail to read file!"));
                return;
         }
		//if the first 9 bytes are not all zero, double the valideSector and go next iteration
		if(buffer[0] || buffer[1] || buffer[2] || buffer[3] || buffer[4] || buffer[5] || buffer[6] || buffer[7] || buffer[8])
			validSectors = validSectors*2;
		else
			break;

	}

	if(validSectors > numSectors)
		validSectors = numSectors;
	
	SetFilePointer (hDevice, NULL, NULL, FILE_BEGIN); 

	unsigned long si = 0;

	//the next two lines have been removed to the Notepad::revStatus() slot. Instead, reportStatus signal is placed here
	//np->textEdit->append("converting the raw data...\n");
    //np->textEdit->repaint();
	emit reportStatus("Converting the raw data...\n");

	//the next two lines have been removed to the Notepad::revProgress() slot. Instead, initDataRead signal is placed here
	//np->dataConvProgress = new QProgressDialog("converting raw data...", "abort converting", 1, numSectors, np); 
    //np->dataConvProgress->setWindowModality(Qt::WindowModal);
	emit initDataRead(validSectors);
	emit reportProgress(1);

	np->SetCancelRead(false);

	while(si < validSectors)
	{
		if (!ReadFile (hDevice, buffer, BufferSize, &bytesRead, NULL) )
		{
			QMessageBox::critical(np, tr("Error"),
						tr("Fail to read file!"));
					return;
		} 

		SDi = 0;

		//dataStr.clear();

		zeroBytes = 0;

		while(SDi < BufferSize)
		{
			//SDi += 8;
			if(SDi == 0)
				SDi += 8;
			
			x = (short int) ((buffer[SDi+1]<<8) | buffer[SDi]);
			y = (short int) ((buffer[SDi+3]<<8) | buffer[SDi+2]);
			z = (short int) ((buffer[SDi+5]<<8) | buffer[SDi+4]);
			SDi += 6;
		
			//skip the last two bytes in the sector
			 

			//remove bad data
			/*
			if(x<-4096 || x>4096 || y<-4096 || y>4096 || z<-4096 || z>4096 )
			{
				x = y = z = -1;
			}
			*/
			//replace this by the reportData signal
			//np->textEdit->append(QString::number(x) + QString(" ") + QString::number(y) + QString(" ") + QString::number(z) + QString(" "));

			dataStr = dataStr + QString::number(x) + QString(" ") + QString::number(y) + QString(" ") + QString::number(z) + QString(" \n");

			if(x == 0 && y==0 && z == 0)
			{
			   zeroBytes += 3;
			}
			else
			   zeroBytes = 0;

			//if zeroBytes > 9, stop reading data
			if(zeroBytes > 9)
			{
				readSectorsDone = true;
			    break;
			}
		}

		
		//this greatly affects the responsiveness of the GUI
		if(si % 1024 == 0)
		{
			emit reportData(dataStr);
		    dataStr.clear();
		}
		if(validSectors < 128)
		{
			emit reportProgress(si);
		}
		else if(si % (int(validSectors/64)) == 0)
		{	
			emit reportProgress(si);
		}

		if(np->readingCancelled())
            break;
	
		if(readSectorsDone)
			break;

		si++;

	}

	//np->dataConvProgress->setValue(numSectors);

	if(!dataStr.isEmpty())
		emit reportData(dataStr);

	emit reportProgress(validSectors);

	CloseHandle(hDevice);

	delete[] buffer;

	return;
}
