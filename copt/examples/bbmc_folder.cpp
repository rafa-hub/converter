//example of batch tests over all files in a folder: only in WINDOWS
//<bin> <param_1> [param_2] in which param_1 is TARGET FOLDER and param_2
//The simplest way to use it is to open a SHELL in the target folder and run:
// binary_name . ..\\log.txt  (note the log file should not be in the same folder as the target file)
//author: pablo san segundo
//date: 1/10/2015

#ifdef WIN32

#include <iostream>
#include <sstream>
#include <string>
#include <windows.h>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#include <stdlib.h>
#include <cstdlib>

#pragma comment(lib, "User32.lib")

#include "../batch/batch_benchmark.h"
//#include "../batch/batch_gen.h"
#include "../batch/benchmark_clique.h"
#include "utils/file.h"
#include "../clique/clique_sat.h"

using namespace std;


//Clique Params
#define INIT_ORDER				clqo::MIN_WIDTH
#define TIMEOUT_CLIQUE			300

void DisplayErrorBox(LPTSTR lpszFunction) { 
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


void main(int argc, char **argv){

   WIN32_FIND_DATA ffd;
   LARGE_INTEGER filesize;
   TCHAR szDir[MAX_PATH];
   size_t length_of_arg;
   HANDLE hFind = INVALID_HANDLE_VALUE;
   DWORD dwError=0;

   // If the directory is not specified as a command-line argument,
   // print usage.

   if(argc!=3) {
      _tprintf(TEXT("Introduzca <PATH_FOLDER> <LOG FILE>: %s <directory name>\n"), argv[0]);
	  return;
   }

  //determine logfile (default or parameter)
  string logfile(argv[2]);
   
   // Check that the input path plus 3 is not longer than MAX_PATH.
   // Three characters are for the "\*" plus NULL appended below.

   StringCchLength(argv[1], MAX_PATH, &length_of_arg);

   if (length_of_arg > (MAX_PATH - 3))
   {
      _tprintf(TEXT("\nDirectory path is too long.\n"));
	  return;
   }

   _tprintf(TEXT("\nTarget directory is %s\n\n"), argv[1]);

   // Prepare string for use with FindFile functions.  First, copy the
   // string to a buffer, then append '\*' to the directory name.

   StringCchCopy(szDir, MAX_PATH, argv[1]);
   StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

   // Find the first file in the directory.
   hFind = FindFirstFile(szDir, &ffd);

   if (INVALID_HANDLE_VALUE == hFind) 
   {
      DisplayErrorBox(TEXT("FindFirstFile"));
	  return;
   } 
     
   //parameters fixed for all tests
   BatchCLQBk<ugraph, Clique<ugraph> > batch;
   clqo::param_t p;
   p.tout=TIMEOUT_CLIQUE;								
   p.unrolled=false;
   p.init_order=INIT_ORDER;

   do
   {
      if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
         _tprintf(TEXT("  %s   <DIR>\n"), ffd.cFileName);
      }
      else {
         filesize.LowPart = ffd.nFileSizeLow;
         filesize.HighPart = ffd.nFileSizeHigh;
         _tprintf(TEXT("  %s   %ld bytes\n"), ffd.cFileName, filesize.QuadPart);
		 char output[256];
		 sprintf(output, "%s\\%s",argv[1], ffd.cFileName);
		

		 //configure parameters for all tests
		batch.add_test<Clique<ugraph>>(p);
		string filename(argv[1]);
		batch.run_single_instance(filename +"\\" + ffd.cFileName, FILE_LOG(logfile.c_str(), APPEND));
		com::stl::print_collection<vint>( batch.get_test(0)->decode_first_solution(), FILE_LOG(logfile.c_str(), APPEND)) ;
		ofstream fs(logfile.c_str(),ios::app);
		fs<<endl<<endl;

		batch.clear();

		cout<<"-----------------------------------------------------------------------"<<endl;
		
      }
   }
   while (FindNextFile(hFind, &ffd) != 0);  //main loop

   //error 
   dwError = GetLastError();
   if (dwError != ERROR_NO_MORE_FILES) 
   {
      DisplayErrorBox(TEXT("FindFirstFile"));
   }

   FindClose(hFind);

}

#else 
//posix
int main(){
	return 0;
}
#endif 








