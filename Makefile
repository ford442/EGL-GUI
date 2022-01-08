tst2: main1.cpp  Makefile
	emcc main1.cpp -sFULL_ES2=0 -sFULL_ES3=1 --closure 0 -sGL_TESTING=1 -sUSE_SDL=2 \
-sUSE_WEBGL2=1 -sENVIRONMENT=web -sMALLOC='emmalloc' -sMAX_WEBGL_VERSION=2 -sMIN_WEBGL_VERSION=2 -sFORCE_FILESYSTEM=1 \
-ffast-math -DNDEBUG -sALLOW_MEMORY_GROWTH=0 -sINITIAL_MEMORY=1400mb \
-O3 -o gui001.js -sEXPORTED_FUNCTIONS='["_main","_str"]' -sEXPORTED_RUNTIME_METHODS=ccall \
--extern-post-js index.js --post-js filesys.js --post-js ccall.js --post-js fs.js -sSUPPORT_BIG_ENDIAN=0
