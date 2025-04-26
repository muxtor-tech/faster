flib = library(
    'faster',
    ['aq.c', 'ast.c', 'avl.c', 'core.c', 'is.c', 'str.c'],
    include_directories: incdir,
)
executable(
    'faster-cli',
    ['main.c'],
    link_with: flib,
    include_directories: incdir,
)
