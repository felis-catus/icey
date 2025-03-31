#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "ice.h"

#ifndef _WIN32
#define min(X, Y)  ((X) < (Y) ? (X) : (Y))
#endif /* _WIN32 */

#ifdef _WIN32
#include "winlite.h"
#else
#include <alloca.h>
#include <ctype.h>
#include <dirent.h>
#define MAX_PATH 260
#define _MAX_EXT 256
#endif /* _WIN32 */

#ifdef _WIN32
#define stricmp _stricmp
#endif /* _WIN32 */

#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif /* _WIN32 */

#define KEY_SIZE 8
#define DEFAULT_KEY	"lREeeapA" /* not a secret! */
#define DEFAULT_EXTENSION ".ctx"

/* ----------------------------------------------------------------------------
	Vars
---------------------------------------------------------------------------- */

static char g_szKey[KEY_SIZE + 1];
static char g_szExtension[_MAX_EXT];
#ifndef _WIN32
const char *g_pszInputExtension = NULL;
#endif /* _WIN32 */
int g_bEncrypt;
int g_bDecrypt;
int g_bQuiet;
int g_bNoFill;
char g_chFillChar = '\n'; /* fill remainder with newlines by default */

/* ------------------------------------------------------------------------- */
static int UTIL_IsPathSeparator( const char c )
{
	return c == '\\' || c == '/';
}

/* ------------------------------------------------------------------------- */
static void UTIL_StripExtension( const char *pszIn, char *pszOut, const size_t outSize )
{
	size_t end = strlen( pszIn ) - 1;
	while ( end > 0 && pszIn[end] != '.' && !UTIL_IsPathSeparator( pszIn[end] ) )
	{
		--end;
	}

	if ( end > 0 && !UTIL_IsPathSeparator( pszIn[end] ) && end < outSize )
	{
		const size_t nChars = min( end, outSize - 1 );
		if ( pszOut != pszIn )
		{
			memcpy( pszOut, pszIn, nChars );
		}

		pszOut[nChars] = 0;
	}
	else
	{
		if ( pszOut != pszIn )
		{
			strncpy( pszOut, pszIn, outSize );
		}
	}
}

/* ------------------------------------------------------------------------- */
static long UTIL_GetFileSize( FILE *pFile )
{
	long fileSize;
	fseek( pFile, 0, SEEK_END );
	fileSize = ftell( pFile );
	fseek( pFile, 0, SEEK_SET );
	return fileSize;
}

#ifndef _WIN32
/* ------------------------------------------------------------------------- */
int stricmp( char const *a, char const *b )
{
	for ( ;; a++, b++ )
	{
		int d = tolower( (unsigned char)*a ) - tolower( (unsigned char)*b );
		if ( d != 0 || !*a )
		{
			return d;
		}
	}
}

/* ------------------------------------------------------------------------- */
int extension_filter( const struct dirent *dir )
{
	if ( !dir )
		return 0;

	if ( dir->d_type == DT_REG ) /* Only deal with regular files */
	{
		const char *ext = strrchr( dir->d_name, '.' );
		if ( !ext || ext == dir->d_name )
		{
			return 0;
		}
		else if ( stricmp( ext, g_pszInputExtension ) == 0 )
		{
			return 1;
		}
	}

	return 0;
}
#endif /* _WIN32 */

