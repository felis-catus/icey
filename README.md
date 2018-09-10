# icey
yet another fking ICE encryption tool

Why?
-----
Originally made as a personal quick and dirty replacement for [Source SDK VICE](https://developer.valvesoftware.com/wiki/VICE), this tool is pretty much identical to that. Stock VICE requires Steam and Source engine libraries to function, while this tool only uses system libraries and can be run everywhere.

Usage
-----
```
icey [-encrypt] [-decrypt] [-quiet] [-key abcdefgh] [-extension .ctx] file file2 ...

-encrypt | -e : encrypt files (default)
-decrypt | -d : decrypt files
-key | -k : key, must be 8 chars
-extension | -x : file extension for output
-quiet | -q : don't print anything (excl. errors)

eg.
icey -encrypt -key sEvVdNEq -extension .ctx file.txt
icey -x .ctx -k sEvVdNEq *.txt
```
