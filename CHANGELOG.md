0.1 -> 0.2
----------
* renamed root to global
* added explicit assignment to var to avoid implicit casts to val.
* added luapp11::do_file to execute lua files.
* added var::do_file to execute lua files.
* added the ability to assign vectors, sets, and maps to vars.
* added the ability to assign function pointers to vars.  They can be called from lua.
* added the ability to assign functors and lambdas to vars.  They can be called from lua.