project('libslimbook',['cpp'], default_options: ['cpp_std=c++17'])
subdir('src')

install_data('99-slimbook-settings.rules', install_dir:'lib/udev/rules.d/')
install_data('slimbook-settings.service', install_dir:'lib/systemd/system/')
install_data('slimbook-sleep', install_dir:'lib/systemd/system-sleep/')
install_subdir('report.d', install_dir:'libexec/slimbook/')
install_data('report-pack', install_dir:'libexec/slimbook/')
install_data('slimbook-hello', install_dir:'bin/')
install_data('slimbookctl.completion', install_dir:'share/bash-completion/completions/', rename:['slimbookctl'])

