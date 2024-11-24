#pragma once

#include <functional>
#include <unordered_map>
#include <unordered_set>

/*

TODO: For the Nominal Token design principal.

NOTES:
	- Token are nominal, containing no data, nor behavior.
	- Datum have no identity, nor behavior.
	- Behavior have no identity, nor data.

	- Token should be able to be added to Datum in order to add data to tokens.
		-> token + datum = value;
		-> datum + token = value;
	- Tokens should be able to be added to Behaviors in order to add behaviors to tokens.
		-> token += behavior;
*/

/*

TODO:
	[X] - Token
		[X] - Core
		[X] - Extras
	[X] - Datum
		[x] - Core
		[X] - Extras
	[X] - Behavior
		[X] - Core
		[X] - Extras

*/

#pragma region Declarations
struct Token;
template<class t>
struct Datum;
template<class t>
struct Static_Datum;
template<class t>
struct Solitary_Datum;
template<class t>
struct Shared_Datum;
template<class R, class... Args>
struct Behavior;
#pragma endregion

#pragma region Datums
template<class T>
/*
	This is the base-line Datum class.
	The Datum is a structure that allows for quick access to data given a token.
	The idea behind datums is that the token doesn't hold data, but is granted access to data.
	This will let multiple classes or objects be able to hold the same data without inheritence.
*/
struct Datum
{
	std::unordered_map<size_t, T> data;
	T& operator [] (size_t token)
	{
		static T invalid;
		if (!token) return invalid;
		return data[token];
	}
};

template<class T>
/*
	The static datum is a datum class that allows multiple tokens to access the same instance of data.
*/
struct Static_Datum
{
	T data;
	std::unordered_set<size_t> tokens;
	Static_Datum(T& idata) : data(idata) {};

	T& operator [] (size_t token)
	{
		static T invalid;
		if (!tokens.contains(token)) return invalid;
		return data;
	};

	operator T& () { return data; }
};

template<class T>
/*
	The solitary datum is a single instance of data for a singular token.
*/
struct Solitary_Datum
{
	T data;
	size_t token;

	Solitary_Datum(const T& idata) : data(idata) {};
	T& operator [] (size_t itoken)
	{
		static T invalid;
		if (itoken != token) return invalid;
		return data;
	}
	void operator = (T& idata) { data = idata; }

};

template<class T>
/*
	Shared Datum allow for tokens to share data between instances and classes.
	This allows for multiple groups of tokens to share instances of particular data.
*/
struct Shared_Datum
{
	std::unordered_map<size_t, size_t> pools;
	std::unordered_map<size_t, T> data;
	size_t count = 0;

	T& operator [] (size_t& token)
	{
		static T invalid;
		if (!pools.contains(token)) return invalid;
		return data[pools[token]];
	}
};
#pragma endregion

#pragma region Behavior stuffs
template<class R, class... Args>
struct Behavior<R(Args...)>
{
#pragma region properties
	std::unordered_set<size_t> tokens; // Container for the subscribed tokens.
	std::function<R(Args...)> behavior; // the actual functor that is to be executed.
	size_t ct = 0; // the current token. This determines the context of the functor.
#pragma endregion
#pragma region Core stuffs
	Behavior<R(Args...)>(std::function<R(Args...)> ibehavior) :
		behavior(ibehavior) {}; // Constructor for ease of creation.
	std::function<R(Args...)> operator [] (size_t token)
	{// Here, when we access a behavior with a token, we return the behavior and update the context for the behavior to the given token.
		if (!tokens.contains(token)) return function<R(Args...)>();
		ct = token;
		return behavior;
	}
#pragma endregion
#pragma region Datum Stuffs
	// Yes all four of these methods are the same, but they are kinda needed in order to handle things when using the design pattern.
	template<class T>
	T& operator [] (Datum<T>& datum)
	{
		return datum[ct];
	}
	template<class T>
	T& operator [] (Static_Datum<T>& datum)
	{
		return datum[ct];
	}
	template<class T>
	T& operator [] (Solitary_Datum<T>& datum)
	{
		return datum[ct];
	}
	template<class T>
	T& operator [] (Shared_Datum<T>& datum)
	{
		return datum[ct];
	}
#pragma endregion
#pragma region QOL
	// This is to be able to do something like Behavior() and apply the behavior to every token subscribed.
	void operator () (Args... args)
	{
		for (auto& token : tokens)
		{
			ct = token;
			behavior(args...);
		}
	}
	// this is to give the ability to do something like this: Datum[Behavior]
	operator size_t& ()
	{
		return ct;
	}
#pragma endregion
};
#pragma endregion

