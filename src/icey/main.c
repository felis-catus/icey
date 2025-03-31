#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <malloc.h>

#include "ice.h"

#ifndef _WIN32
#define min(X, Y)  ((X) < (Y) ? (X) : (Y))
#endif // _WIN32

#ifdef _WIN32
#include "winlite.h"
#else
#include <dirent.h>
#define MAX_PATH 260
#define _MAX_EXT 256
#endif // _WIN32

#ifdef _WIN32
#define stricmp _stricmp
#endif // _WIN32

#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif // _WIN32

#define KEY_SIZE 8
#define DEFAULT_KEY	"x9Ke0BY7"
#define DEFAULT_EXTENSION ".ctx"

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

static char g_szKey[KEY_SIZE + 1];
static char g_szExtension[_MAX_EXT];
#ifndef _WIN32
const char *g_pszInputExtension = NULL;
#endif // _WIN32
bool g_bEncrypt;
bool g_bDecrypt;
bool g_bQuiet;
bool g_bNoFill;
char g_chFillChar = '\n'; // fill remainder with newlines by default

//-----------------------------------------------------------------------------
// Utils
//-----------------------------------------------------------------------------

#define Msg( format, ... ) if ( !g_bQuiet ) fprintf( stdout, format, ##__VA_ARGS__ )
#define Warning( format, ... ) fprintf( stderr, format, ##__VA_ARGS__ )

//-----------------------------------------------------------------------------
inline bool UTIL_IsPathSeparator( const char c )
{
	return c == '\\' || c == '/';
}

//-----------------------------------------------------------------------------
void UTIL_StripExtension( const char *pszIn, char *pszOut, const size_t outSize )
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

//-----------------------------------------------------------------------------
long UTIL_GetFileSize( FILE *pFile )
{
	fseek( pFile, 0, SEEK_END );
	const long fileSize = ftell( pFile );
	fseek( pFile, 0, SEEK_SET );
	return fileSize;
}

#ifndef _WIN32
//-----------------------------------------------------------------------------
int stricmp( char const *a, char const *b )
{
	for ( ;; a++, b++ )
	{
		int d = tolower( (unsigned char)*a ) - tolower( (unsigned char)*b );
		if ( d != 0 || !*a )
			return d;
	}
}

//-----------------------------------------------------------------------------
int extension_filter( const struct dirent *dir )
{
	if ( !dir )
		return 0;

	if ( dir->d_type == DT_REG ) // Only deal with regular files
	{
		const char *ext = strrchr( dir->d_name, '.' );
		if ( ( !ext ) || ( ext == dir->d_name ) )
			return 0;
		else
		{
			if ( stricmp( ext, g_pszInputExtension ) == 0 )
				return 1;
		}
	}

	return 0;
}
#endif // _WIN32

//-----------------------------------------------------------------------------
bool ProcessFile( const char *pszFileName )
{
	FILE *pFile = fopen( pszFileName, "rb" );
	if ( pFile == NULL )
	{
		Warning( "couldn't open %s for input\n", pszFileName );
		return false;
	}

	ICE_KEY *pKey = ice_key_create( 0 );
	ice_key_set( pKey, (unsigned char *)g_szKey );

	const long fileSize = UTIL_GetFileSize( pFile );
	const int blockSize = ice_key_block_size( pKey );
	const int remainder = fileSize % blockSize;

	if ( g_bNoFill && remainder != 0 )
	{
		Warning( "warning: %d bytes of unencrypted data leaked due to -nofill.\n", remainder );
	}

	const bool bFill = ( !g_bNoFill && remainder != 0 ) ? true : false;
	const long bufferSize = fileSize + ( bFill ? ( blockSize - remainder ) : 0 );

	unsigned char *pInBuf = alloca( bufferSize );
	unsigned char *pOutBuf = alloca( bufferSize );

	memset( pInBuf, 0, bufferSize );
	memset( pOutBuf, 0, bufferSize );

	fread( pInBuf, bufferSize, 1, pFile );
	fclose( pFile );

	if ( bFill )
	{
		const int endPos = ( fileSize + 1 ) - 1;
		for ( int i = endPos; i < bufferSize; ++i )
		{
			pInBuf[i] = g_chFillChar;
		}
	}

	int bytesLeft = bufferSize;

	const unsigned char *pIn = pInBuf;
	unsigned char *pOut = pOutBuf;

	while ( bytesLeft >= blockSize )
	{
		if ( g_bEncrypt )
			ice_key_encrypt( pKey, pIn, pOut );
		else if ( g_bDecrypt )
			ice_key_decrypt( pKey, pIn, pOut );

		bytesLeft -= blockSize;
		pIn += blockSize;
		pOut += blockSize;
	}

	memcpy( pOut, pIn, bytesLeft );

	char szOutName[MAX_PATH];
	UTIL_StripExtension( pszFileName, szOutName, MAX_PATH );
	strncat( szOutName, g_szExtension, MAX_PATH - strlen( szOutName ) );

	pFile = fopen( szOutName, "wb" );
	if ( pFile == NULL )
	{
		Warning( "couldn't open %s for output\n", szOutName );
		return false;
	}

	fwrite( pOutBuf, bufferSize, 1, pFile );
	fclose( pFile );

	Msg( "handled file %s\n", pszFileName );

	ice_key_destroy( pKey );
	return true;
}

