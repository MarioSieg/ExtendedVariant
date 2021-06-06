# [WIP] ExtendedVariant
This single header library is part of my C++ extended standard ```stdex``` libraries. Check our my profile for more.<br>
Working with C++ 17's ```std::variant``` can be cumbersome and verbose.<br>
This single header library contains the alternative ```stdex::variant```,<br>
which has the same interface as ```std::variant``` and usually works as a drop-in replacement,<br>
but has a cleaner interface and more goodies.<br>

<h2> Features </h2>

:heavy_check_mark: Single header file, C++ 17<br>
:heavy_check_mark: STL interface support (```std::get```, ```std::get_if```, ```std::visit``` etc..)<br>
:heavy_check_mark: Cleaner and less verbose interface (see examples below)<br>
:heavy_check_mark: Lower memory footprint (uses smart index type based on type count)<br>
:heavy_check_mark: Allows custom data alignment<br>
:heavy_check_mark: Full ```constexpr``` support<br>
:heavy_check_mark: Small template code generation<br>
:heavy_check_mark: Fast compile times<br>
:heavy_check_mark: Bonus functions and methods (see examples below)<br>

<h2> Usage </h2>

Just copy the ```extended_variant.hpp``` file into your source code, that's it.<br>
Please remember to include the ```LICENSE``` file according to the license agreement.<br>

<h2> Examples </h2>

<h3> Checking element type </h3>

With ```std::variant```:<br>
```cpp
if(std::holds_alternative<int>(variant))
 ...
```
 
With ```stdex::variant```:
```cpp
if(variant.contains<int>)
 ...
```

<h3> Checking element type and value without exceptions </h3>

With ```std::variant```:<br>
```cpp
if(std::holds_alternative<int>(variant) && std::get<int>(variant) == 3)
 ...
```
 
With ```stdex::variant```:
```cpp
if(variant.contains<int>(3))
 ...
```
Since the type can be elided from the literal, we can even write:
```cpp
if(variant.contains(3))
 ...
```

<h3> Getting the value directly </h3>

With ```std::variant```:<br>
```cpp
int value = std::get<int>(variant);
```
 
With ```stdex::variant``` using ```std::optional```:
```cpp
std::optional<int> value = variant.get<int>();
```

With ```stdex::variant``` using a default value on type mismatch:
```cpp
int value = variant.get_or_default<int>(); // Returns the default value of int (0) when the types do not match
```

With ```stdex::variant``` using a custom value on type mismatch:
```cpp
int value = *variant.get_or<int>(10); // Returns 10 when the types do not match
```

With ```stdex::variant``` using a lambda:
```cpp
int value = variant.get_or_invoke<int>([]() -> int { return 10 + 10; }); // Invokes the lambda and returns 20 when the types do not match
```

<h3> Converting to std::tuple </h3>

With ```stdex::variant```:<br>
```cpp
stdex::variant<int, float> variant{};
std::tuple<int, float> tuple = variant.as_tuple();
```

<h3> Converting to std::variant </h3>

With ```stdex::variant```:<br>
```cpp
stdex::variant<int, float> variant{};
std::variant<int, float> tuple = variant.as_std();
```

<h3> Visiting types </h3>

With ```std::variant``` using the overload pattern:<br>
```cpp
template <typename... Ts> struct overload : Ts... { using Ts::operator()...; };
template <typename... Ts> overload(Ts...) -> overload<Ts...>;

std::variant<int, float> variant{};
std::visit
(
	overload
	{
		[](int) { std::cout << "integer"; },
		[](float) { std::cout << "floating point"; },
	}, 
	variant
);
```
 
With ```stdex::variant``` using ```std::optional```:<br>
```cpp
stdex::variant<int, float> variant{};
variant.visit
(
	[](int) { std::cout << "integer"; },
	[](float) { std::cout << "floating point"; }
);
```

<h3> Getting the index at compile time </h3>

With ```stdex::variant```:<br>
```cpp
auto indexOfInt = stdex::variant<int, float>::index_of<int>();
```

<h3> Contributing </h3>

This library is not finished yet,
so it's **very** open to contributions!<br>
Everybody is welcome, just create an issue or submit your PR!<br>
