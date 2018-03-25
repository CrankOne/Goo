# Goo Unit Tests

Goo library provides (pretty rudimentary, though) unit testing facility to
perform self-checks.

## TODO

Some lines are excluded with `_Goo_m_SHALL_FAIL` macro indicating that this
usage must lead to compile-time failure (usually, due to SFINAE). CMake scripts
shall further be modified in order to perform automated failure tests with
`add_test()` / `set_tests_properties( ... WILL_FAIL TRUE)`. See
[this SO answer](https://stackoverflow.com/a/30191576/1734499) for example.

