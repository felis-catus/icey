#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <malloc.h>
#include <direct.h>

#include "ice.h"

#include "winlite.h"

#define stricmp _stricmp
#define strnicmp _strnicmp

#define KEY_SIZE 8
#define DEFAULT_KEY	"x9Ke0BY7"
#define DEFAULT_EXTENSION ".ctx"

// Vars
static char g_szKey[KEY_SIZE + 1];
static char g_szExtension[_MAX_EXT];
bool g_bEncrypt;
bool g_bDecrypt;
bool g_bQuiet;

// Utils
#define Msg( format, ... ) if ( !g_bQuiet ) fprintf( stdout, format, __VA_ARGS__ )
#define Warning( format, ... ) fprintf( stderr, format, __VA_ARGS__ )
#define Error( format, ... ) fprintf( stderr, format, __VA_ARGS__ ); exit( EXIT_FAILURE )

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
		size_t nChars = min( end, outSize - 1 );
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

bool ProcessFile( const char *pszFileName )
{
	FILE *pFile = fopen( pszFileName, "rb" );
	if ( !pFile )
	{
		Warning( "couldn't open %s for input\n", pszFileName );
		return false;
	}

	size_t fileSize = 0;
	fseek( pFile, 0, SEEK_END );
	fileSize = ftell( pFile );
	rewind( pFile );

	unsigned char *pInBuf = (unsigned char*)_alloca( fileSize );
	unsigned char *pOutBuf = (unsigned char *)_alloca( fileSize );

	fread( pInBuf, fileSize, 1, pFile );
	fclose( pFile );

	ICE_KEY *pKey = ice_key_create( 0 );
	ice_key_set( pKey, (unsigned char*)g_szKey );

	int blockSize = ice_key_block_size( pKey );
	int bytesLeft = fileSize;

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
	UTIL_StripExtension( pszFileName, szOutName, sizeof( szOutName ) );
	strncat( szOutName, g_szExtension, sizeof( szOutName ) );

	pFile = fopen( szOutName, "wb" );
	if ( !pFile )
	{
		Warning( "couldn't open %s for output\n", szOutName );
		return false;
	}

	fwrite( pOutBuf, fileSize, 1, pFile );
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
		Warning( "usage: icey [-encrypt] [-decrypt] [-key abcdefgh] [-extension .ctx] file file2 ...\n\n" );
		Warning( "-encrypt | -e : encrypt files (default)\n" );
		Warning( "-decrypt | -d : decrypt files\n" );
		Warning( "-key | -k : key, must be 8 chars\n" );
		Warning( "-extension | -x : file extension for output\n" );
		Warning( "-quiet | -q : don't print anything (excl. errors)\n\n" );
		Warning( "eg.\n" );
		Warning( "icey -encrypt -key sEvVdNEq -extension .ctx file.txt\n" );
		Warning( "icey -x .ctx -k sEvVdNEq *.txt\n\n" );
		return;
	}

	g_szKey[0] = '\0';
	g_szExtension[0] = '\0';

	g_bEncrypt = true; // assume encrypt by default

	int i = 1;
	while( i < argc )
	{
		if ( stricmp( argv[i], "-e" ) == 0 || stricmp( argv[i], "-encrypt" ) == 0 )
		{
			g_bEncrypt = true;
			i++;
		}
		else if ( stricmp( argv[i], "-d" ) == 0 || stricmp( argv[i], "-decrypt" ) == 0 )
		{
			g_bDecrypt = true;
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
			_getcwd( cwd, sizeof( cwd ) );

			char filename[MAX_PATH];
			char extension[_MAX_EXT];
			_splitpath( pszFileName, NULL, NULL, filename, extension );

			if ( extension[0] != '\0' )
				pszExtension = extension;

			char szSearch[MAX_PATH];
			snprintf( szSearch, sizeof( szSearch ), "%s\\*%s", cwd, pszExtension );
			WIN32_FIND_DATAA file;
			HANDLE hFind = FindFirstFileA( szSearch, &file );

			if ( hFind == INVALID_HANDLE_VALUE )
			{
				Error( "error: windows threw %d, bailing\n", GetLastError() );
			}

			bool bFound = true;
			while ( bFound )
			{
				ProcessFile( file.cFileName );
				bFound = FindNextFileA( hFind, &file );
			}

			FindClose( hFind );
		}
		else
		{
			ProcessFile( pszFileName );
		}
	}

	return 0;
}
