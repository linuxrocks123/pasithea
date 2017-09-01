#! /usr/bin/env python

import os

WIN32=True
MACOSX=True
LINUX=True

PROGRAMS = ("pasithea",)
FLAGS="-g"

flist = os.listdir(".")

def has_suffix(filename,suffix):
    if filename.rfind(suffix)==-1:
        return False
    return filename.rfind(suffix)+len(suffix)==len(filename)

def get_headers(fname):
    f = open(fname)
    lines = f.read().splitlines()
    for line in lines:
        if line.find("#include")==0:
            to_include = line.split()[1]
            if to_include[0]=='"':
                print to_include[1:len(to_include)-1]+" / "+fname
                print fname+" touch "+fname

common_objects = []
for fname in flist:
    if has_suffix(fname,".cpp"):
        base_name=fname[:len(fname)-4]
        get_headers(fname)
        if LINUX:
            print base_name+".cpp / "+base_name+".o"
        if WIN32:
            print base_name+".cpp / "+base_name+".obj"
        if MACOSX:
            print base_name+".cpp / "+base_name+".macho"
        if LINUX:
            print base_name+".o g++ -std=gnu++1y -c "+FLAGS+" "+base_name+".cpp"
        if WIN32:
            print base_name+".obj /opt/mxe/usr/bin/i686-w64-mingw32.static-g++ -std=gnu++1y -static -mwindows -c "+FLAGS+" "+base_name+".cpp -o "+base_name+".obj"
        if MACOSX:
            print base_name+'.macho sh -c "source ./mac.source; o64-g++ -I/opt/osxcross/target/macports/pkgs/opt/local/include -std=gnu++1y -c '+FLAGS+' '+base_name+'.cpp -o '+base_name+'.macho"'
        if not has_suffix(fname,"_main.cpp"):
            common_objects.append(base_name)
            for target in PROGRAMS:
                if LINUX:
                    print base_name+".o / "+target
                if WIN32:
                    print base_name+".obj / "+target+"_win32.exe"
                if MACOSX:
                    print base_name+".macho / "+target+"_mac"
        else:
            if LINUX:
                print base_name+".o / "+fname[:fname.index("_")]
            if WIN32:
                print base_name+".obj / "+fname[:fname.index("_")]+"_win32.exe"
            if MACOSX:
                print base_name+".macho / "+fname[:fname.index("_")]+"_mac"
    elif has_suffix(fname,".hpp") or has_suffix(fname,".h"):
        get_headers(fname)

for target in PROGRAMS:
    if LINUX:
        print target+' sh -c "g++ -std=gnu++1y -lfltk `ls philotes/*.o | grep -v _main.o$` '+' '.join(map(lambda x: x+".o",common_objects))+' '+target+'_main.o -o '+target+'"'
    if WIN32:
        print target+'_win32.exe sh -c "/opt/mxe/usr/bin/i686-w64-mingw32.static-g++ -std=gnu++1y -static -mwindows `ls philotes/*.obj | grep -v _main.obj$` '+' '.join(map(lambda x: x+".obj",common_objects))+' '+target+'_main.obj /opt/mxe/usr/i686-w64-mingw32.static/lib/libfltk.a philotes/win32_bin/cyg*.dll -lws2_32 -lole32 -luuid -lcomctl32 -o '+target+'_win32.exe"'
    if MACOSX:
        print target+'_mac sh -c "source ./mac.source; o64-g++ -std=gnu++1y `ls philotes/*.macho | grep -v _main.macho$` '+' '.join(map(lambda x: x+".macho",common_objects))+' '+target+'_main.macho -o '+target+'_mac -L/opt/osxcross/target/macports/pkgs/opt/local/lib -Wl,-headerpad_max_install_names /opt/osxcross/target/macports/pkgs/opt/local/lib/libfltk.a -lpthread -framework Cocoa"'
