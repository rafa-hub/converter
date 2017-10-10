//--------------------------------------------
// Test only for windows OS; needs passing the path of the folder as VS argument
// author: alopez
// date: 6/10/114

#ifdef WIN32


#include <iostream>
#include <string>
#include "gtest/gtest.h"
#include <iostream>
#include <string>
#include <windows.h>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#include "time.h"
#include <stdlib.h>
#include <cstdlib>
#include "utils/common.h"
#include "../clique/clique.h"

#pragma comment(lib, "User32.lib")


//#define LOG_FILE  "C:/Users/Pablo/Desktop/biiwsp/res_clique.txt"
#define LOG_FILE  "C:/Users/i7/Desktop/trial_set.txt"

using namespace std;


void DisplayErrorBox(LPTSTR lpszFunction);

TEST(Folder_tests, ugraph){
	WIN32_FIND_DATA ffd;
   LARGE_INTEGER filesize;
   TCHAR szDir[MAX_PATH];
   size_t length_of_arg;
   HANDLE hFind = INVALID_HANDLE_VALUE;
   DWORD dwError=0;

	extern int __argc;
    extern char** __argv;  

   // If the directory is not specified as a command-line argument,
   // print usage.

   if(__argc != 2)
   {
      _tprintf(TEXT("\nUsage: %s <directory name>\n"), __argv[0]);
	  return;
   }

   // Check that the input path plus 3 is not longer than MAX_PATH.
   // Three characters are for the "\*" plus NULL appended below.

   StringCchLength(__argv[1], MAX_PATH, &length_of_arg);

   if (length_of_arg > (MAX_PATH - 3))
   {
      _tprintf(TEXT("\nDirectory path is too long.\n"));
	  return;
   }

   _tprintf(TEXT("\nTarget directory is %s\n\n"), __argv[1]);

   // Prepare string for use with FindFile functions.  First, copy the
   // string to a buffer, then append '\*' to the directory name.

   StringCchCopy(szDir, MAX_PATH, __argv[1]);
   StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

   // Find the first file in the directory.

   hFind = FindFirstFile(szDir, &ffd);

   if (INVALID_HANDLE_VALUE == hFind) 
   {
      DisplayErrorBox(TEXT("FindFirstFile"));
	  return;
   } 
   
   // List all the files in the directory with some info about them.
   SYSTEMTIME timeinfo;
   ofstream fs(LOG_FILE);
   if(!fs){
		cerr<<"log file for tests not opened correctly"<<endl;
		return;
   }
   GetSystemTime(&timeinfo);
   fs<<timeinfo.wYear<<"/"<<timeinfo.wMonth<<"/"<<timeinfo.wDay<<", "<<timeinfo.wHour<<endl;
   fs<<"Nombre               Time"<<endl;
   
   
   do
   {
      if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
         _tprintf(TEXT("  %s   <DIR>\n"), ffd.cFileName);
      }
      else
      {
         filesize.LowPart = ffd.nFileSizeLow;
         filesize.HighPart = ffd.nFileSizeHigh;
         _tprintf(TEXT("  %s   %ld bytes\n"), ffd.cFileName, filesize.QuadPart);
		 char output[256];
		 sprintf(output, "%s\\%s",__argv[1], ffd.cFileName);
		 clock_t t_ini, t_fin;
		 double secs;
		 sparse_ugraph* p_usg = new sparse_ugraph(output);
		 Clique<sparse_ugraph> cusg(p_usg, clqo::param_t());
		 int sol;
		 if((sol=cusg.set_up())>0){
			fs<<ffd.cFileName<<"SOLVED TRIVIALLY:"<<sol<<endl;
			delete p_usg;
			continue;
		 }
		
		cout<<"-----------------------------------------------------------------------"<<endl;
		
		t_ini = clock();
		cusg.run();		
		t_fin = clock();
		secs = (double)(t_fin - t_ini) / CLOCKS_PER_SEC;
		cout<<"ha tardado : "<<secs<<endl;
		fs<<ffd.cFileName<<"      "<<cusg.get_max_clique()<<"       "<<secs<<endl;

        cout<<"clique: "<<cusg.get_max_clique()<<endl;  //y otras funcionalidades
		cout<<"-----------------------------------------------------------------------"<<endl;
		delete p_usg;
      }
   }
   while (FindNextFile(hFind, &ffd) != 0);
 
   dwError = GetLastError();
   if (dwError != ERROR_NO_MORE_FILES) 
   {
      DisplayErrorBox(TEXT("FindFirstFile"));
   }

   FindClose(hFind);
}

void DisplayErrorBox(LPTSTR lpszFunction) 
{ 
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and clean up

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}


#endif