//-----------------------------------------------------------------------------
int main( int argc, char *argv[] )
{
	g_bQuiet = false;

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
		Warning( "%s", pszHelp );
#endif
		return EXIT_SUCCESS;
	}

	g_szKey[0] = '\0';
	g_szExtension[0] = '\0';

	g_bEncrypt = true; // assume encrypt by default

	int i = 1;
	while ( i < argc )
	{
		if ( stricmp( argv[i], "-e" ) == 0 || stricmp( argv[i], "-encrypt" ) == 0 )
		{
			g_bEncrypt = true;
			g_bDecrypt = false;
			++i;
		}
		else if ( stricmp( argv[i], "-d" ) == 0 || stricmp( argv[i], "-decrypt" ) == 0 )
		{
			g_bDecrypt = true;
			g_bEncrypt = false;
			++i;
		}
		else if ( stricmp( argv[i], "-k" ) == 0 || stricmp( argv[i], "-key" ) == 0 )
		{
			++i;
			if ( strlen( argv[i] ) != KEY_SIZE )
			{
				Warning( "error: ICE key must be 8 char text!\n" );
				return EXIT_FAILURE;
			}

			strncpy( g_szKey, argv[i], sizeof( g_szKey ) );
			++i;
		}
		else if ( stricmp( argv[i], "-x" ) == 0 || stricmp( argv[i], "-extension" ) == 0 )
		{
			++i;
			if ( strlen( argv[i] ) < 1 )
			{
				Warning( "error: bad extension.\n" );
				return EXIT_FAILURE;
			}

			strncpy( g_szExtension, argv[i], sizeof( g_szExtension ) );
			++i;
		}
		else if ( stricmp( argv[i], "-q" ) == 0 || stricmp( argv[i], "-quiet" ) == 0 )
		{
			g_bQuiet = true;
			++i;
		}
		else if ( stricmp( argv[i], "-nofill" ) == 0 )
		{
			Warning( "-nofill\n" );
			g_bNoFill = true;
			++i;
		}
		else
		{
			break;
		}
	}

	if ( i >= argc )
	{
		Warning( "error: no files in cmd line.\n" );
		return EXIT_FAILURE;
	}

	if ( g_szKey[0] == '\0' )
	{
		Msg( "no key, using default (%s)\n", DEFAULT_KEY );
		strncpy( g_szKey, DEFAULT_KEY, sizeof( g_szKey ) );
	}

	if ( g_szExtension[0] == '\0' )
	{
		Msg( "no extension, using default (%s)\n", DEFAULT_EXTENSION );
		strncpy( g_szExtension, DEFAULT_EXTENSION, sizeof( g_szExtension ) );
	}

	for ( ; i < argc; ++i )
	{
		const char *pszFileName = argv[i];

		if ( strstr( pszFileName, "*." ) )
		{
			char cwd[MAX_PATH];
			if ( getcwd( cwd, sizeof( cwd ) ) == NULL )
			{
				Warning( "error: couldn't get current directory.\n" );
				return EXIT_FAILURE;
			}

			char szFileName[MAX_PATH];
			char szExtension[_MAX_EXT];
#ifdef _WIN32
			_splitpath( pszFileName, NULL, NULL, szFileName, szExtension );
#endif
			const char *pszExtension = DEFAULT_EXTENSION;
			if ( szExtension[0] != '\0' )
				pszExtension = szExtension;

			char szSearch[MAX_PATH];
			snprintf( szSearch, sizeof( szSearch ), "%s\\*%s", cwd, pszExtension );

#ifdef _WIN32
			WIN32_FIND_DATAA file;
			const HANDLE hFind = FindFirstFileA( szSearch, &file );

			if ( hFind == INVALID_HANDLE_VALUE )
			{
				// print windows error
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

				Warning( "%s", (LPSTR)lpMsgBuf );
				LocalFree( lpMsgBuf );

				Warning( "error: windows threw %lu, bailing.\n", dwError );
				return EXIT_FAILURE;
			}
#else
			g_pszInputExtension = strrchr( pszFileName, '.' );
			struct dirent **nameList;
			int n = scandir( ".", &nameList, extension_filter, alphasort );
			if ( n < 0 )
			{
				Warning( "error: scandir failed, bailing\n" );
				return EXIT_FAILURE;
			}
			else
			{
				while ( n-- )
				{
					ProcessFile( nameList[n]->d_name );
				}

				free( nameList );
			}
#endif

#ifdef _WIN32
			bool bFound = true;
			while ( bFound )
			{
				ProcessFile( file.cFileName );
				bFound = FindNextFileA( hFind, &file );
			}

			FindClose( hFind );
#endif
		}
		else
		{
			ProcessFile( pszFileName );
		}
	}

	return 0;
}
