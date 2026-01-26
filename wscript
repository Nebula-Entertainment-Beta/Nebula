APPNAME = "MyGame"

top = "."
out = "build"

def options(opt):
    opt.load("compiler_cxx")

def configure(conf):
    conf.load("compiler_cxx")

def build(bld):
    bld.shlib(target="Nebula", source="Nebula/src/greet.cpp", includes="Nebula/include")
    bld.program(target=APPNAME, source="App/src/main.cpp", includes="Nebula/include", use="Nebula", rpath=".")