import os
import os.path

#def _relpath(path, root):
#    path = os.path.normpath(os.path.abspath(path))
#    root = os.path.normpath(os.path.abspath(root))
#    assert path.startswith(root)
#    return path[len(root) + 1:]
#os.path.relpath = _relpath

def gatherSources(dirs):
    res = []
    for d in dirs:
        for dirname, dirnames, filenames in os.walk(d):
            for filename in filenames:
                fpath = os.path.join(dirname, filename)
                if fpath.endswith('.cpp') or fpath.endswith('.c'):
                    res.append(fpath)
    return res

def configureCompiler(env, platform, configuration):
    tools = {
        'linux-gcc'  : ['qt4', 'g++', 'ar', 'gnulink'],
        'win32-msvc' : ['qt4', 'msvc', 'mslink'],
    }
    flags = {
        'linux-gcc'  : {
            'debug'   : ['-Wall', '-Werror', '-g'],
            'release' : ['-Wall', '-Werror'],
        },
        'win32-msvc' : {
            'debug'   : ['/Zi'],
            'release' : [],
        },
    }
    defines = {
        'linux-gcc'  : ['TARGET_PLATFORM_LINUX', 'TARGET_PLATFORM_LINUX_GCC'],
        'win32-msvc' : ['TARGET_PLATFORM_WINDOWS', 'TARGET_PLATFORM_WIN32_MSVC'],
    }
    commonDefines = ['PUGIXML_NO_EXCEPTIONS']
    targetArch = {
        'linux-gcc'  : None,
        'win32-msvc' : 'x86',
        'win64-msvc' : 'x86_64',
    }
        
    if targetArch[platform]:
        env['TARGET_ARCH'] = targetArch[platform]
    for tool in tools[platform]:
        env.Tool(tool)
    env.Append(CCFLAGS = flags[platform][configuration])
    env.Append(CPPDEFINES = defines[platform] + commonDefines)


def createBinary(env, name, qtLibs, libs, cppPaths, libPaths, sourceDirs):
    env = env.Clone()
    
    configureCompiler(env, env['platform'], env['configuration'])
    env.EnableQt4Modules(qtLibs, debug=False, crosscompiling=False)

    env.Append(CPPPATH=cppPaths)
    env.Append(LIBPATH=libPaths)

    env.AppendUnique(LIBS=libs)

    sourceDirs = map(lambda x : os.path.join(env['root'], x), sourceDirs)

    sources = gatherSources(sourceDirs)

    sources = map(lambda x : os.path.relpath(x, env['root']), sources)

    objects = env.Object(sources)

    if env['platform'] == 'win32-msvc':
        env['PDB'] = '${TARGET}.pdb'
        name += '.exe'

    return env.Program(name, objects)

