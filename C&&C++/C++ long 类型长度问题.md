`long`、`long long` 为 `long int`、`long long int` 缩写。

`long` >= `int`，但是不一定大于 `int`。


大多数编译器在 64 位机器上定义 `int` 为 32 位，`long`、`long long` 为 64 位。和编译器、平台都有关。

`1 << 30` 为 2 的 30 次方，但是默认为 `int` 类型，一般左移超过 30 位就要小心了。

`1L << 60`、`1LL << 60`，默认类型为 `long`/`long long`。

尽量使用 `limits.h` 来获取某个类型的最大/小值。

