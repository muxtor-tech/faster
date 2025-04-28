# compilation test
test(
    'multibyte-non-unicode',
    executable(
        'test-binary-0nu',
        [
            'mb-unit.c',
            '../src/avl.c',
            '../src/core.c',
            '../src/ast.c',
            '../src/is.c',
            '../src/str.c',
        ],
        c_args: ['-O0', '-g3', '-DFASTER_UNICODE_SUPPORT=0'],
        include_directories: incdir,
        override_options: ['warning_level=0'],
    ),
)
test(
    'multibyte-utf8',
    executable(
        'test-binary-0u8',
        [
            'mb-unit.c',
            '../src/avl.c',
            '../src/core.c',
            '../src/ast.c',
            '../src/is.c',
            '../src/str.c',
        ],
        c_args: ['-O0', '-g3', '-DFASTER_UNICODE_SUPPORT=1'],
        include_directories: incdir,
        override_options: ['warning_level=0'],
    ),
)
test(
    'multibyte-utf16',
    executable(
        'test-binary-0u16',
        [
            'mb-unit.c',
            '../src/avl.c',
            '../src/core.c',
            '../src/ast.c',
            '../src/is.c',
            '../src/str.c',
        ],
        c_args: ['-O0', '-g3', '-DFASTER_UNICODE_SUPPORT=2'],
        include_directories: incdir,
        override_options: ['warning_level=0'],
    ),
)
test(
    'multibyte-utf32',
    executable(
        'test-binary-0u32',
        [
            'mb-unit.c',
            '../src/avl.c',
            '../src/core.c',
            '../src/ast.c',
            '../src/is.c',
            '../src/str.c',
        ],
        c_args: ['-O0', '-g3', '-DFASTER_UNICODE_SUPPORT=4'],
        include_directories: incdir,
        override_options: ['warning_level=0'],
    ),
)

# debug versions
test(
    'avl-test-small',
    executable(
        'test-binary-1',
        ['avl-unit-1.c', '../src/str.c', '../src/avl.c', '../src/core.c'],
        c_args: ['-O0', '-g3'],
        include_directories: incdir,
        override_options: ['warning_level=0'],
    ),
)
test(
    'avl-test-large',
    executable(
        'test-binary-2',
        ['avl-unit-2.c', '../src/str.c', '../src/avl.c', '../src/core.c'],
        c_args: ['-O0', '-g3'],
        include_directories: incdir,
        override_options: ['warning_level=0'],
    ),
)
test(
    'avl-test-rem',
    executable(
        'test-binary-3',
        ['avl-unit-3.c', '../src/str.c', '../src/avl.c', '../src/core.c'],
        c_args: ['-O0', '-g3'],
        include_directories: incdir,
        override_options: ['warning_level=0'],
    ),
)
test(
    'interned_strings',
    executable(
        'test-binary-4',
        ['../src/str.c', 'is-unit.c', '../src/avl.c', '../src/is.c', '../src/core.c'],
        c_args: ['-O0', '-g3'],
        include_directories: incdir,
        override_options: ['warning_level=0'],
    ),
)
test(
    'interned_strings-non-unicode',
    executable(
        'test-binary-4n',
        ['../src/str.c', 'is-unit.c', '../src/avl.c', '../src/is.c', '../src/core.c'],
        c_args: ['-O0', '-g3', '-DFASTER_UNICODE_SUPPORT=0'],
        include_directories: incdir,
        override_options: ['warning_level=0'],
    ),
)

# heavy optimized versions
test(
    'o-avl-test-small',
    executable(
        'test-binary-1o',
        ['avl-unit-1.c', '../src/str.c', '../src/avl.c', '../src/core.c'],
        c_args: ['-O3', '-g0'],
        include_directories: incdir,
        override_options: ['warning_level=0'],
    ),
)
test(
    'o-avl-test-large',
    executable(
        'test-binary-2o',
        ['avl-unit-2.c', '../src/str.c', '../src/avl.c', '../src/core.c'],
        c_args: ['-O3', '-g0'],
        include_directories: incdir,
        override_options: ['warning_level=0'],
    ),
)
test(
    'o-avl-test-rem',
    executable(
        'test-binary-3o',
        ['avl-unit-3.c', '../src/str.c', '../src/avl.c', '../src/core.c'],
        c_args: ['-O3', '-g0'],
        include_directories: incdir,
        override_options: ['warning_level=0'],
    ),
)
test(
    'o-interned_strings',
    executable(
        'test-binary-4o',
        ['../src/str.c', 'is-unit.c', '../src/avl.c', '../src/is.c', '../src/core.c'],
        c_args: ['-O3', '-g0'],
        include_directories: incdir,
        override_options: ['warning_level=0'],
    ),
)
test(
    'o-interned_strings-non-unicode',
    executable(
        'test-binary-4on',
        ['../src/str.c', 'is-unit.c', '../src/avl.c', '../src/is.c', '../src/core.c'],
        c_args: ['-O3', '-g0', '-DFASTER_UNICODE_SUPPORT=0'],
        include_directories: incdir,
        override_options: ['warning_level=0'],
    ),
)

# hash table tests
test(
    'ht-test-large',
    executable(
        'test-binary-6',
        ['ht-unit.c', '../src/str.c', '../src/ht.c', '../src/core.c'],
        c_args: ['-O0', '-g3'],
        include_directories: incdir,
        override_options: ['warning_level=0'],
    ),
)
test(
    'o-ht-test-large',
    executable(
        'test-binary-6o',
        ['ht-unit.c', '../src/str.c', '../src/ht.c', '../src/core.c'],
        c_args: ['-O3', '-g0'],
        include_directories: incdir,
        override_options: ['warning_level=0'],
    ),
)
test(
    'o-ht-test-large-non-unicode',
    executable(
        'test-binary-6onu',
        ['ht-unit.c', '../src/str.c', '../src/ht.c', '../src/core.c'],
        c_args: ['-O3', '-g0', '-DFASTER_UNICODE_SUPPORT=0'],
        include_directories: incdir,
        override_options: ['warning_level=0'],
    ),
)

# non-parallel tests
test(
    'atomic-queue',
    executable(
        'test-binary-5',
        ['../src/aq.c', 'aq-unit.c'],
        c_args: ['-O0', '-g3'],
        include_directories: incdir,
        override_options: ['warning_level=0'],
    ),
    is_parallel: false,
)
test(
    'o-atomic-queue',
    executable(
        'test-binary-5o',
        ['../src/aq.c', 'aq-unit.c'],
        c_args: ['-O3', '-g0'],
        include_directories: incdir,
        override_options: ['warning_level=0'],
    ),
    is_parallel: false,
)
