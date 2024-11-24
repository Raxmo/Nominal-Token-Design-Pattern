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
	[ ] - Token
		[ ] - Core
		[ ] - Extras
	[ ] - Datum
		[x] - Core
		[ ] - Extras
	[ ] - Behavior
		[ ] - Core
		[ ] - Extras

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
	operator T& () { return data; }
	void operator >> (Token& itoken) { token = itoken; }

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

	T& operator [] (size_t token)
	{
		static T invalid;
		if (!pools.contains(token)) return invalid;
		return data[pools[token]];
	}
};
#pragma endregion

#pragma region Behavior stuffs

#pragma endregion

#pragma region Accessors

#pragma endregion