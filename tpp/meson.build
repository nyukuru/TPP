# Workaround for https://github.com/mesonbuild/meson/issues/2320
config_h = configure_file(
  configuration : {
    'PACKAGE_NAME' :    '"' + meson.project_name() + '"',
    'PACKAGE_VERSION' : '"' + meson.project_version() + '"',
    'PACKAGE_HTTP_VERSION' : '"TwitchBot (https://github.com/nyukuru/TPP, ' + meson.project_version() + ')"',
    'PACKAGE_STRING' :  '"' + meson.project_name() + ' ' + meson.project_version() + '"',
  },
  output: 'config.h',
  install : true,
  install_dir : include_dir / 'tpp'
)

