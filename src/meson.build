libusb = dependency('libusb-1.0', static : true)

mtk_lib = static_library('mtk', [
  'mtk_da.c',
  'mtk_device.c',
  'mtk_preloader.c',
], include_directories : include, dependencies : libusb)

mtk_dep = declare_dependency(link_with : mtk_lib, include_directories : include, dependencies : libusb)
