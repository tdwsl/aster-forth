[ASTER-FORTH]

Aster FORTH is an extensible, portable FORTH in C. It can easily be
embedded in C programs and I'm planning on using it as such in future projects.
It doesn't feature the entirety of the ANS standard, but words that do exist
should mostly adhere to it. Type WORDS to see what has currently been
implemented.

[INSTALLING]

To install Aster FORTH on a Linux or Unix system, run install.sh. This
installs the static library, which can be linked to with -laster, as well as
the headers aster.h and aster_boot.h. Alternatively, it can easily be included
in a project by adding the relevant files. Look at main.c for more information.

[USE]

Aster FORTH was written in C, and can easily be intergrated in your projects.

Before using it, be sure to run aster_init. You'll probably want to run
aster_runString, too, as Aster FORTH isn't too useful without some default
words. Here is a snippet from main.c:

...
aster_init(argc, args);
aster_runString(aster_boot);
...

If you don't want to use any arguments, that's fine, just make sure that argc
is set to zero. A string is provided in aster_boot.h, but you can modify it
or use your own if you want.

Words can be added using either aster_addConstant or aster_addC.

An example of aster_addC would be:

    aster_addC(func, "Fun", ASTER_IMMEDIATE|ASTER_COMPILEONLY);

This would create the word Fun using the C function "func" that is both
immediate and compile-only. To create a word that is neither immediate nor
compile-only, just use zero.

Words should take no arguments and return void. To access the top of each
stack, use aster_stack[aster_sp-1] and aster_rstack[aster_rsp-1]. The macros
aster_sassert and aster_rassert can be used to check the presence of values
on the data and return stack respectively.

Both stacks in Aster FORTH are can contain up to 256 values and indexed with
an unsigned char, just to make things easier for checking overflow. Note that
the size of an unsigned char on the host system has to be 8 bits for this to
work. Aster FORTH should still work otherwise, but overflow checking won't.

Oftentimes, you may just want to add a constant value to the FORTH environment
from your program - aster_addConstant does that.

An example of aster_addConstant would be:

    aster_addConstant(10, "TEN");

Aster FORTH is case insensitive, so the literal Ten would override TEN. The
case of the words is, however, retained in the dictionary, so you'll be able
to see it when you type WORDS.

[LICENSE]

Aster is free to use under the MIT license. For more details, look at
"license.txt".

