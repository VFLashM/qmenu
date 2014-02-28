import binary

Import('env')

name = 'qmenu'

libs = {
    'win32-msvc' : [],
    'linux-gcc' : [],
}
libpaths = {
    'win32-msvc' : [],
    'linux-gcc' : [],
}
cpppaths = ['Src']
srcpaths = ['Src']

platform = env['platform']

result = binary.createBinary(
    env,
    name, 
    ['QtCore', 'QtNetwork', 'QtGui', 'QtOpenGL'], 
    libs[platform],
    cpppaths,
    libpaths[platform],
    srcpaths,
)

Return('result')

