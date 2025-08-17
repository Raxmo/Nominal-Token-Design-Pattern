#pragma once

#include <functional>
#include <unordered_map>
#include <unordered_set>

/*
================================================================================
Nominal Token Design Pattern - Core Header
================================================================================

This header implements the Nominal Token Design Pattern, which separates identity
(Tokens), data (Datums), and behavior (Behaviors) into distinct, composable units.

-------------------------------
Ideology & Motivation
-------------------------------
- **Token**: A nominal type representing a pure identity, containing no data or behavior by default.
- **Datum**: A data container that holds values but has no identity or behavior of its own.
- **Behavior**: An executable unit of logic that has no identity or data storage.

This separation of concerns allows for a highly flexible and dynamic system where:
- Data and behaviors can be associated with tokens at runtime.
- Objects can be composed with desired characteristics without relying on traditional inheritance, avoiding rigid class hierarchies.
- Data and behaviors can be shared and reused across multiple tokens or groups of tokens.

-------------------------------
Usage Overview
-------------------------------
The pattern is designed to be intuitive and expressive.

1.  **Create a Token**:
    `Token myToken;`

2.  **Define a Datum** to hold a specific type of data:
    `Datum<std::string> Name;`

3.  **Associate Data with a Token**:
    `myToken + Name = "John Doe";`

4.  **Define a Behavior** to encapsulate logic:
    `Behavior<void()> Greet = { [&](){ std::cout << "Hello, " << Greet[Name] << std::endl; } };`

5.  **Subscribe a Token to a Behavior**:
    `myToken += Greet;`

6.  **Execute a Behavior**:
    `myToken[Greet]();` // Executes for one token
    `Greet();`          // Executes for all subscribed tokens

================================================================================
*/

#pragma region Declarations
// Forward declarations for all core components of the pattern.
struct Token;
template<class t> struct Datum;
template<class t> struct Static_Datum;
template<class t> struct Solitary_Datum;
template<class t> struct Shared_Datum;
template<class R, class... Args> struct Behavior;
#pragma endregion

#pragma region Datums
// -----------------------------------------------------------------------------
// Datum: Associates a unique data value with each token.
// This is the most common type of datum.
// -----------------------------------------------------------------------------
template<class T>
struct Datum {
    std::unordered_map<size_t, T> data;

    // Accesses (or creates) the data associated with a specific token ID.
    T& operator [] (const size_t& token) {
        static T invalid; // Return a static invalid instance if token is 0.
        if (!token) return invalid;
        return data[token];
    }
};

// -----------------------------------------------------------------------------
// Static_Datum: Shares a single data instance among a subscribed set of tokens.
// Useful for properties that are constant across a group.
// -----------------------------------------------------------------------------
template<class T>
struct Static_Datum {
    T data;
    std::unordered_set<size_t> tokens;

    Static_Datum(const T& idata) : data(idata) {}

    // Accesses the shared data if the token is in the subscribed set.
    T& operator [] (size_t token) {
        static T invalid;
        if (!tokens.contains(token)) return invalid;
        return data;
    }

    // Allows direct access to the underlying data.
    operator T& () { return data; }
};

// -----------------------------------------------------------------------------
// Solitary_Datum: A data instance that can only be associated with a single token.
// -----------------------------------------------------------------------------
template<class T>
struct Solitary_Datum {
    T data;
    size_t token = 0; // The ID of the single associated token.

    Solitary_Datum(const T& idata) : data(idata) {}

    // Accesses the data only if the requesting token's ID matches the stored one.
    T& operator [] (size_t itoken) {
        static T invalid;
        if (itoken != token) return invalid;
        return data;
    }
    void operator = (T& idata) { data = idata; }
};

// -----------------------------------------------------------------------------
// Shared_Datum: Allows distinct groups of tokens to share data instances.
// Tokens are mapped to a "pool" of data.
// -----------------------------------------------------------------------------
template<class T>
struct Shared_Datum {
    std::unordered_map<size_t, size_t> pools; // Maps a token ID to a data pool ID.
    std::unordered_map<size_t, T> data;       // Stores the data for each pool.
    size_t count = 0;

    // Accesses the data from the pool associated with the given token.
    T& operator [] (size_t& token) {
        static T invalid;
        if (!pools.contains(token)) return invalid;
        return data[pools[token]];
    }
};
#pragma endregion

#pragma region Behavior
// -----------------------------------------------------------------------------
// Behavior: Encapsulates executable logic that can be subscribed to by tokens.
// The behavior's logic is executed within the "context" of a specific token.
// -----------------------------------------------------------------------------
template<class R, class... Args>
struct Behavior<R(Args...)> {
#pragma region properties
    std::unordered_set<size_t> tokens;      // Container for the subscribed token IDs.
    std::function<R(Args...)> behavior;     // The actual functor to be executed.
    size_t ct = 0;                          // The "current token" context for execution.
#pragma endregion

#pragma region Core
    Behavior(std::function<R(Args...)> ibehavior) : behavior(ibehavior) {}

