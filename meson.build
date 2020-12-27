###########
# project #
###########
project('chip32', 'c')
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

###########
# license #
###########

message('copyright vbextreme 2018')
message('released under GPLv3')

#########
# build #
#########

executable(meson.project_name(), src, include_directories: includeDir, dependencies: libDeps, install: true)

