
libslimbook = shared_library('slimbook', ['slimbook.cpp','configuration.cpp'], install: true, version: '1.0.0')

executable('slimbookctl', ['slimbookctl.cpp'],
    link_with: libslimbook,
    install: true,
    install_mode: ['rwsr-xr-x','root','root']
    )

install_headers('slimbook.h')
