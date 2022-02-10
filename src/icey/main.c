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
#define strnicmp _strnicmp
#endif // _WIN32

#ifdef _WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif // _WIN32

#define KEY_SIZE 8
#define DEFAULT_KEY	"x9Ke0BY7"
#define DEFAULT_EXTENSION ".ctx"

// Vars
static char g_szKey[KEY_SIZE + 1];
static char g_szExtension[_MAX_EXT];
#ifndef _WIN32
const char *g_szInputExtension = NULL;
#endif // _WIN32
bool g_bEncrypt;
bool g_bDecrypt;
bool g_bQuiet;
bool g_bNoFill;
char g_chFillChar = '\n'; // fill remainder with newlines by default

// Utils
#define Msg( format, ... ) if ( !g_bQuiet ) fprintf( stdout, format, ##__VA_ARGS__ )
#define Warning( format, ... ) fprintf( stderr, format, ##__VA_ARGS__ )
#define Error( format, ... ) fprintf( stderr, format, ##__VA_ARGS__ ); exit( EXIT_FAILURE )

bool PATHSEPARATOR( char c )
{
	return c == '\\' || c == '/';
}

void UTIL_StripExtension( const char *in, char *out, size_t outSize )
{
	size_t end = strlen( in ) - 1;
	while ( end > 0 && in[end] != '.' && !PATHSEPARATOR( in[end] ) )
	{
		--end;
	}

	if ( end > 0 && !PATHSEPARATOR( in[end] ) && end < outSize )
	{
		const size_t nChars = min( end, outSize - 1 );
		if ( out != in )
		{
			memcpy( out, in, nChars );
		}
		out[nChars] = 0;
	}
	else
	{
		if ( out != in )
		{
			strncpy( out, in, outSize );
		}
	}
}

#ifndef _WIN32
int stricmp( char const *a, char const *b )
{
	for ( ;; a++, b++ )
	{
		int d = tolower( (unsigned char)*a ) - tolower( (unsigned char)*b );
		if ( d != 0 || !*a )
			return d;
	}
}

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
			if ( stricmp( ext, g_szInputExtension ) == 0 )
				return 1;
		}
	}

	return 0;
}
#endif // _WIN32