#pragma region Tokens
struct Token
{
	size_t self;

#pragma region Base
	Token()
	{
		static size_t count;
		self = ++count;
	}
	Token(size_t& id) : self(id) {};
#pragma endregion
#pragma region Datum stuffs
	// Much like the behaviors, all of the datum stuffs is the same here as well.
	template<class T>
	T& operator [] (Datum<T>& idatum)
	{
		return idatum[self];
	}
	template<class T>
	T& operator [] (Solitary_Datum<T>& idatum)
	{
		return idatum[self];
	}
	template<class T>
	T& operator [] (Shared_Datum<T>& idatum)
	{
		return idatum[self];
	}
	template<class T>
	T& operator [] (Static_Datum<T>& idatum)
	{
		return idatum[self];
	}
#pragma endregion
#pragma region Behavior stuffs
	template<class R, class... Args>
	std::function<R(Args...)> operator []
	(Behavior<R(Args...)>& behavior)
	{
		return behavior[self];
	}
#pragma endregion
#pragma region QOL
	operator size_t& ()
	{
		return self;
	}
#pragma endregion
};
#pragma endregion

#pragma region Overloads
#pragma region Datums
#pragma region Datum
template<class T>
T& operator + (size_t& token, Datum<T>& datum)
{// Adding datums to tokens with: token + datum = value;
	return datum.data[token];
}
template <class T>
T operator - (size_t& token, Datum<T>& datum)
{// remove datums from tokens with: optional_value = token - datum;
	T val = datum[token];
	datum.data.erase(token);
	return val;
}
#pragma endregion
#pragma region Static Datums
template<class T>
void operator += (size_t& token, Static_Datum<T>& datum)
{// Adding a token to a static datum
	datum.tokens.insert(token);
}
template <class T>
void operator -= (size_t& token, Static_Datum<T>& datum)
{// removing a token from a static datum
	datum.tokens.erase(token);
}
#pragma endregion
#pragma region Solitary Datum
template <class T> // This is the way to access the token stored in the solitary datum.
T& operator >> (size_t& token, Solitary_Datum<T>& datum)
{
	datum.token = token;
	return datum.data;
}
#pragma endregion
#pragma region Shared Datums
// Shared datums are a bit weird, but not the worst to impliment.
template<class T>
T& operator + (size_t& token, Shared_Datum<T>& datum)
{
	datum.tokens[token] = ++datum.count;
	return datum[token];
}
template <class T>
size_t& operator >> (size_t& token, Shared_Datum<T>& datum)
{
	datum.tokens[token] = datum.count;
	return datum.tokens[token];
}

// This is how you remove a token from the pool in the datum.
template<class T>
T operator - (size_t& token, Shared_Datum<T>& datum)
{
	T val = datum[token];
	datum.tokens.erase[token];
	return val;
}
#pragma endregion
#pragma endregion
#pragma region Behaviors
template<class R, class... Args> // Adds tokens to the Behavior
void operator += (size_t& token, Behavior<R(Args...)>& behavior)
{
	behavior.tokens.insert(token);
}
template<class R, class... Args> // removes tokens from the behavior
void operator -= (size_t& token, Behavior<R(Args...)>& behavior)
{
	behavior.tokens.erse(token);
}
#pragma endregion
#pragma endregion