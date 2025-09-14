it's a thing I guess

# documentation
Not much, but there is a syntax guide [here](docs/README.md) if you want to read it.

# building
```sh
git submodule update --init
make
```
Then run `build/mml --help` to display the command-line options.

# library documentation
It's not much of a library, but it is built to be easily extendable (hopefully that's true).
A minimal example:
```c
#include <stdint.h>

#include "mml/mml.h"

int32_t main(void)
{
	struct MML_state state = MML_init_state(NULL);
	MML_parse_stmts("println{cos{1.5pi} == 0.0}", &state);

	for (size_t i = 0; i < state.exprs.n; ++i)
		MML_eval_expr(&state, state.exprs.ptr[i]);

	MML_cleanup_state(&state);
}
```

And it can be compiled with this command (assuming you've run `make build_all`, are currently in the root directory, and named the example file `test.c`):
```sh
gcc -o test test.c -Iincl -Lbuild -lmml
```
