TEMPLATE = subdirs
SUBDIRS = neurolab neurogui neurolib automata thirdparty # asyncLife
#asyncLife.depends = automata
neurolib.depends = automata
neurogui.depends = neurolib thirdparty
neurolab.depends = neurogui 
