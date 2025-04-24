flib = library(
    'faster',
    ['ast.c', 'avl.c', 'core.c', 'is.c'],
    include_directories: incdir,
)
executable(
    'faster-cli',
    ['main.c'],
    link_with: flib,
    include_directories: incdir,
)