/* ------------------------------------------------------------------------- */
static int ProcessFile( const char *pszFileName )
{
	ICE_KEY *pKey;
	FILE *pFile;
	long fileSize;
	int blockSize;
	int remainder;
	int bFill;
	long bufferSize;
	unsigned char *pInBuf;
	unsigned char *pOutBuf;
	int bytesLeft;
	const unsigned char *pIn;
	unsigned char *pOut;
	char szOutName[MAX_PATH];

	pFile = fopen( pszFileName, "rb" );
	if ( pFile == NULL )
	{
		fprintf( stderr, "couldn't open %s for input\n", pszFileName );
		return 0;
	}

	pKey = ice_key_create( 0 );
	ice_key_set( pKey, (unsigned char *)g_szKey );

	fileSize = UTIL_GetFileSize( pFile );
	blockSize = ice_key_block_size( pKey );
	remainder = fileSize % blockSize;

	if ( g_bNoFill && remainder != 0 )
	{
		fprintf( stderr, "warning: %d bytes of unencrypted data leaked due to -nofill.\n", remainder );
	}

	bFill = ( !g_bNoFill && remainder != 0 ) ? 1 : 0;
	bufferSize = fileSize + ( bFill ? ( blockSize - remainder ) : 0 );

	pInBuf = alloca( bufferSize );
	pOutBuf = alloca( bufferSize );

	memset( pInBuf, 0, bufferSize );
	memset( pOutBuf, 0, bufferSize );

	fread( pInBuf, bufferSize, 1, pFile );
	fclose( pFile );

	if ( bFill )
	{
		const int endPos = ( fileSize + 1 ) - 1;
		int i = endPos;
		for ( ; i < bufferSize; ++i )
		{
			pInBuf[i] = g_chFillChar;
		}
	}

	bytesLeft = bufferSize;

	pIn = pInBuf;
	pOut = pOutBuf;

	while ( bytesLeft >= blockSize )
	{
		if ( g_bEncrypt )
		{
			ice_key_encrypt( pKey, pIn, pOut );
		}
		else if ( g_bDecrypt )
		{
			ice_key_decrypt( pKey, pIn, pOut );
		}

		bytesLeft -= blockSize;
		pIn += blockSize;
		pOut += blockSize;
	}

	memcpy( pOut, pIn, bytesLeft );

	UTIL_StripExtension( pszFileName, szOutName, MAX_PATH );
	strncat( szOutName, g_szExtension, MAX_PATH - strlen( szOutName ) );

	pFile = fopen( szOutName, "wb" );
	if ( pFile == NULL )
	{
		fprintf( stderr, "couldn't open %s for output\n", szOutName );
		return 0;
	}

	fwrite( pOutBuf, bufferSize, 1, pFile );
	fclose( pFile );

	if ( !g_bQuiet ) 
		fprintf( stdout, "handled file %s\n", pszFileName );

	ice_key_destroy( pKey );
	return 1;
}

