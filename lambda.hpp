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

/* S Combinator
 * SK Combinatory Calculus is Turing Complete.
 * S, K, V(x) = I
 */
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

/* Iota Combinator
 * The Iota Combinator is Turing Complete by itself.
 * IOTA, IOTA = I
 * IOTA, (IOTA, IOTA) = FALSE = ZERO
 * IOTA, (IOTA, (IOTA, IOTA)) = TRUE = K
 * IOTA, (IOTA, (IOTA, (IOTA, IOTA))) = S
 * SK Combinatory Calculus is Turing Complete, so
 *   Iota Combinatory Calculus is Turing Complete.
 */
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

// Matural One
const Abstraction ONE(
    V(f) >> (
        V(x) >> (
            V(f), V(x)
        )
    )
);

// Natural Successor
const Abstraction SUCC(
    V(n) >> (
        V(f) >> (
            V (x) >> (
                V(f), (V(n), V(f), V(x))
            )
        )
    )
);

// Natural Addition
const Abstraction PLUS(
    V(m) >> (
        V(n) >> (
            V(m), SUCC, V(n)
        )
    )
);

// Natural Multiplication
const Abstraction MULT(
    V(m) >> (
        V(n) >> (
            V(m), (PLUS, V(n)), ZERO
        )
    )
);

// Natural Exponentiation
const Abstraction POW(
    V(m) >> (
        V(n) >> (
            V(n), (MULT, V(m)), ONE
        )
    )
);

// Natural Predecessor
const Abstraction PRED(
    V(n) >> (
        V(f) >> (
            V(x) >> (
                V(n),
                (
                    V(g) >> (
                        V(h) >> (
                            V(h), (V(g), V(f))
                        )
                    )
                ),
                (
                    V(u) >> (
                        V(x)
                    )
                ),
                (
                    V(u) >> (
                        V(u)
                    )
                )
            )
        )
    )
);

// Natural Subtraction
const Abstraction SUB(
    V(m) >> (
        V(n) >> (
            V(n), PRED, V(m)
        )
    )
);

// Natural Zero Test
const Abstraction ISZERO(
    V(n) >> (
        V(n),
        (
            V(x) >> (
                FALSE
            )
        ),
        TRUE
    )
);

// Natural <= Test
const Abstraction LEQ(
    V(m) >> (
        V(n) >> (
            ISZERO, (SUB, V(m), V(n))
        )
    )
);

/**
 * Returns the Church Numeral corresponding to the given argument.
 */
Abstraction NAT(int n) noexcept {
    // The number is positive.
    if (n > 0) {
        Application numeral(V(f), V(x));
        
        for (int i = 1; i < n; ++i) {
            numeral = (V(f), numeral);
        }
        
        return
            Abstraction(
                V(f) >> (
                    V(x) >> (
                        numeral
                    )
                )
            )
        ;
    }
    
    // Requests for a non-positive number returns Church Numeral 0.
    else {
        return ZERO;
    }
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

// Pair First Accessor
const Abstraction FIRST(
    V(p) >> (
        V(p), TRUE
    )
);

// Pair Second Accessor
const Abstraction SECOND(
    V(p) >> (
        V(p), FALSE
    )
);
//}

// Lists
//{
// Empty List
const Abstraction NIL(
    V(x) >> (
        TRUE
    )
);

// Empty List Test
const Abstraction ISNIL(
    V(p) >> (
        V(p),
        (
            V(x) >> (
                V(y) >> (
                    FALSE
                )
            )
        )
    )
);

// List Prepending
const Abstraction CONS(
    V(h) >> (
        V(t) >> (
            PAIR, V(h), V(t)
        )
    )
);

// List Head Accessor
const Abstraction HEAD(
    FIRST
);

// List Tail Accessor
const Abstraction TAIL(
    SECOND
);

// List Index Accessor
const Application INDEX(
    Y,
    (
        V(f) >> (
            V(l) >> (
                V(n) >> (
                    ISZERO, V(n), (HEAD, V(l)), (V(f), (TAIL, V(l)), (PRED, V(n)))
                )
            )
        )
    )
);
//}

// Algorithms
//{
// Factorial Function
const Application FACT(
    Y,
    (
        V(f) >> (
            V(n) >> (
                ISZERO, V(n), ONE, (MULT, V(n), (V(f), (PRED, V(n))))
            )
        )
    )
);

// Fibonacci Function
const Application FIBO(
    Y,
    (
        V(f) >> (
            V(n) >> (
                ISZERO, V(n), ZERO,
                (
                    ISZERO, (PRED, V(n)), ONE,
                    (PLUS, (V(f), (PRED, V(n))), (V(f), (PRED, (PRED, V(n)))))
                )
            )
        )
    )
);
//}
//}

// The lambda term to be evaluated.
const LambdaTerm& MAIN = (
    FACT, NAT(3)
);

#undef V