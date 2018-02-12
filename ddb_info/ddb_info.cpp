/* ******************************************************************** **
** @@ DDB_Info
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Dscr   :
** ******************************************************************** */

/* ******************************************************************** **
**                uses pre-compiled headers
** ******************************************************************** */

#include "stdafx.h"

#include "..\shared\file.h"
#include "..\shared\file_find.h"
#include "..\shared\mmf.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef NDEBUG
#pragma optimize("gsy",on)
#pragma comment(linker,"/FILEALIGN:512 /MERGE:.rdata=.text /MERGE:.data=.text /SECTION:.text,EWR /IGNORE:4078")
#endif

/* ******************************************************************** **
** @@                   internal defines
** ******************************************************************** */

/* ******************************************************************** **
** @@ struct
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

#pragma pack(push,1)
struct DDB_HEADER
{
   DWORD             _dw1;
   DWORD             _dw2;
   WORD              _w1;
   char              _pszSignature[18];
   WORD              _w3;
   WORD              _w4;
   BYTE              _by1;
   BYTE              _by2;
   BYTE              _by3;
   BYTE              _by4;
};
#pragma pack(pop)

/* ******************************************************************** **
** @@ struct DDB_SINGLE
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

#pragma pack(push,1)
struct DDB_SINGLE
{
   WORD           _wFileType;
   DWORD          _dwRecCnt;
   DWORD          _dwSize;
};
#pragma pack(pop)

/* ******************************************************************** **
** @@ struct DDB_LIST
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

#pragma pack(push,1)
struct DDB_LIST
{
   DWORD          _dwNext;
   DWORD          _dw1;
   DWORD          _dw2;
   WORD           _wFileType;
   DWORD          _dwRecCnt;
   DWORD          _dwSize;
};
#pragma pack(pop)

/* ******************************************************************** **
** @@                   internal prototypes
** ******************************************************************** */

/* ******************************************************************** **
** @@                   external global variables
** ******************************************************************** */

extern DWORD   dwKeepError = 0;

/* ******************************************************************** **
** @@                   static global variables
** ******************************************************************** */
                  
static MMF        _MF;

static MMF*       _pMF = &_MF;  

static FILE*      _pOut = NULL;

/* ******************************************************************** **
** @@                   real code
** ******************************************************************** */

