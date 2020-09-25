# Paradigm Standard Library (PSL)

## Introduction
This library is written to supplement the standard library with functionality that is either missing, or functionality that is better suited for use in game development. As an example, the library contains facilities to handle serialization in as much zero-overhead that is possible. It has a custom allocator mechanism implementeted that allows it to represent various different types of memory resources (such as memory resources that might be writeable, but not readable, like certain parts of the GPU memory).

#### What this library isn't
It's not a replacement standard library. The standard library is high quality, well tested, performant, and used extensively within this library, and this library will not supply replacements for well implemented functionality that is present in the standard library (such as the algorithms, etc...). This library supplements the standard library.

It might provide wrapping facilities to standard library functionality, either to facilitate future expansion of this library, or to unify usage.

## Customization
The library has a user tweakable configuration point, this is all present in the `psl::config` namespace. All configurations are templated and use the `default_setting_t` as the template type, so when you provide a specialization for that type for the given config, you can change the behaviour throughout the application from the original default behaviour.

## Error Handling strategy

The PSL comes with various error handling strategies, these can be tweaked (like most elements) in the `psl::config` namespace. Notably the ones relating to `exception` and `assert`. In this section we will will delve into what the various error categories are.

### Expected Errors
These should be clear from the API, when calling into a method this will commonly be return values. These are considered "common errors", in that doing nothing will result in the application continuing normally (as long as you don't try to use the result).

### Unexpected Errors
These come in the variation of either `exception`, `asserts`, or `std::exit/abort` (depending on your configuration settings). These happen when an invariant has been violated (like accessing an array out of bounds, or doing a division by zero). These are considered "unrecoverable unless you write code to handle it".

These can be configurated to be "never-recoverable" by turning off exceptions, and can be ignored (at your own danger) in release builds, i.e. making the assumption this error category never triggers during a well-behaved release version of your product (as it should, but that's not always possible).

### Warnings
Sometimes errors don't cause stability related consequences, but they are still an incorrect usage. Like unexpected errors, these aren't always clear from the API. They will however warn you in the logs (if allowed, check your settings for this). These warnings could range from performance related impact (such as expensive conversions), to warnings of things that might change down the road.

### Constraint Errors
This last category is a bit different than the previous ones. Although this category can overlap with `Unexpected Errors`, it is different enough to warrent a seperate category. Constraint errors indicate that you have extended the library in an unsafe manner by extending it with code that does not satisfy required constraints. As an example, the allocator makes assumptions on alignments passed to it, if your custom memory resource does not satisfy its own alignment requirements, then this is considered a constraint error and should be resolved.

Like `Unexpected Errors`, these can be turned off in release builds, but not for debug, as this category indicates points where an error could cause hard to debug errors down the road (such as memory corruption). It's a big red alarm yelling "please consult the documentation".

## Serialization
A low-overhead (zero-runtime cost when not de/serializing) serialization solution is implemented in this library. You can wrap your types in a `psl::property<Type, Name>` object, which is then used by this system to deal with handling the de/serialization. You can customize the input and output formats, but default a custom format is used that can handle all the features present in the system.

The system comes with an ability to handle versioning, and automatic version upgrading at runtime (with compile time checks for potential issues).

`psl::property<T, N>` is the exact same size as the contained type (`T`), and acts as a transparent wrapper type over the contained type. The name is compiled into the binary, and has no runtime cost. The name can be used in `compile-time expressions` as well. For example you could make sure your type has a property with the given name in a `static_assert`.

## Resource Handling
Resource handling is one of the more important aspects that need to be safely handled during the runtime of an engine. In this library there exists two facilities to deal with resources which together create the wanted functionality.

First there is the `psl::meta::library`, this contains a collection of information about unique resources. This information is stored in `psl::meta::data`, a polymorphic type that contains at least a `psl::uid` (unique for that resource in the library), and associated `psl::meta::tag`'s.

The library has the functionality to appoint which resource depends-on which resource, their disk based location (if any), and the environment they apply. An environment is any user provided distinct grouping. As examples this could be the target platform (Windows, Android, etc..), or graphics backends (vulkan, gles, etc...).

The `psl::resource::cache` contains instances of resources, and references to their associated `psl::meta::data`. These can then be safely interacted with using the `psl::resource::handle` given back.

## Threading Workgroups
These are facilities to give you easy access to worker-like `psl::thread` (a thin wrapper over normal `std::jthread`'s). These threads can be subscribed to many `psl::workgroup`'s, and these workgroups are best used for dedicated task categories (such as having a workgroup dedicated for all your physics).

By default these behave as if one thread will manage them, so be sure to change the behaviour if you need external synchronization.

## Allocators
### Motivation
The standard library comes with a high quality API for custom allocator and memory_resource behaviour, but these all make the assumption the backing resource can be reached physically for both reading/writing, and are synchronized. This isn't always true for our intended use case. Most likely you will never need this customization, but for the rare use cases this is needed, this will help you along.

For usage of this it is best to consult the `paradigm` project, which uses this behaviour for dealing with GPU backed resources.