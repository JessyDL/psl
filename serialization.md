# Serialization

## Motivation

Serialization is one of the most important features you'll need in a game engine, and even though *a* version of serialization is easy to implement, implementing a robust, safe solution can be an uphill battle.

The solution provided is slightly different than most serialization libraries out there, as it's not directly linked to a specific format (but does come with its own format as a default).

Great effort was spent in making sure the solution was safe to use, enforced by compile-time constraint checking; that it was customizable, as everyone out there has different needs the design of the solution takes a more algorithm-like approach; and zero-overhead, as nobody likes paying for something they aren't using.

The constraints to satisfy zero-overhead means that marking something as serializable has no size or performance related overheads at runtime in release builds compared to the object when it is not wrapped in a serializeable property.

## Implementation Details

There are two layers to the serialization system (this is opaque to the user), one is a static system used for when the types are known, and no polymorphism is needed. In this case, as everything is transparent to the compiler, we can generate all the needed code, handle versioning, and the compiler is free to optimize as much as it can.

The second layer is a bit more complex as it can handle polymorphic types. This layer requires the user to register a unique name for their type (compile errors will warn you when you forget this). This layer does have some overhead (the cost of a lookup, and storing a factory-like container, this is neglible outside of the context of serialization).