    // Sets the execution context to a specific token and returns the behavior's function.
    std::function<R(Args...)>& operator [] (const size_t& token) {
        static std::function<R(Args...)> invalid;
        if (!tokens.contains(token)) return invalid;
        ct = token;
        return behavior;
    }
#pragma endregion

#pragma region Datum Access
    // Allows a behavior to access a datum using the current token's context.
    // Example: `myBehavior[MyDatum]` will access `MyDatum` for the current token.
    template<class T> T& operator [] (Datum<T>& datum) { return datum[ct]; }
    template<class T> T& operator [] (Static_Datum<T>& datum) { return datum[ct]; }
    template<class T> T& operator [] (Solitary_Datum<T>& datum) { return datum[ct]; }
    template<class T> T& operator [] (Shared_Datum<T>& datum) { return datum[ct]; }
#pragma endregion

#pragma region QOL
    // Executes the behavior for every subscribed token.
    void operator () (Args... args) {
        for (auto& token : tokens) {
            ct = token;
            behavior(args...);
        }
    }

    // Allows the behavior to be used where a token ID is expected, providing the current context.
    // Example: `MyDatum[myBehavior]`
    operator size_t& () { return ct; }
#pragma endregion
};
#pragma endregion

#pragma region Tokens
// -----------------------------------------------------------------------------
// Token: Represents a pure identity.
// It acts as a key to access associated data and behaviors.
// -----------------------------------------------------------------------------
struct Token {
    size_t self; // The unique ID of this token.

#pragma region Base
    Token() {
        static size_t count = 0;
        self = ++count;
    }
    Token(size_t& id) : self(id) {}
    ~Token() {}
#pragma endregion

#pragma region Datum Access
    // Allows a token to directly access data within a datum.
    // Example: `myToken[MyDatum]`
    template<class T> T& operator [] (Datum<T>& idatum) { return idatum[self]; }
    template<class T> T& operator [] (Solitary_Datum<T>& idatum) { return idatum[self]; }
    template<class T> T& operator [] (Shared_Datum<T>& idatum) { return idatum[self]; }
    template<class T> T& operator [] (Static_Datum<T>& idatum) { return idatum[self]; }
#pragma endregion

#pragma region Behavior Access
    // Allows a token to access a specific behavior's function.
    // Example: `myToken[myBehavior]()`
    template<class R, class... Args>
    std::function<R(Args...)> operator [] (Behavior<R(Args...)>& behavior) {
        return behavior[self];
    }
#pragma endregion

#pragma region QOL
    // Allows a token to be used wherever its ID (size_t) is needed.
    operator size_t& () { return self; }
#pragma endregion
};
#pragma endregion

#pragma region Operator Overloads
// These overloads provide the expressive, high-level syntax for the pattern.

#pragma region Datums
// Associates a value with a token in a Datum. Usage: `token + datum = value;`
template<class T> T& operator + (size_t& token, Datum<T>& datum) { return datum.data[token]; }
// Removes a token's data from a Datum. Usage: `value = token - datum;`
template <class T> T operator - (size_t& token, Datum<T>& datum) { T val = datum[token]; datum.data.erase(token); return val; }

// Subscribes a token to a Static_Datum. Usage: `token += static_datum;`
template<class T> void operator += (size_t& token, Static_Datum<T>& datum) { datum.tokens.insert(token); }
// Unsubscribes a token from a Static_Datum. Usage: `token -= static_datum;`
template <class T> void operator -= (size_t& token, Static_Datum<T>& datum) { datum.tokens.erase(token); }

// Assigns a token to a Solitary_Datum. Usage: `token >> solitary_datum = value;`
template <class T> T& operator >> (size_t& token, Solitary_Datum<T>& datum) { datum.token = token; return datum.data; }

// Adds a token to a new data pool in a Shared_Datum. Usage: `token + shared_datum = value;`
template<class T> T& operator + (size_t& token, Shared_Datum<T>& datum) { datum.pools[token] = ++datum.count; return datum[token]; }
// Adds a token to the most recent data pool in a Shared_Datum. Usage: `token >> shared_datum;`
template <class T> size_t& operator >> (size_t& token, Shared_Datum<T>& datum) { datum.pools[token] = datum.count; return datum.pools[token]; }
// Removes a token from a Shared_Datum. Usage: `value = token - shared_datum;`
template<class T> T operator - (size_t& token, Shared_Datum<T>& datum) { T val = datum[token]; datum.pools.erase(token); return val; }
#pragma endregion

#pragma region Behaviors
// Subscribes a token to a Behavior. Usage: `token += behavior;`
template<class R, class... Args> void operator += (size_t& token, Behavior<R(Args...)>& behavior) { behavior.tokens.insert(token); }
// Unsubscribes a token from a Behavior. Usage: `token -= behavior;`
template<class R, class... Args> void operator -= (size_t& token, Behavior<R(Args...)>& behavior) { behavior.tokens.erase(token); }
#pragma endregion

#pragma endregion


/*

Shared_Datum
    -> be able to add a token to a particular pool of data
    -> be able to access the data accossiated with a token


What I want to be able to do, is to say something like:
	player + Model = model_1;
*/