[ASTER-FORTH]

Aster FORTH is a subroutine-threaded, portable FORTH in C. It can easily be
embedded in C programs and I'm planning on using it as such in future projects.
It doesn't feature the entirety of the ANS standard, but words that do exist
should mostly adhere to it. Type WORDS to see what has currently been
implemented.

[INSTALLING]

After running compile.sh or compile_ncurses.sh, Aster can be installed on
Linux/Unix by running install.sh with root priveleges. When compiling with
ncurses, only the executable is linked to ncurses.

[USE]

Once Aster is installed, it can beincluded using #include "<aster.h>" and
linked to using "-laster". Before using Aster, aster_init must be called.

Key words such as "key", "emit" and "bye" are defined using function pointers
- aster_key, aster_emit and aster_bye respectively - and can be redefined at
runtime, which will also affect all uses of these words, for example in the
interactive interpreter or ACCEPT. Look at aster.h and aster_main.c for more
details.

Words can be added using either aster_addConstant or aster_addC.

An example of aster_addC would be:

    aster_addC(func, "FUN", ASTER_IMMEDIATE|ASTER_COMPILEONLY);

This would create the word FUN using the C function "func" that is both
immediate and compile-only. To create a word that is neither immediate nor
compile-only, just use zero.

Words should take no arguments and return void. To access the top of each
stack, use aster_stack[aster_sp-1] and aster_rstack[aster_rsp-1]. The macros
aster_sassert and aster_rassert can be used to check the presence of values
on the data and return stack respectively.

An example of aster_addConstant would be:

    aster_addConstant(10, "TEN");

With both of these examples, it is important to note the capitalization; Aster
capitalizes all of its input before finding words, but as aster_addC and
aster_addConstant use string literals they are case sensitive.

[LICENSE]

Aster is free to use under the MIT license. For more details, look at
"license.txt".