/* ------------------------------------------------------------------------- */
int main( const int argc, char *argv[] )
{
	int i;
	const char *pszExtension = DEFAULT_EXTENSION;
	char szExtension[_MAX_EXT];
	char szSearch[1024];

#ifdef _WIN32
	WIN32_FIND_DATAA file;
	HANDLE hFind;
	int bFound;
#else
	struct dirent **nameList;
	int dirIdx;
#endif /* _WIN32 */

	szExtension[0] = '\0';

	g_bQuiet = 0;

	if ( argc < 2 )
	{
		const char *pszHelp =
			"usage: icey [-encrypt] [-decrypt] [-quiet] [-key abcdefgh] [-extension .ctx] file file2 ...\n\n"
			"-encrypt | -e : encrypt files (default)\n"
			"-decrypt | -d : decrypt files\n"
			"-key | -k : key, must be 8 chars\n"
			"-extension | -x : file extension for output\n"
			"-quiet | -q : don't print anything (excl. errors)\n"
			"-nofill : don't fill remainder with blank bytes (warning, this will leak unencrypted data)\n\n"
			"e.g.\n"
			"icey -encrypt -key lREeeapA -extension .ctx file.txt\n"
			"icey -x .ctx -k lREeeapA *.txt\n";

#ifdef _WIN32
		FreeConsole();
		MessageBoxA( NULL, pszHelp, "icey - yet another ICE encryption tool", MB_OK );
#else
		fprintf( stderr, "%s", pszHelp );
#endif /* _WIN32 */

		return EXIT_SUCCESS;
	}

	g_szKey[0] = '\0';
	g_szExtension[0] = '\0';

	g_bEncrypt = 1; /* assume encrypt by default */

	i = 1;
	while ( i < argc )
	{
		if ( stricmp( argv[i], "-e" ) == 0 || stricmp( argv[i], "-encrypt" ) == 0 )
		{
			g_bEncrypt = 1;
			g_bDecrypt = 0;
			++i;
		}
		else if ( stricmp( argv[i], "-d" ) == 0 || stricmp( argv[i], "-decrypt" ) == 0 )
		{
			g_bDecrypt = 1;
			g_bEncrypt = 0;
			++i;
		}
		else if ( stricmp( argv[i], "-k" ) == 0 || stricmp( argv[i], "-key" ) == 0 )
		{
			++i;
			if ( strlen( argv[i] ) != KEY_SIZE )
			{
				fprintf( stderr, "error: ICE key must be 8 char text!\n" );
				return EXIT_FAILURE;
			}

			strncpy( g_szKey, argv[i], sizeof( g_szKey ) );
			++i;
		}
		else if ( stricmp( argv[i], "-x" ) == 0 || stricmp( argv[i], "-extension" ) == 0 )
		{
			++i;
			if ( strlen( argv[i] ) < 1 || *argv[i] != '.' )
			{
				fprintf( stderr, "error: bad extension.\n" );
				return EXIT_FAILURE;
			}

			strncpy( g_szExtension, argv[i], sizeof( g_szExtension ) );
			++i;
		}
		else if ( stricmp( argv[i], "-q" ) == 0 || stricmp( argv[i], "-quiet" ) == 0 )
		{
			g_bQuiet = 1;
			++i;
		}
		else if ( stricmp( argv[i], "-nofill" ) == 0 )
		{
			fprintf( stderr, "-nofill\n" );
			g_bNoFill = 1;
			++i;
		}
		else
		{
			break;
		}
	}

	if ( i >= argc )
	{
		fprintf( stderr, "error: no files in cmd line.\n" );
		return EXIT_FAILURE;
	}

	if ( g_szKey[0] == '\0' )
	{
		if ( !g_bQuiet ) 
			fprintf( stdout, "no key, using default (%s)\n", DEFAULT_KEY );

		strncpy( g_szKey, DEFAULT_KEY, sizeof( g_szKey ) );
	}

	if ( g_szExtension[0] == '\0' )
	{
		if ( !g_bQuiet ) 
			fprintf( stdout, "no extension, using default (%s)\n", DEFAULT_EXTENSION );

		strncpy( g_szExtension, DEFAULT_EXTENSION, sizeof( g_szExtension ) );
	}

	for ( ; i < argc; ++i )
	{
		const char *pszFileName = argv[i];

		if ( strstr( pszFileName, "*." ) )
		{
			char szWorkingDir[MAX_PATH];
			if ( getcwd( szWorkingDir, sizeof( szWorkingDir ) ) == NULL )
			{
				fprintf( stderr, "error: couldn't get current directory.\n" );
				return EXIT_FAILURE;
			}

			if ( szExtension[0] != '\0' )
			{
				pszExtension = szExtension;
			}

			szSearch[0] = '\0';
			strncat( szSearch, szWorkingDir, MAX_PATH - 1 );
			strncat( szSearch, "\\*", 3 );
			strncat( szSearch, pszExtension, MAX_PATH - 1 );

#ifdef _WIN32
			hFind = FindFirstFileA( szSearch, &file );

			if ( hFind == INVALID_HANDLE_VALUE )
			{
				/* print windows error */
				LPVOID lpMsgBuf;
				const DWORD dwError = GetLastError();

				FormatMessageA(
					FORMAT_MESSAGE_ALLOCATE_BUFFER |
					FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					dwError,
					MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
					(LPSTR)&lpMsgBuf,
					0, NULL );

				fprintf( stderr, "%s", (LPSTR)lpMsgBuf );
				LocalFree( lpMsgBuf );

				fprintf( stderr, "error: windows threw %lu, bailing.\n", dwError );
				return EXIT_FAILURE;
			}
#else
			g_pszInputExtension = strrchr( pszFileName, '.' );
			dirIdx = scandir( ".", &nameList, extension_filter, alphasort );
			if ( dirIdx < 0 )
			{
				fprintf( stderr, "error: scandir failed, bailing\n" );
				return EXIT_FAILURE;
			}
			else
			{
				while ( dirIdx-- )
				{
					ProcessFile( nameList[dirIdx]->d_name );
				}

				free( nameList );
			}
#endif /* _WIN32 */

#ifdef _WIN32
			bFound = 1;
			while ( bFound )
			{
				ProcessFile( file.cFileName );
				bFound = FindNextFileA( hFind, &file );
			}

			FindClose( hFind );
#endif /* _WIN32 */
		}
		else
		{
			ProcessFile( pszFileName );
		}
	}

	return 0;
}
