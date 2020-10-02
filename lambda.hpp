/**
 * Contains the definitions for various lambda expressions.
 */

// Boolean True
const Abstraction TRUE(
    Variable("x") >> (
        Variable("y") >> (
            Variable("x")
        )
    )
);

// Boolean False
const Abstraction FALSE(
    Variable("x") >> (
        Variable("y") >> (
            Variable("y")
        )
    )
);

// Boolean And
const Abstraction AND(
    Variable("p") >> (
        Variable("q") >> (
            Variable("p"), Variable("q"), Variable("p")
        )
    )
);

// Boolean Or
const Abstraction OR(
    Variable("p") >> (
        Variable("q") >> (
            Variable("p"), Variable("p"), Variable("q")
        )
    )
);

// Boolean Not
const Abstraction NOT(
    Variable("p") >> (
        Variable("p"), FALSE, TRUE
    )
);

// The lambda term to be evaluated.
const LambdaTerm& MAIN = (
    AND,
    (OR, TRUE, (NOT, TRUE)),
    (OR, FALSE, (NOT, FALSE))
);