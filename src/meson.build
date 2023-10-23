
libslimbook = shared_library('slimbook', 'slimbook.cpp', install: true, version: '1.0.0')

executable('slimbookctl', ['slimbookctl.cpp'],
    link_with: libslimbook,
    install: true
    )

install_headers('slimbook.h')
