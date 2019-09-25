@set CFLAG=-DNDEBUG
@set CFLAG=%CFLAG% -c
@set CFLAG=%CFLAG% -Ox
@set CFLAG=%CFLAG% -EHsc
cl -DKANAMESHIKI_HEAP_SPECIALIZATION=1 src/KanameShiki.cpp %CFLAG% -Fo:KanameShiki1.obj && lib -out:KanameShiki1.lib KanameShiki1.obj
cl -DKANAMESHIKI_HEAP_SPECIALIZATION=0 src/KanameShiki.cpp %CFLAG% -Fo:KanameShiki0.obj && lib -out:KanameShiki0.lib KanameShiki0.obj
