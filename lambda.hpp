#define V(X) Variable(#X)

// Term Definitions
//{
// Combinators
//{
// I Combinator
const Abstraction I(
    V(x) >> (
        V(x)
    )
);

// K Combinator
const Abstraction K(
    V(x) >> (
        V(y) >> (
            V(x)
        )
    )
);

// S Combinator
const Abstraction S(
    V(x) >> (
        V(y) >> (
            V(z) >> (
                V(x), V(z), (V(y), V(z))
            )
        )
    )
);

// B Combinator
const Abstraction B(
    V(x) >> (
        V(y) >> (
            V(z) >> (
                V(x), (V(y), V(z))
            )
        )
    )
);

// C Combinator
const Abstraction C(
    V(x) >> (
        V(y) >> (
            V(z) >> (
                V(x), V(z), V(y)
            )
        )
    )
);

// W Combinator
const Abstraction W(
    V(x) >> (
        V(y) >> (
            V(x), V(y), V(y)
        )
    )
);

// U Combinator
const Abstraction U(
    V(x) >> (
        V(x), V(x)
    )
);

// Y Combinator
const Abstraction Y(
    V(g) >> (
        V(x) >> (
            V(g), (V(x), V(x))
        ),
        V(x) >> (
            V(g), (V(x), V(x))
        )
    )
);

// Iota Combinator
const Abstraction IOTA(
    V(f) >> (
        V(f), S, K
    )
);

// Omega Combinator
const Application OMEGA(
    U, U
);
//}

// Booleans
//{
// Boolean True
const Abstraction TRUE(
    V(x) >> (
        V(y) >> (
            V(x)
        )
    )
);

// Boolean False
const Abstraction FALSE(
    V(x) >> (
        V(y) >> (
            V(y)
        )
    )
);

// Boolean Not
const Abstraction NOT(
    V(p) >> (
        V(p), FALSE, TRUE
    )
);

// Boolean And
const Abstraction AND(
    V(p) >> (
        V(q) >> (
            V(p), V(q), V(p)
        )
    )
);

// Boolean Or
const Abstraction OR(
    V(p) >> (
        V(q) >> (
            V(p), V(p), V(q)
        )
    )
);

// Boolean Xor
const Abstraction XOR(
    V(p) >> (
        V(q) >> (
            V(p), (NOT, V(q)), V(q)
        )
    )
);
//}

// Natural Numbers
//{
// Natural Zero
const Abstraction ZERO(
    V(f) >> (
        V(x) >> (
            V(x)
        )
    )
);

// Natural Successor
const Abstraction SUCC(
    V(n) >> (
        V(f) >> (
            V (x) >> (
                V(f),
                (V(n), V(f), V(x))
            )
        )
    )
);

/**
 * Returns the Church Numeral corresponding to the given argument.
 */
Application NAT(int n) noexcept {
    Application x(I, ZERO);
    
    for (int i = 0; i < n; ++i) {
        x = (SUCC, x);
    }
    
    return x;
}
//}

// Pairs
//{
// Value Pair
const Abstraction PAIR(
    V(x) >> (
        V(y) >> (
            V(f) >> (
                V(f), V(x), V(y)
            )
        )
    )
);

// First Pair Accessor
const Abstraction FIRST(
    V(p) >> (
        V(p), TRUE
    )
);

// Second Pair Accessor
const Abstraction SECOND(
    V(p) >> (
        V(p), FALSE
    )
);

// Empty List
const Abstraction NIL(
    V(x) >> (
        TRUE
    )
);

// Empty List Test
const Abstraction NULLP(
    V(p) >> (
        V(p), (
            V(x) >> (
                V(y) >> (
                    FALSE
                )
            )
        )
    )
);
//}
//}

// The lambda term to be evaluated.
const LambdaTerm& MAIN = (
    NAT(2)
);

#undef V