TEMPLATE = subdirs
SUBDIRS = common automata asyncLife

automata.depends = common
asyncLife.depends = common automata
