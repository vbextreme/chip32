###########
# project #
###########
project('chip32', 'c')
projectName = 'chip32'

cc = meson.get_compiler('c')

###########
# include #
###########

includeDir = include_directories('include')

###############
# source file #
###############

src = [ 'core/main.c', 'core/rbhash.c', 'core/hashalg.c', 'core/str.c', 'core/chip32.c', 'core/c32asm.c', 'core/c32asmlex.c', 'core/c32asmpars.c' ]

##################
# compiler flags #
##################

# warnings
add_global_arguments('-Wall', language: 'c')
add_global_arguments('-Wextra', language: 'c')
add_global_arguments('-Wuninitialized', language: 'c')
add_global_arguments('-Winit-self', language: 'c')
add_global_arguments('-Wstrict-aliasing', language: 'c')
add_global_arguments('-Wstrict-overflow', language: 'c')
add_global_arguments('-Wfloat-equal', language: 'c')
add_global_arguments('-Wvla', language: 'c')

# standard
add_global_arguments('-std=gnu11', language: 'c')

# open mp
if get_option('openmp') > 0
  add_global_arguments('-fopenmp', language:'c')
  add_global_link_arguments('-fopenmp', language:'c')
endif 

# gprof
if get_option('gprof') > 0
  add_global_arguments('-pg', language:'c')
  add_global_link_arguments('-pg', language:'c')
endif

# optimization
if( get_option('optimize') > 0 )
  add_global_arguments('-O2', language: 'c')
  add_global_arguments('-march=native', language: 'c')
  add_global_arguments('-mtune=native', language: 'c')
else
  add_global_arguments('-g', language: 'c')
endif

##########################
# libraries dependencies #
##########################

# template
#m_dep = cc.find_library('m', required : true)
libDeps = [  ] 

#########################
# software dependencies #
#########################

# template
#tic = find_program('tic', required: true)

#################
# Custom Target #
#################

# template
#tic_src = custom_target('tic',
#  output: 'termll-extend',
#  input: 'ti/termll-extend.ti',
#  command: [tic, '-x', '-o', '@OUTPUT@', '@INPUT@']
#)
#src += [ tic_src ]

#########
# debug #
#########

opt_debug = get_option('ebug')
if opt_debug > 0
  message('debug enabled')
  debugstr = '-DEF_DEBUG_ENABLE=@0@'
  debugstr = debugstr.format(opt_debug)
  add_global_arguments(debugstr, language: 'c')
endif

if get_option('olor') > 0
  message('color debug enabled')
  add_global_arguments('-DEF_DEBUG_COLOR=1', language: 'c')
endif

if get_option('assert') > 0
  message('assertion enabled')
  add_global_arguments('-DEF_ASSERT_ENABLE=1', language: 'c')
endif

###########
# license #
###########

message('copyright vbextreme 2018')
message('released under GPLv3')

#########
# build #
#########

executable(projectName, src, include_directories: includeDir, dependencies: libDeps, install: true)