/* ******************************************************************** **
** @@ PrintSingle()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static void PrintSingle
(
   const BYTE* const    pBuf,
   const BYTE* const    pSingle
)
{
   ASSERT(pBuf);

   DDB_SINGLE*    pBlock = (DDB_SINGLE*)pSingle;

   if (IsBadReadPtr(pBlock,sizeof(DDB_SINGLE)))
   {
      // Error !
      ASSERT(0);
      return;
   }

   fprintf(_pOut,"%04X  ",pBlock->_wFileType);
   fprintf(_pOut,"%08X  ",pBlock->_dwRecCnt);
   fprintf(_pOut,"%08X  ",pBlock->_dwSize);
}

/* ******************************************************************** **
** @@ PrintList()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static void PrintList
(
   const BYTE* const    pBuf,
   const BYTE* const    pList
)
{
   ASSERT(pBuf);

   DDB_LIST*      pBlock = (DDB_LIST*)pList;

   if (IsBadReadPtr(pBlock,sizeof(DDB_LIST)))
   {
      // Error !
      ASSERT(0);
      return;
   }

   fprintf(_pOut,"%08X  ",pBlock->_dwNext);
   fprintf(_pOut,"%08X  ",pBlock->_dw1);
   fprintf(_pOut,"%08X  ",pBlock->_dw2);
   fprintf(_pOut,"%04X  ",pBlock->_wFileType);
   fprintf(_pOut,"%08X  ",pBlock->_dwRecCnt);
   fprintf(_pOut,"%08X  ",pBlock->_dwSize);

   if (pBlock->_dwNext)
   {
      fprintf(_pOut,"\n**                                                             ");

      PrintSingle(pBuf,pBuf + pBlock->_dwNext);
   }
}

/* ******************************************************************** **
** @@ PrintEntry()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static void PrintInfo(const BYTE* const pBuf)
{
   ASSERT(pBuf);

   DWORD    dwStart = *(DWORD*)(pBuf + sizeof(DDB_HEADER));

   if (!dwStart)
   {
      // Smth special !
      return;
   }
   
   if (IsBadReadPtr(pBuf,dwStart))
   {
      // Error !
      ASSERT(0);
      return;
   }

   DWORD    dwSizeEx = dwStart - sizeof(DDB_HEADER) - sizeof(DWORD);

   fprintf(_pOut,"%08X  ",dwStart);
   fprintf(_pOut,"%08X  ",dwSizeEx); // ExData size

   BYTE*    pDataEx = (BYTE*)(pBuf + sizeof(DDB_HEADER) + sizeof(DWORD));

   if (IsBadReadPtr(pDataEx,dwSizeEx))
   {
      // Error !
      ASSERT(0);
      return;
   }

   if (dwSizeEx)
   {
      fprintf(_pOut,"L  ");
      PrintList(pBuf,(BYTE*)pDataEx);
   }
   else
   {
      fprintf(_pOut,"S  ");
      PrintSingle(pBuf,(BYTE*)pDataEx);
   }
}

/* ******************************************************************** **
** @@ ForEach()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static void ForEach(const WIN32_FIND_DATA& w32fd)
{
   ASSERT(_pMF);

   if (!_pMF->OpenReadOnly(w32fd.cFileName))
   {
      perror("\a\nOpen Input File Error !\n");
      return;
   }
   
   BYTE*       pBuf   = _pMF->Buffer();
   DWORD       dwSize = _pMF->Size();

   if (!pBuf || !dwSize || (dwSize < sizeof(DDB_HEADER)))
   {
      // Error !
      ASSERT(0);
      return;
   }

   if (IsBadReadPtr(pBuf,sizeof(DDB_HEADER)))
   {
      // Error !
      ASSERT(0);
      return;
   }

   fprintf(_pOut,"%08X  ",dwSize);

   PrintInfo(pBuf);

   fprintf(_pOut,"**  %s\n",w32fd.cFileName);

   _pMF->Close();
}

/* ******************************************************************** **
** @@ ShowHelp()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static void ShowHelp()
{
   const char  pszCopyright[] = "-*-   DDB_Info 1.0   *   (c)   Gazlan, 2015   -*-";
   const char  pszDescript [] = "DDB files header printer";
   const char  pszE_Mail   [] = "complains_n_suggestions direct to gazlan@yandex.ru";

   printf("%s\n\n",pszCopyright);
   printf("%s\n\n",pszDescript);
   printf("Usage: ddb_info.com [wildcards]\n\n");
   printf("%s\n\n",pszE_Mail);
}

/* ******************************************************************** **
** @@ main()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

int main(int argc,char** argv)  
{
   if (argc != 2)
   {
      ShowHelp();
      return 0;
   }

   if (argc == 2)
   {
      if ((!strcmp(argv[1],"?")) || (!strcmp(argv[1],"/?")) || (!strcmp(argv[1],"-?")) || (!stricmp(argv[1],"/h")) || (!stricmp(argv[1],"-h")))
      {
         ShowHelp();
         return 0;
      }
   }

   FindFile*   pFF = new FindFile;

   if (!pFF)
   {
      return -1;
   }

   if (argc == 2)
   {
      if ((!strcmp(argv[1],"?")) || (!strcmp(argv[1],"/?")) || (!strcmp(argv[1],"-?")) || (!stricmp(argv[1],"/h")) || (!stricmp(argv[1],"-h")))
      {
         ShowHelp();
         return 0;
      }

      pFF->SetMask(argv[1]);
   }
   else
   {
      pFF->SetMask("*.*");
   }

   _pOut = fopen("ddb_info.txt","wt");

   if (!_pOut)
   {
      // Error !
      ASSERT(0);
      return 0;
   }
   
   while (pFF->Fetch())
   {
      if ((pFF->_w32fd.dwFileAttributes | FILE_ATTRIBUTE_NORMAL) && !(pFF->_w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      {
         ForEach(pFF->_w32fd);
      }
   }

   fclose(_pOut);
   _pOut = NULL;

   delete pFF;
   pFF = NULL;

   return 0;
}