bool ProcessFile( const char *pszFileName )
{
	FILE *pFile = fopen( pszFileName, "rb" );
	if ( !pFile )
	{
		Warning( "couldn't open %s for input\n", pszFileName );
		return false;
	}

	fseek( pFile, 0, SEEK_END );
	
	const long fileSize = ftell( pFile );
	
	rewind( pFile );

	ICE_KEY *pKey = ice_key_create( 0 );
	ice_key_set( pKey, (unsigned char *)g_szKey );

	const int blockSize = ice_key_block_size( pKey );
	const int remainder = fileSize % blockSize;

	if ( g_bNoFill && remainder != 0 )
	{
		Warning( "warning: %d bytes of unencrypted data leaked due to -nofill.\n", remainder );
	}
	
	const bool bFill = ( !g_bNoFill && remainder != 0 ) ? true : false;
	const long bufferSize = fileSize + ( bFill ? ( blockSize - remainder ) : 0 );

	unsigned char *pInBuf = (unsigned char *)alloca( bufferSize );
	unsigned char *pOutBuf = (unsigned char *)alloca( bufferSize );

	memset( pInBuf, 0, bufferSize );
	memset( pOutBuf, 0, bufferSize );

	fread( pInBuf, bufferSize, 1, pFile );
	fclose( pFile );

	if ( bFill )
	{
		for ( int i = fileSize - 1; i < bufferSize; i++ )
		{
			pInBuf[i] = g_chFillChar;
		}
	}

	int bytesLeft = bufferSize;

	unsigned char *p1 = pInBuf;
	unsigned char *p2 = pOutBuf;

	while ( bytesLeft >= blockSize )
	{
		if ( g_bEncrypt )
			ice_key_encrypt( pKey, p1, p2 );
		else if ( g_bDecrypt )
			ice_key_decrypt( pKey, p1, p2 );

		bytesLeft -= blockSize;
		p1 += blockSize;
		p2 += blockSize;
	}

	memcpy( p2, p1, bytesLeft );

	char szOutName[MAX_PATH];
	UTIL_StripExtension( pszFileName, szOutName, MAX_PATH );
	strncat( szOutName, g_szExtension, MAX_PATH - strlen( szOutName ) );

	pFile = fopen( szOutName, "wb" );
	if ( !pFile )
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

int main( int argc, char *argv[] )
{
	g_bQuiet = false;

	if ( argc < 2 )
	{
		Warning( "usage: icey [-encrypt] [-decrypt] [-quiet] [-key abcdefgh] [-extension .ctx] file file2 ...\n\n" );
		Warning( "-encrypt | -e : encrypt files (default)\n" );
		Warning( "-decrypt | -d : decrypt files\n" );
		Warning( "-key | -k : key, must be 8 chars\n" );
		Warning( "-extension | -x : file extension for output\n" );
		Warning( "-quiet | -q : don't print anything (excl. errors)\n" );
		Warning( "-nofill : don't fill remainder with blank bytes (warning, this will leak unencrypted data)\n\n" );
		Warning( "e.g.\n" );
		Warning( "icey -encrypt -key sEvVdNEq -extension .ctx file.txt\n" );
		Warning( "icey -x .ctx -k sEvVdNEq *.txt\n" );
		return 0;
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
			i++;
		}
		else if ( stricmp( argv[i], "-d" ) == 0 || stricmp( argv[i], "-decrypt" ) == 0 )
		{
			g_bDecrypt = true;
			g_bEncrypt = false;
			i++;
		}
		else if ( stricmp( argv[i], "-k" ) == 0 || stricmp( argv[i], "-key" ) == 0 )
		{
			i++;
			if ( strlen( argv[i] ) != KEY_SIZE )
			{
				Error( "error: ICE key must be 8 char text!\n" );
			}

			strncpy( g_szKey, argv[i], sizeof( g_szKey ) );
			i++;
		}
		else if ( stricmp( argv[i], "-x" ) == 0 || stricmp( argv[i], "-extension" ) == 0 )
		{
			i++;
			if ( strlen( argv[i] ) < 1 )
			{
				Error( "error: bad extension.\n" );
			}

			strncpy( g_szExtension, argv[i], sizeof( g_szExtension ) );
			i++;
		}
		else if ( stricmp( argv[i], "-q" ) == 0 || stricmp( argv[i], "-quiet" ) == 0 )
		{
			g_bQuiet = true;
			i++;
		}
		else if ( stricmp( argv[i], "-nofill" ) == 0 )
		{
			Warning( "-nofill\n" );
			g_bNoFill = true;
			i++;
		}
		else
		{
			break;
		}
	}

	if ( i >= argc )
	{
		Error( "error: no files in cmd line.\n" );
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

	for ( ; i < argc; i++ )
	{
		const char *pszFileName = argv[i];
		const char *pszExtension = DEFAULT_EXTENSION;
		
		if ( strstr( pszFileName, "*." ) )
		{
			char cwd[MAX_PATH];
			if ( !GetCurrentDir( cwd, sizeof( cwd ) ) )
			{
				Error( "error: couldn't get current directory.\n" );
			}

			char filename[MAX_PATH];
			char extension[_MAX_EXT];
#ifdef _WIN32
			_splitpath( pszFileName, NULL, NULL, filename, extension );
#endif
			if ( extension[0] != '\0' )
				pszExtension = extension;

			char szSearch[MAX_PATH];
			snprintf( szSearch, sizeof( szSearch ), "%s\\*%s", cwd, pszExtension );

#ifdef _WIN32
			WIN32_FIND_DATAA file;
			HANDLE hFind = FindFirstFileA( szSearch, &file );

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
				Error( "error: windows threw %lu, bailing.\n", dwError );
			}
#else
			g_szInputExtension = strrchr( pszFileName, '.' );
			struct dirent **nameList;
			int n = scandir( ".", &nameList, extension_filter, alphasort );
			if ( n < 0 )
			{
				Error( "error: scandir failed, bailing\n" );
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
