# TODO
[X] negation operator!! negative numbers are annoying without it <br />
[X] insert multiplication operation between consecutive values between which there is no operation (so 3i would parse the same as 3*i) <br />
[X] variable assignment (statements?) <br />
[X] fix memory leaks caused by adding variable assignment (memory leaks for literally everything?), then rebase dev back into main
[ ] fix segfault when using `--set_var:` to insert a variable when the variable value string is empty (e.g. `--set_var:A=`)
[ ] add syntax & capability for using ~~_Decimal128~~ probably `long double` to store a higher-precision number <br />
~~[ ] library-capability (parse with a string with "$ident" variables whose values are inserted at some point later on)~~ actually this is kind of a garbage idea that makes very little sense<br />
[ ] syntax guide and other documentation <br />
[ ] interactive prompt (like python's IDLE) <br />
