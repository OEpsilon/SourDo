# SourDo

SourDo is a scripting language intended for use in C++ programs. It takes inspiration from Lua while also giving a more C++ like experience.

The language and the provided interpreter is still currently in development.

## Code Examples
<i> Note: Requires the basic library to be loading using the `sourdo::load_lib_basic` function. </i>

### Hello World
Prints to the console "Hello World"
```
print("Hello World!")
```

### Factorial
Returns the factorial of the given number 'value'
```
func factorial(value)
    if value <= 1 then
        return 1
    end
    return value * factorial(value - 1)
end
```
