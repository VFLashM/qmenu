import os
import sys
import qt4
sys.modules['SCons.Tool.qt4'] = sys.modules['qt4']

platforms = ['win32-msvc', 'linux-gcc']
configurations = ['release', 'debug']

optsCache = 'options.cache'

opts = Variables(optsCache)
opts.Add(EnumVariable('platform', 'Target platform', platforms[0], platforms))
opts.Add(EnumVariable('configuration', 'Build configuration', configurations[0], configurations))

env = Environment(options=opts, tools = [])

print opts.GenerateHelpText(env)
opts.Save(optsCache, env)

env['root'] = os.path.abspath('.')

prefix = '.build.%s.%s' % (env['platform'], env['configuration'])
binary = env.SConscript('binary.SConscript', exports=['env'], build_dir=prefix, duplicate=0)

res = env.Install('.', binary)

Default(res)
